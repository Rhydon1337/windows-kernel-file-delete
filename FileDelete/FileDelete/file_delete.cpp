#include "file_delete.h"


#include "common.h"
#include "FileReference.h"
#include "handle.h"
#include "ProcessReference.h"

extern "C" POBJECT_TYPE NTAPI ObGetObjectType(_In_ PVOID Object);

NTSTATUS close_all_file_handles(const wchar_t* file_path) {
	UNREFERENCED_PARAMETER(file_path);
	PSYSTEM_HANDLE_INFORMATION all_system_handles = get_all_handles();
	for (size_t i = 0; i < all_system_handles->NumberOfHandles; i++) {
		SYSTEM_HANDLE_TABLE_ENTRY_INFO handle_info = all_system_handles->Handles[i];
		if (*IoFileObjectType == ObGetObjectType(handle_info.Object)) {
			POBJECT_NAME_INFORMATION object_name_information;
			auto status = IoQueryFileDosDeviceName((PFILE_OBJECT)handle_info.Object, &object_name_information);
			if (!NT_SUCCESS(status)) {
				continue;
			}
			if (0 == wcscmp(file_path, object_name_information->Name.Buffer)) {
				ProcessReference process;
				if (!NT_SUCCESS(process.init(handle_info.UniqueProcessId, true))) {
					continue;
				}
				NtClose((HANDLE)handle_info.HandleValue);
			}
			ExFreePool(object_name_information);
		}	
	}
	ExFreePool(all_system_handles);
	return STATUS_SUCCESS;
}

NTSTATUS send_delete_file_irp(PFILE_OBJECT file_object) {
	
}

NTSTATUS delete_file(const FileDeleteArgs& args) {
	CHECK(close_all_file_handles(args.file_path));

	wchar_t full_file_path[256] = {};
	RtlCopyMemory(full_file_path, L"\\??\\", 8);
	RtlCopyMemory(full_file_path + 4, &args.file_path, wcslen(args.file_path) * sizeof(wchar_t));
	
	FileReference file_reference;
	CHECK(file_reference.init(full_file_path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE));
	file_reference.get()->DeleteAccess = true;
	file_reference.get()->SectionObjectPointer->ImageSectionObject = nullptr;
	IoGetLowerDeviceObject(file_reference.get()->DeviceObject);
	return STATUS_SUCCESS;
}
