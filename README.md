# windows-kernel-file-delete

## TL;DR
Force a file delete using a windows kernel driver

Tested on Windows x64 1909

## How its works
The deletion process is divided into several stages:

1. Close all the file handles
  * Get all the handles in the system (ZwQuerySystemInformation and SystemHandleInformation)
  * Check every handle if the handle is associated to IoFileObjectType (ObGetObjectType)
  * Compare the handle file name to target file name (IoQueryFileDosDeviceName)
  * If its the target file We attach to target process (KeStackAttachProcess)
  * Close the handle (NtClose)
  * Detach from target process (KeUnstackDetachProcess)
2. Get a reference to the file (ZwCreateFile and ObReferenceObjectByHandle)
3. Special treatment for executables, set the FileObject SectionObjectPointer->ImageSectionObject, SectionObjectPointer->DataSectionObject and SectionObjectPointer->SharedCacheMap to null in order to mark that the file is not mapped
4. Send IRP to the file system driver which marks the file for deletion
  * Get the file system device object (IoGetBaseFileSystemDeviceObject)
  * Allocate IRP and fill it with IRP_MJ_SET_INFORMATION and FILE_DISPOSITION_INFORMATION (IoAllocateIrp, KeInitializeEvent and IoSetCompletionRoutine)
  * Send the IRP to the file system device object (IoCallDriver and KeWaitForSingleObject)
5. Restore SectionObjectPointer->ImageSectionObject, SectionObjectPointer->DataSectionObject and SectionObjectPointer->SharedCacheMap fields
6. Close the file reference in order to delete it (ZwClose and ObDereferenceObject)

## Usage

sc create FileDelete binPath= {driver_path} type= kernel

sc start FileDelete

FileDeleteCom.exe {file_path}

DONE!!!
