//
// File: watcher.h
//
// Copyright (C) 2023 ranstar74. All rights violated.
//
// Part of "Rage Am" Research Project.
//
#pragma once

#include "common/types.h"
#include "common/logger.h"
#include "fileutils.h"

#include <Windows.h>

namespace rageam::file
{
	enum eWatcherFlags_
	{
		WatcherFlags_None = 0,
		WatcherFlags_Recurse = 1 << 0,
	};
	using eWatcherFlags = int;

	// Filters tracked actions on file system entry
	enum eNotifyFlags_
	{
		NotifyFlags_FileName = 1 << 0,
		NotifyFlags_DirectoryName = 1 << 1,
		NotifyFlags_Attributes = 1 << 2,
		NotifyFlags_FileSize = 1 << 3,
		NotifyFlags_LastWrite = 1 << 4,
		NotifyFlags_LastAccess = 1 << 5,

		NotifyFlags_All = (1 << 5) - 1,
	};
	using eNotifyFlags = int;

	// Action that was done to file system entry
	enum eChangeAction // Those map to WinApi FILE_ACTION_
	{
		ChangeAction_Added = 1,
		ChangeAction_Removed = 2,
		ChangeAction_Modified = 3,
		ChangeAction_Renamed = 4,
	};

	struct DirectoryChange
	{
		eChangeAction	Action;
		WPath			Path;		// Absolute path to file/directory
		WPath			NewPath;	// For ChangeAction_Renamed, path after renaming
	};

	/**
	 * \brief OS directory watcher, much faster and flexible than fiDirectoryWatcher.
	 */
	class Watcher
	{
		static constexpr size_t BUFFER_SIZE = 0x1000; // For FILE_NOTIFY_INFORMATION

		WPath				m_Path;
		HANDLE				m_DirectoryHandle = INVALID_HANDLE_VALUE;
		eWatcherFlags		m_Flags = 0;
		eNotifyFlags		m_Filter = 0;
		alignas(4) char		m_Buffer[BUFFER_SIZE] = {};
		u32					m_Offset = 0;
		bool				m_HasChanges = false;
		bool				m_ChangesRequested = false;
		OVERLAPPED			m_PollOverlap = {};
		u32					m_WaitTime;
		std::atomic_bool	m_StopRequested = false;

		bool RequestChanges()
		{
			DWORD bytesReturned;
			BOOL read = ReadDirectoryChangesW(
				m_DirectoryHandle,
				m_Buffer, BUFFER_SIZE,
				m_Flags & WatcherFlags_Recurse,
				m_Filter,
				&bytesReturned, &m_PollOverlap, NULL);

			if (!read)
			{
				AM_WARNINGF(L"file::DirectoryWatcher::ReadChanges() -> Failed to read changes. error code: %u; path: %ls",
					GetLastError(), m_Path.GetCStr());
				return false;
			}

			return true;
		}

		bool InitWatch(ConstWString path)
		{
			HANDLE hDirectory = CreateFileW(
				path,
				FILE_GENERIC_READ,
				FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
				NULL);

			if (hDirectory == INVALID_HANDLE_VALUE)
			{
				AM_WARNINGF(L"file::DirectoryWatcher::InitWatch() -> Failed to open directory. error code: %u; path: %ls",
					GetLastError(), path);
				return false;
			}

			m_PollOverlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_DirectoryHandle = hDirectory;
			m_Path = path;

			return true;
		}

	public:
		// waitTime specifies how much time (in ms) thread will wait when polling changes from OS
		Watcher(ConstWString dirPath, eNotifyFlags filter, eWatcherFlags flags = 0, u32 waitTime = INFINITY)
		{
			m_Filter = filter;
			m_Flags = flags;
			m_WaitTime = waitTime;
			InitWatch(dirPath);
		}

		~Watcher()
		{
			CloseHandle(m_DirectoryHandle);
			CloseHandle(m_PollOverlap.hEvent);
		}

		void RequestStop()
		{
			// Stop worker thread from waiting for changes and instantly exit
			m_StopRequested = true;
			SetEvent(m_PollOverlap.hEvent);
		}

		// NOTE: Current thread will be paused until change occurs!
		// Don't use in main thread or it will stuck.
		bool GetNextChange(DirectoryChange& change)
		{
			if (m_StopRequested)
				return false;

			// We are loading batch of changes from win api and iterating them one by one

			// Request changes from WinApi
			if (!m_ChangesRequested)
			{
				RequestChanges();
				m_ChangesRequested = true;
			}

			// Try to pull new changes
			if (!m_HasChanges)
			{
				// Pull changes
				if (WaitForSingleObject(m_PollOverlap.hEvent, m_WaitTime) == WAIT_TIMEOUT)
				{
					return false;
				}

				// Skip was requested, exit
				if (m_StopRequested)
				{
					return false;
				}

				m_HasChanges = true;
			}

			auto getNotifyInfo = [&](u32 offset)
				{
					char* buffer = m_Buffer + offset;
					return (FILE_NOTIFY_INFORMATION*)buffer; // NOLINT(clang-diagnostic-cast-align)	
				};

			// Get current notify info
			FILE_NOTIFY_INFORMATION* notifyInfo = getNotifyInfo(m_Offset);

			// Set offset to next info
			m_Offset += notifyInfo->NextEntryOffset;

			// Fill change
			// NOTE: Paths in FILE_NOTIFY_INFORMATION are not null terminated! Specified length is byte width
			// so we have to additionally divide it by two (because wchar_t is 2 bytes) to get actual length
			change = DirectoryChange();
			change.Action = eChangeAction(notifyInfo->Action); // Actions directly map to WinApi enum
			change.Path = m_Path; // Change path's are relative, start with absolute path to append relative later
			if (notifyInfo->Action != FILE_ACTION_RENAMED_OLD_NAME)
			{
				change.Path.Join(notifyInfo->FileName, (int)notifyInfo->FileNameLength / 2);
			}
			else
			{
				// Special case: Renaming
				// FILE_ACTION_RENAMED_NEW_NAME is always next to FILE_ACTION_RENAMED_OLD_NAME
				// We have to shift to next entry manually

				// Old name comes first
				change.Path.Join(notifyInfo->FileName, (int)notifyInfo->FileNameLength / 2);

				// Now get the new name
				// NOTE: m_Offset was already set to next item before
				notifyInfo = getNotifyInfo(m_Offset);
				change.NewPath = m_Path;
				change.NewPath.Join(notifyInfo->FileName, (int)notifyInfo->FileNameLength / 2);

				// Set offset to next item again
				m_Offset += notifyInfo->NextEntryOffset;
			}

			// Next offset 0 indicates that this was the last entry
			if (notifyInfo->NextEntryOffset == 0)
			{
				m_ChangesRequested = false;
				m_HasChanges = false;
				m_Offset = 0;
				memset(m_Buffer, 0, BUFFER_SIZE);
			}

			// WinApi sends modified 'event' for parent directory if file was changed, we don't need this
			if (change.Action == ChangeAction_Modified && file::IsDirectory(change.Path))
				return false;

			return true;
		}
	};
}
