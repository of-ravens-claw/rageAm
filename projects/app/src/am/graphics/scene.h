//
// File: scene.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "vertexdeclaration.h"
#include "am/system/ptr.h"
#include "rage/atl/string.h"
#include "rage/grcore/fvf.h"
#include "rage/math/mtxv.h"
#include "rage/math/quatv.h"
#include "rage/spd/aabb.h"

namespace rageam::graphics
{
	class SceneNode;
	class SceneGeometry;
	class SceneMesh;
	class Scene;

	// Used as placeholder for unsupported semantics in loader types
	static constexpr auto INVALID_SEMANTIC = VertexSemantic(-1);

	// For geometries that don't have material
	static constexpr u16 DEFAULT_MATERIAL = u16(-1);

	/**
	 * \brief Packed vertex channel data, (for e.g. 'Position' or 'Normal') or vertex indices.
	 */
	struct SceneData
	{
		char* Buffer;
		DXGI_FORMAT Format;

		template<typename T>
		T* GetBufferAs() { return reinterpret_cast<T*>(Buffer); }
	};

	class SceneHandle
	{
	protected:
		u16 m_Index;

	public:
		SceneHandle(u16 index) { m_Index = index; }

		virtual ~SceneHandle() = default;

		u16 GetIndex() const { return m_Index; }
	};

	enum eMaterialTexture
	{ // We can extend this with PBR set when time comes...
		MT_Diffuse,
		MT_Specular,
		MT_Normal,

		MT_Unknown = -1,
	};

	/**
	 * \brief Material properties of scene geometry.
	 */
	class SceneMaterial : public SceneHandle
	{
		Scene* m_Scene;

	public:
		SceneMaterial(Scene* parent, u16 index) : SceneHandle(index)
		{
			m_Scene = parent;
		}

		virtual ConstString GetName() const = 0;
		virtual u32 GetNameHash() const = 0;

		virtual ConstString GetTextureName(eMaterialTexture texture) const = 0;
	};

	/**
	 * \brief Represents single geometry primitive of 3D model.
	 */
	class SceneGeometry : public SceneHandle
	{
		SceneMesh* m_Mesh;

	public:
		SceneGeometry(SceneMesh* parent, u16 index) : SceneHandle(index)
		{
			m_Mesh = parent;
		}

		virtual u16 GetMaterialIndex() const = 0;

		virtual u32 GetVertexCount() const = 0;
		virtual u32 GetIndexCount() const = 0;

		virtual void GetIndices(SceneData& data) const = 0;
		virtual bool GetAttribute(SceneData& data, VertexSemantic semantic, u32 semanticIndex) const = 0;

		virtual const rage::spdAABB& GetAABB() const = 0;

		bool HasSkin() const;
		SceneMesh* GetParentMesh() const { return m_Mesh; }
		SceneNode* GetParentNode() const;
	};

	/**
	 * \brief Represents a 3D model (mesh) in scene.
	 */
	class SceneMesh
	{
		Scene* m_Scene;
		SceneNode* m_Parent;

	public:
		SceneMesh(Scene* scene, SceneNode* parent)
		{
			m_Scene = scene;
			m_Parent = parent;
		}

		virtual ~SceneMesh() = default;

		// Number of geometric primitives this model is composed of,
		// usually primitives are split by material.
		virtual u16 GetGeometriesCount() const = 0;
		virtual SceneGeometry* GetGeometry(u16 index) const = 0;

		bool HasSkin() const;
		SceneNode* GetParentNode() const { return m_Parent; }
	};

	/**
	 * \brief Represents single element (node) in scene. Node has name and optionally transform.
	 */
	class SceneNode : public SceneHandle
	{
		friend class Scene;

	protected:
		SceneNode* m_Parent;
		SceneNode* m_NextSibling;
		SceneNode* m_FirstChild;
		Scene* m_Scene;

		rage::Mat44V m_LocalMatrix;
		rage::Mat44V m_WorldMatrix;

	public:
		SceneNode(Scene* scene, SceneNode* parent, u16 index) : SceneHandle(index)
		{
			m_Scene = scene;
			m_Parent = parent;

			m_NextSibling = nullptr;
			m_FirstChild = nullptr;

			m_LocalMatrix = rage::Mat44V::Identity();
			m_WorldMatrix = rage::Mat44V::Identity();
		}

		virtual ConstString GetName() const = 0;
		virtual SceneMesh* GetMesh() const = 0;

		virtual bool HasSkin() const = 0;
		// Number of skinned bones (or joints)
		virtual u16 GetBoneCount() const = 0;
		// Gets skinned bone node
		virtual SceneNode* GetBone(u16 index) = 0;
		//virtual const rage::Mat44V& GetInverseBoneMatrix(u16 index) = 0;
		virtual const rage::Mat44V& GetWorldBoneTransform(u16 index) = 0;

		bool HasMesh() const { return GetMesh() != nullptr; }

		// Called by the scene on initialization, don't use
		void ComputeMatrices();

		// So called 'XForm' in 3ds max, when model is exported without transformation applied
		// We have to create skeleton and bones for such models

		virtual bool HasTranslation() const = 0;
		virtual bool HasRotation() const = 0;
		virtual bool HasScale() const = 0;

		virtual rage::Vec3V GetTranslation() const = 0;
		virtual rage::QuatV GetRotation() const = 0;
		virtual rage::Vec3V GetScale() const = 0;

		// Translation + Rotation + Scale transformation matrix
		const rage::Mat44V& GetLocalTransform() const { return m_LocalMatrix; }
		// Combined local + parent node matrices into world transform matrix
		const rage::Mat44V& GetWorldTransform() const { return m_WorldMatrix; }

		bool HasTransform() const { return HasTranslation() || HasRotation() || HasScale(); }

		// Navigation

		SceneNode* GetParent() const { return m_Parent; }
		SceneNode* GetNextSibling() const { return m_NextSibling; }
		SceneNode* GetFirstChild() const { return m_FirstChild; }
	};

	/**
	 * \brief A Scene represents 3D model data loaded from file such as obj/fbx/glb.
	 */
	class Scene
	{
		rage::atString m_Name;

		bool m_HasTransform;
		bool m_NeedDefaultMaterial;
		bool m_HasMultipleRootNodes;
		bool m_HasSkinning;

		void FindSkinnedNodes();
		void FindTransformedModels();
		void FindNeedDefaultMaterial();
		void ScanForMultipleRootBones();
		void ComputeNodeMatrices() const;
	public:
		Scene() = default;
		virtual ~Scene() = default;

		// Must be called after loading
		virtual void Init();

		// Loads model from file at given path
		virtual bool Load(ConstWString path) = 0;

		virtual u16 GetNodeCount() const = 0;
		virtual SceneNode* GetNode(u16 index) const = 0;
		// Returns the first node on depth 0, it still may have sibling nodes
		// Use GetFirstChild / GetNextSibling / GetParent to traverse the tree
		virtual SceneNode* GetFirstNode() const = 0;

		virtual u16 GetMaterialCount() const = 0;
		virtual SceneMaterial* GetMaterial(u16 index) const = 0;
		SceneMaterial* GetMaterial(const SceneGeometry* geometry) const
		{
			return GetMaterial(geometry->GetMaterialIndex());
		}

		ConstString GetName() const { return m_Name; }
		void SetName(ConstString name) { m_Name = name; }

		// Checks whether any model geometry use default (unassigned) material
		bool NeedDefaultMaterial() const { return m_NeedDefaultMaterial; }
		// Whether there is at least one model with non-zero transform
		bool HasTransform() const { return m_HasTransform; }
		// Whether there's not single root node in scene
		bool HasMultipleRootNodes() const { return m_HasMultipleRootNodes; }
		// Whether any node in the scene use skinning
		bool HasSkinning() const { return m_HasSkinning; }
	};
	using ScenePtr = amPtr<Scene>;

	class SceneFactory
	{
	public:
		static amPtr<Scene> LoadFrom(ConstWString path);
		static bool IsSupportedFormat(ConstWString extension);
	};
}
