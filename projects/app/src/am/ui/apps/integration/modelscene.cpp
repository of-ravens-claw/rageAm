#include "modelscene.h"
#ifdef AM_INTEGRATED

#include "am/ui/font_icons/icons_am.h"
#include "am/ui/styled/slwidgets.h"
#include "rage/paging/builder/builder.h"

void rageam::ModelScene::CreateEntity(const rage::Vec3V& coors)
{
	DeleteEntity();

	m_EntityPos = coors;
	m_Entity = SHV::OBJECT::CREATE_OBJECT_NO_OFFSET(
		RAGEAM_HASH, m_EntityPos.X(), m_EntityPos.Y(), m_EntityPos.Z(), FALSE, TRUE, FALSE);
}

void rageam::ModelScene::DeleteEntity()
{
	if (m_Entity == 0)
		return;
	SHV::ENTITY::SET_ENTITY_AS_MISSION_ENTITY(m_Entity, FALSE, TRUE);
	SHV::OBJECT::DELETE_OBJECT(&m_Entity);
}

bool rageam::ModelScene::LoadAndCompileDrawable(ConstWString path)
{
	DeleteDrawable();

	m_FileWatcher.SetEnabled(false);

	amPtr<asset::DrawableAsset> asset = asset::AssetFactory::LoadFromPath<asset::DrawableAsset>(path);
	if (!asset)
	{
		AM_ERRF(L"ModelScene::LoadAndCompileDrawable() -> Failed to load drawable from path %ls", path);
		return false;
	}

	gtaDrawable* drawable = new gtaDrawable();
	if (!asset->CompileToGame(drawable))
	{
		AM_ERRF(L"ModelScene::LoadAndCompileDrawable() -> Failed to compile drawable from path %ls", path);
		return false;
	}
	m_Drawable = amUniquePtr<gtaDrawable>(drawable);

	m_FileWatcher.SetEntry(String::ToUtf8Temp(asset->GetDrawableModelPath()));
	m_FileWatcher.SetEnabled(true);

	return true;
}

void rageam::ModelScene::RegisterArchetype()
{
	if (m_Archetype)
		UnregisterArchetype();

	static gmAddress initArchetypeFromDef_Addr = gmAddress::Scan("48 89 5C 24 08 57 48 83 EC 20 49 8B F8 48 8B D9 E8 ?? ?? ?? ?? 44");
	static void(__fastcall * initArchetypeFromDef)(CBaseModelInfo*, rage::strLocalIndex, rage::fwArchetypeDef*, bool) =
		initArchetypeFromDef_Addr.To<decltype(initArchetypeFromDef)>();

	static gmAddress baseModelInfoCtor_Addr = gmAddress::Scan("65 48 8B 14 25 58 00 00 00 48 8D 05 ?? ?? ?? ?? 45");
	static void(__fastcall * baseModelInfoCtor)(CBaseModelInfo*) =
		baseModelInfoCtor_Addr.To<decltype(baseModelInfoCtor)>();

	CBaseModelInfo* modelInfo = (CBaseModelInfo*)rage_malloc(0xB0);
	baseModelInfoCtor(modelInfo);

	auto& drBb = m_Drawable->GetLodGroup().GetBoundingBox();
	auto& drBs = m_Drawable->GetLodGroup().GetBoundingSphere();

	CBaseArchetypeDef modelDef{};
	modelDef.m_Name = RAGEAM_HASH;
	modelDef.m_AssetType = rage::fwArchetypeDef::ASSET_TYPE_DRAWABLE;
	modelDef.m_BoundingBox = drBb;
	modelDef.m_BsCentre = drBs.GetCenter();
	modelDef.m_BsRadius = drBs.GetRadius().Get();
	modelDef.m_AssetName = RAGEAM_HASH;
	modelDef.m_LodDist = 100;
	modelDef.m_PhysicsDictionary = RAGEAM_HASH;
	modelDef.m_Flags = rage::ADF_STATIC | rage::ADF_BONE_ANIMS;
	//modelDef.m_TextureDictionary = rage::joaat("adder");

	initArchetypeFromDef(modelInfo, 0, &modelDef, /*false*/ true);

	modelInfo->m_Flags |= 1; // Drawable loaded?

	// TODO: Must be done by init archetype...
	static gmAddress registerArchetype_Addr = gmAddress::Scan(
		"48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 20 48 83 3D ?? ?? ?? ?? ?? 8B FA");
	static void(__fastcall * registerArchetype)(CBaseModelInfo*, rage::strLocalIndex) =
		registerArchetype_Addr.To<decltype(registerArchetype)>();
	registerArchetype(modelInfo, 0);
	m_Archetype = modelInfo;
}

void rageam::ModelScene::UnregisterArchetype()
{
	if (!m_Archetype)
		return;

	AM_ASSERT(m_ArchetypeOld == nullptr, "ModelScene::UnregisterArchetype() -> Old archetype is not unloaded yet.");

	m_ArchetypeOld = m_Archetype;
	m_Archetype = nullptr;
}

void rageam::ModelScene::FinalizeOldArchetype()
{
	if (m_ArchetypeOld && m_ArchetypeOld->m_RefCount == 0)
	{
		static auto modelInfoDctor =
			gmAddress((*gmAddress::Scan("48 8D 05 ?? ?? ?? ?? 4C 89 49 58").GetRef(3).To<u64*>())).ToFunc<void(CBaseModelInfo*, bool)>();

		modelInfoDctor(m_ArchetypeOld, false);
		rage_free(m_ArchetypeOld);
		m_ArchetypeOld = nullptr;
	}
}

void rageam::ModelScene::RegisterDrawable()
{
	rage::strStreamingModule* drawableStore = hooks::Streaming::GetModule("ydr");
	if (m_DrawableSlot == rage::INVALID_STR_INDEX)
		drawableStore->AddSlot(m_DrawableSlot, RAGEAM_HASH);
	drawableStore->Set(m_DrawableSlot, m_Drawable.get());
}

void rageam::ModelScene::UnregisterDrawable()
{
	rage::strStreamingModule* drawableStore = hooks::Streaming::GetModule("ydr");
	if (m_DrawableSlot != rage::INVALID_STR_INDEX)
	{
		drawableStore->Set(m_DrawableSlot, nullptr);
		drawableStore->RemoveSlot(m_DrawableSlot);
		m_DrawableSlot = rage::INVALID_STR_INDEX;
	}
}

void rageam::ModelScene::RequestReload()
{
	m_HasModelRequest = true;
	if (m_IsLoaded)
		m_CleanUpRequested = true;
}

void rageam::ModelScene::DeleteDrawable()
{
	if (!m_Drawable)
		return;

	// Temporary hack until we have simple allocator back because os allocator can't detect
	// if block is valid or not so it cant dispatch delete to virtual allocator for builded resources
	if (m_Drawable->HasMap())
		GetAllocator(rage::ALLOC_TYPE_VIRTUAL)->Free(m_Drawable.release());
	else
		m_Drawable = nullptr;
}

rageam::ModelScene::~ModelScene()
{
	if (m_IsLoaded)
	{
		CleanUp();
		while (m_IsLoaded) {} // Wait until scene is fully unloaded
	}
}

void rageam::ModelScene::OnEarlyUpdate()
{
	std::unique_lock lock(m_Mutex);

	FinalizeOldArchetype();
}

void rageam::ModelScene::OnLateUpdate()
{
	std::unique_lock lock(m_Mutex);

	// Model file was changed, reload drawable
	if (m_FileWatcher.GetChangeOccuredAndReset())
	{
		RequestReload();
	}

	// We accept request only once previous entity was cleaned up
	if (m_HasModelRequest && !m_CleanUpRequested)
	{
		if (m_DrawableRequest)
		{
			DeleteDrawable();
			m_Drawable = std::move(m_DrawableRequest);
		}

		// Use existing drawable (if we just want to respawn entity or if drawable
		// was set externally) or compile a new one from asset
		if (m_Drawable || LoadAndCompileDrawable(m_LoadRequest->Path))
		{
			RegisterDrawable();
			RegisterArchetype();
			CreateEntity(m_EntityPos);
			m_IsLoaded = true;

			//if (m_Drawable->GetSkeletonData())
			//	m_Drawable->GetSkeletonData()->DebugPrint();

			if (LoadCallback) LoadCallback();
		}
		m_HasModelRequest = false;
	}

	if (m_CleanUpRequested)
	{
		m_FileWatcher.SetEnabled(false);

		// Full entity unload takes more than one frame, we first delete
		// entity and on the next frame we clean up the rest
		if (m_Entity != 0) // First tick
		{
			DeleteEntity();
		}
		else // Second tick
		{
			UnregisterArchetype();
			UnregisterDrawable();
			DeleteDrawable();
			m_IsLoaded = false;
			m_CleanUpRequested = false;
		}
	}
}

void rageam::ModelScene::SetupFor(ConstWString path, const rage::Vec3V& coors)
{
	SetEntityPos(coors);

	std::unique_lock lock(m_Mutex);
	m_LoadRequest = std::make_unique<LoadRequest>(path);
	RequestReload();
}

void rageam::ModelScene::CleanUp()
{
	std::unique_lock lock(m_Mutex);
	m_CleanUpRequested = true;
}

void rageam::ModelScene::SetEntityPos(const rage::Vec3V& pos)
{
	std::unique_lock lock(m_Mutex);
	m_EntityPos = pos;

	if (m_Entity != 0)
	{
		scrInvoke([=]
			{
				SHV::ENTITY::SET_ENTITY_COORDS_NO_OFFSET(
					m_Entity, pos.X(), pos.Y(), pos.Z(), FALSE, FALSE, TRUE);
			});
	}
}

gtaDrawable* rageam::ModelScene::GetDrawable()
{
	std::unique_lock lock(m_Mutex);
	return m_Drawable.get();
}

void rageam::ModelScene::SetDrawable(gtaDrawable* drawable)
{
	std::unique_lock lock(m_Mutex);
	m_DrawableRequest = amUniquePtr<gtaDrawable>(drawable);
	RequestReload();
}

rage::Vec3V rageam::ModelSceneApp::GetEntityScenePos() const
{
	return m_IsolatedSceneActive ? DEFAULT_ISOLATED_POS : DEFAULT_POS;
}

void rageam::ModelSceneApp::UpdateDrawableStats()
{
	m_NumModels = 0;
	m_NumGeometries = 0;
	m_VertexCount = 0;
	m_TriCount = 0;

	gtaDrawable* drawable = m_ModelScene.GetDrawable();
	if (!drawable)
		return;

	rage::rmcLodGroup& lodGroup = drawable->GetLodGroup();
	const rage::spdAABB& boundingBox = lodGroup.GetBoundingBox();

	m_Dimensions = boundingBox.Max - boundingBox.Min;
	m_NumLods = lodGroup.GetLodCount();
	for (int i = 0; i < m_NumLods; i++)
	{
		rage::rmcLod* lod = lodGroup.GetLod(i);
		auto& lodModels = lod->GetModels();

		m_NumModels += lodModels.GetSize();
		for (auto& model : lodModels)
		{
			auto& modelGeoms = model->GetGeometries();

			m_NumGeometries += modelGeoms.GetSize();
			for (auto& geom : modelGeoms)
			{
				m_VertexCount += geom->GetVertexCount();
				m_TriCount += geom->GetPrimitiveCount();
			}
		}
	}
}

void rageam::ModelSceneApp::ResetCameraPosition()
{
	if (!m_Camera)
		return;

	rage::Vec3V camPos;
	rage::Vec3V targetPos;
	rage::Vec3V scenePos = m_IsolatedSceneActive ? DEFAULT_ISOLATED_POS : DEFAULT_POS;

	gtaDrawable* drawable = m_ModelScene.GetDrawable();
	if (drawable)
	{
		rage::rmcLodGroup& lodGroup = drawable->GetLodGroup();
		auto& bb = lodGroup.GetBoundingBox();
		auto& bs = lodGroup.GetBoundingSphere();

		camPos = scenePos;
		// Shift camera away to fully see bounding sphere + add light padding
		camPos += rage::VEC_BACK * bs.GetRadius() * 1.5f;
		// Entities are spawned with bottom of bounding box aligned to specified coord
		targetPos = scenePos + rage::VEC_UP * bb.Height() * rage::S_HALF;
	}
	else
	{
		camPos = scenePos + rage::VEC_FORWARD;
		targetPos = scenePos;
	}

	m_Camera->SetPosition(camPos);
	m_Camera->LookAt(targetPos);
}

void rageam::ModelSceneApp::UpdateCamera()
{
	if (m_CameraEnabled)
	{
		if (m_UseOrbitCamera)
			m_Camera = std::make_unique<integration::OrbitCamera>();
		else
			m_Camera = std::make_unique<integration::FreeCamera>();;

		ResetCameraPosition();
		m_Camera->SetActive(true);
	}
	else
	{
		m_Camera = nullptr;
	}
}

void rageam::ModelSceneApp::DrawDrawableUi(const gtaDrawable* drawable) const
{
	if (!drawable)
		return;

	if (ImGui::CollapsingHeader("Render Options"))
	{
		graphics::DebugRender& debugRender = GRenderContext->DebugRender;
		ImGui::Checkbox("Skeleton", &debugRender.bRenderSkeleton);
		ImGui::Checkbox("Bound Mesh", &debugRender.bRenderBoundMesh);
		ImGui::Text("Extents:");
		ImGui::Checkbox("Lod Group", &debugRender.bRenderLodGroupExtents);
		ImGui::Checkbox("Bound", &debugRender.bRenderBoundExtents);
		ImGui::Checkbox("Geometry", &debugRender.bRenderGeometryExtents);
	}

	if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Dimensions: ( %.02f, %.02f, %.02f )",
			m_Dimensions.X, m_Dimensions.Y, m_Dimensions.Z);
		ImGui::Text("LODs: %u", m_NumLods);
		ImGui::Text("Models: %u", m_NumModels);
		ImGui::Text("Geometries: %u", m_NumGeometries);
		ImGui::Text("Vertices: %u", m_VertexCount);
		ImGui::Text("Triangles: %u", m_TriCount);
	}

	//if (ImGui::CollapsingHeader("Skeleton", ImGuiTreeNodeFlags_DefaultOpen))
	if (SlGui::CollapsingHeader(ICON_AM_SKELETON" Skeleton", ImGuiTreeNodeFlags_DefaultOpen))
	{
		rage::crSkeletonData* skeleton = drawable->GetSkeletonData();
		if (skeleton)
		{
			ImGui::Text("Skeleton Bones");
			if (ImGui::BeginTable("TABLE_SKELETON", 6, ImGuiTableFlags_Borders))
			{
				ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Parent");
				ImGui::TableSetupColumn("Sibling");
				ImGui::TableSetupColumn("Child");
				ImGui::TableSetupColumn("Tag");

				ImGui::TableHeadersRow();

				for (u16 i = 0; i < skeleton->GetBoneCount(); i++)
				{
					rage::crBoneData* bone = skeleton->GetBone(i);

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					ImGui::Text("%u", i);
					ImGui::TableNextColumn();
					ImGui::Text("%s", bone->GetName());
					ImGui::TableNextColumn();
					ImGui::Text("%i", bone->GetParentIndex());
					ImGui::TableNextColumn();
					ImGui::Text("%i", bone->GetNextIndex());
					ImGui::TableNextColumn();
					ImGui::Text("%i", (s16)skeleton->GetFirstChildBoneIndex(i));
					ImGui::TableNextColumn();
					ImGui::Text("%i", bone->GetBoneTag());
				}

				ImGui::EndTable();
			}

			std::function<void(rage::crBoneData*)> recurseBone;
			recurseBone = [&](const rage::crBoneData* bone)
				{
					if (!bone)
						return;

					ImGui::Indent();
					while (bone)
					{
						ImGui::BulletText("%s", bone->GetName());
						constexpr ImVec4 attrColor = ImVec4(0, 0.55, 0, 1);

						ImGui::SameLine();
						ImGui::TextColored(attrColor, "Tag: %u", bone->GetBoneTag());

						//rage::Vec3V pos, scale;
						//rage::QuatV rot;
						//const rage::Mat44V& mtx = skeleton->GetBoneTransform(bone->GetIndex());
						//mtx.Decompose(&pos, &scale, &rot);

						//ImGui::TextColored(attrColor, "Trans: %f %f %f", pos.X(), pos.Y(), pos.Z());
						//ImGui::TextColored(attrColor, "Scale: %f %f %f", scale.X(), scale.Y(), scale.Z());
						//ImGui::TextColored(attrColor, "Rot: %f %f %f %f", rot.X(), rot.Y(), rot.Z(), rot.W());

						recurseBone(skeleton->GetFirstChildBone(bone->GetIndex()));

						bone = skeleton->GetBone(bone->GetNextIndex());
					}
					ImGui::Unindent();
				};

			rage::crBoneData* root = skeleton->GetBone(0);
			recurseBone(root);
		}
	}
}

void rageam::ModelSceneApp::OnRender()
{
	if (ImGui::IsKeyPressed(ImGuiKey_RightBracket))
	{
		m_CameraEnabled = !m_CameraEnabled;
		UpdateCamera();
	}

	if (SlGui::BeginToolWindow("Scene Toolbar"))
	{
		if (SlGui::ToggleButton(ICON_AM_OBJECT " Isolate", m_IsolatedSceneActive))
		{
			ResetCameraPosition();
			m_ModelScene.SetEntityPos(GetEntityScenePos());

			scrInvoke([=]
				{
					bool display = !m_IsolatedSceneActive;
					SHV::UI::DISPLAY_HUD(display);
					SHV::UI::DISPLAY_RADAR(display);
				});
		}
		ImGui::ToolTip("Isolates scene model");

		if (SlGui::ToggleButton(ICON_AM_CAMERA_GIZMO" Camera", m_CameraEnabled))
			UpdateCamera();

		if (SlGui::ToggleButton(ICON_AM_ORBIT" Orbit", m_UseOrbitCamera))
			UpdateCamera();
		ImGui::ToolTip("Use orbit camera instead of free");

		if (SlGui::MenuButton(ICON_AM_HOME" Reset Cam"))
			ResetCameraPosition();
	}
	SlGui::EndToolWindow();

	if (ImGui::Begin("Scene"))
	{
		ImGui::Text("Entity Handle: %u", m_ModelScene.GetEntityHandle());

		char drawablePtrBuf[64];
		sprintf_s(drawablePtrBuf, 64, "%p", m_ModelScene.GetDrawable());
		ImGui::InputText("Drawable Ptr", drawablePtrBuf, 64, ImGuiInputTextFlags_ReadOnly);

		constexpr ImVec2 buttonSize = { 90, 0 };

		if (ImGui::Button("Load", buttonSize))
		{
			m_ModelScene.SetupFor(ASSET_PATH, GetEntityScenePos());
		}

		if (ImGui::Button("Unload", buttonSize))
		{
			m_ModelScene.CleanUp();
		}

		if (ImGui::Button("Load YDR", buttonSize))
		{
			gtaDrawable* drawable;
			rage::pgRscBuilder::Load(&drawable, "C:/Users/falco/Desktop/cable1_root.ydr", 165);
			if (drawable)
			{
				m_ModelScene.SetDrawable(drawable);
				m_ModelScene.SetEntityPos(GetEntityScenePos());
			}
			else
			{
				m_ModelScene.CleanUp();
				AM_ERRF("ModelSceneApp::OnRender() -> Failed to build drawable from ydr.");
			}
		}

		if (ImGui::Button("Export", buttonSize))
		{
			using namespace asset;

			amPtr<DrawableAsset> asset = AssetFactory::LoadFromPath<DrawableAsset>(
				ASSET_PATH);

			if (asset)
				asset->CompileToFile(L"C:/Users/falco/Desktop/collider.ydr");
		}

		DrawDrawableUi(m_ModelScene.GetDrawable());
	}
	ImGui::End();
}

rageam::ModelSceneApp::ModelSceneApp()
{
	m_ModelScene.LoadCallback = [&]
		{
			UpdateDrawableStats();
		};
}

#endif