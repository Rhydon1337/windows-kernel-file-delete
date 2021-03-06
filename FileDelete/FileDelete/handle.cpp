#include "handle.h"

extern "C" NTSTATUS NTAPI ZwQuerySystemInformation(IN size_t SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL);

SYSTEM_HANDLE_INFORMATION* get_all_handles() {
    size_t handles_allocation_size = 0;
    PVOID handles_pool = nullptr;

    while (true) {
        handles_allocation_size += 0x10000;
        handles_pool = ExAllocatePool(PagedPool, handles_allocation_size);

        auto status = ZwQuerySystemInformation(SystemHandleInformation, handles_pool, (ULONG)handles_allocation_size, nullptr);
        if (status == STATUS_INFO_LENGTH_MISMATCH) {
            ExFreePool(handles_pool);
        }
        else {
            break;
        }
    }
    return (SYSTEM_HANDLE_INFORMATION*)handles_pool;
}
