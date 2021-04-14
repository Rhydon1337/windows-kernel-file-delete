#pragma once
#include <ntifs.h>

#include "common.h"

NTSTATUS delete_file(const FileDeleteArgs& args);
