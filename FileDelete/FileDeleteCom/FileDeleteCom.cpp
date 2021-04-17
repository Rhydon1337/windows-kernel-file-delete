#include <Windows.h>
#include <iostream>

#include "../FileDelete/common.h"

int wmain(int, wchar_t** argv) {
	std::cout << "Hello World!\n";
	HANDLE driver = CreateFileA("\\\\.\\FileDelete", GENERIC_ALL, 0,
		nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == driver) {
		return 1;
	}
	FileDeleteArgs args = {};
	memcpy(args.file_path, argv[1], wcslen(argv[1]) * sizeof(wchar_t));

	DeviceIoControl(driver, FILE_DELETE_IOCTL, &args, sizeof(FileDeleteArgs),
		nullptr, 0, nullptr, nullptr);

	CloseHandle(driver);
	return 0;
}