#include "FileReference.h"

FileReference::FileReference()
	: m_file_object(nullptr){
}

FileReference::~FileReference() {
	if (nullptr != m_file_object) {
		ObDereferenceObject(m_file_object);
	}
}

NTSTATUS FileReference::init(const wchar_t* file_path, ULONG desired_access, ULONG share_access) {
	HANDLE file_handle;
	OBJECT_ATTRIBUTES object_attributes;
	UNICODE_STRING file_path_unicode_string;
	RtlInitUnicodeString(&file_path_unicode_string, file_path);
	InitializeObjectAttributes(&object_attributes, &file_path_unicode_string, OBJ_KERNEL_HANDLE, nullptr, nullptr);
	IO_STATUS_BLOCK io_status_block;
	CHECK(ZwCreateFile(&file_handle, desired_access, &object_attributes, &io_status_block, nullptr, FILE_ATTRIBUTE_NORMAL,
		share_access, FILE_OPEN, 0, nullptr, 0));
	auto status = ObReferenceObjectByHandle(file_handle, desired_access, *IoFileObjectType, KernelMode, (PVOID*)&m_file_object, nullptr);
	ZwClose(file_handle);
	return status;
}

PFILE_OBJECT FileReference::get() const {
	return m_file_object;
}
