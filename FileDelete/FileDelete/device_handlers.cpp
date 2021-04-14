#include "device_handlers.h"

#include "common.h"
#include "file_delete.h"

NTSTATUS device_create_close(PDEVICE_OBJECT device_object, PIRP irp) {
	UNREFERENCED_PARAMETER(device_object);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;

	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS device_ioctl(PDEVICE_OBJECT device_object, PIRP irp) {
	UNREFERENCED_PARAMETER(device_object);

	NTSTATUS nt_status;
	PIO_STACK_LOCATION  irp_stack_location = IoGetCurrentIrpStackLocation(irp);
	size_t input_buffer_length = irp_stack_location->Parameters.DeviceIoControl.InputBufferLength;

	switch (irp_stack_location->Parameters.DeviceIoControl.IoControlCode) {
	case FILE_DELETE_IOCTL:
	{
		if (input_buffer_length != sizeof FileDeleteArgs) {
			nt_status = STATUS_INVALID_PARAMETER;
			break;
		}
		auto args = static_cast<FileDeleteArgs*>(irp->AssociatedIrp.SystemBuffer);
		nt_status = delete_file(*args);
	}
	break;
	default: 
	{
		nt_status = STATUS_INVALID_DEVICE_REQUEST;
	}
	break;
	}

	irp->IoStatus.Status = nt_status;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return nt_status;
}
