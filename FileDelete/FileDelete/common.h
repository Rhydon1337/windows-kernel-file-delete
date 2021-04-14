#pragma once

struct FileDeleteArgs {
	wchar_t file_path[256];
};

#define FILE_DELETE_IOCTL CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1337, METHOD_BUFFERED, FILE_WRITE_DATA)