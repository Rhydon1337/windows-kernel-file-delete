#pragma once

#include <ntifs.h>

#include "consts.h"

class FileReference {
public:
	FileReference();
	~FileReference();

	NTSTATUS init(const wchar_t* file_path, ULONG desired_access, ULONG share_access);

	PFILE_OBJECT get_object() const;

	HANDLE get_handle() const;
	
	DELETE_DEFAULT_CTORS(FileReference);
private:
	PFILE_OBJECT m_file_object;
	HANDLE m_file_handle;
};