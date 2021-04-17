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


NTSTATUS io_complete(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context) {
	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Context);
	Irp->UserIosb->Status = Irp->IoStatus.Status;
	Irp->UserIosb->Information = Irp->IoStatus.Information;

	KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, false);
	IoFreeIrp(Irp);
	
	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS send_delete_file_irp(PFILE_OBJECT file_object) {
	KEVENT event;
	PDEVICE_OBJECT device_object = IoGetBaseFileSystemDeviceObject(file_object);

	PIRP irp = IoAllocateIrp(device_object->StackSize, false);

	// Set the complete routine that will free the IRP and signal the event
	KeInitializeEvent(&event, SynchronizationEvent, false);
	IoSetCompletionRoutine(
		irp,
		io_complete,
		&event,
		true,
		true,
		true);

	FILE_DISPOSITION_INFORMATION file_disposition;
	file_disposition.DeleteFile = true;

	IO_STATUS_BLOCK io_status_block;

	irp->AssociatedIrp.SystemBuffer = &file_disposition;
	irp->UserEvent = &event;
	irp->UserIosb = &io_status_block;
	irp->Tail.Overlay.OriginalFileObject = file_object;
	irp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
	irp->RequestorMode = KernelMode;
	
	IO_STACK_LOCATION* stack_location = IoGetNextIrpStackLocation(irp);
	stack_location->MajorFunction = IRP_MJ_SET_INFORMATION;
	stack_location->DeviceObject = device_object;
	stack_location->FileObject = file_object;
	stack_location->Parameters.SetFile.Length = sizeof(FILE_DISPOSITION_INFORMATION);
	stack_location->Parameters.SetFile.FileInformationClass = FileDispositionInformation;
	stack_location->Parameters.SetFile.FileObject = file_object;

	IoCallDriver(device_object, irp);
	KeWaitForSingleObject(&event, Executive, KernelMode, true, nullptr);

	return STATUS_SUCCESS;
}

NTSTATUS delete_file(const FileDeleteArgs& args) {
	CHECK(close_all_file_handles(args.file_path));

	wchar_t full_file_path[256] = {};
	RtlCopyMemory(full_file_path, L"\\??\\", 8);
	RtlCopyMemory(full_file_path + 4, &args.file_path, wcslen(args.file_path) * sizeof(wchar_t));
	
	FileReference file_reference;
	CHECK(file_reference.init(full_file_path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE));
	file_reference.get_object()->DeleteAccess = true;

	auto section_object = file_reference.get_object()->SectionObjectPointer;
	PVOID image_section_object = nullptr;
	PVOID data_section_object = nullptr;
	PVOID shared_cache_map = nullptr;
	
	if (nullptr != section_object) {
		image_section_object = file_reference.get_object()->SectionObjectPointer->ImageSectionObject;
		data_section_object = file_reference.get_object()->SectionObjectPointer->DataSectionObject;
		shared_cache_map = file_reference.get_object()->SectionObjectPointer->SharedCacheMap;
	}

	auto status = send_delete_file_irp(file_reference.get_object());

	if (nullptr != section_object) {
		file_reference.get_object()->SectionObjectPointer->ImageSectionObject = image_section_object;
		file_reference.get_object()->SectionObjectPointer->DataSectionObject = data_section_object;
		file_reference.get_object()->SectionObjectPointer->SharedCacheMap = shared_cache_map;
	}
	
	return status;
}
