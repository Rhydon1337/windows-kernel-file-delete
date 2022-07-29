#include "ProcessReference.h"

ProcessReference::ProcessReference()
	: m_process(nullptr) {
}

ProcessReference::~ProcessReference() {
	if (nullptr != m_process) {
		ObDereferenceObject(m_process);
		if (m_attach) {
			KeUnstackDetachProcess(m_apc_state);
			ExFreePool(m_apc_state);
		}
	}
	
}

NTSTATUS ProcessReference::init(size_t pid, bool attach)
{
	CHECK(PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(pid), &m_process));
	m_attach = attach;
	if (attach)
	{
		m_apc_state = (KAPC_STATE*)ExAllocatePool2(NonPagedPool, sizeof(KAPC_STATE), '2cba');
		if (NULL == m_apc_state)
			m_apc_state = (KAPC_STATE*)ExAllocatePoolZero(NonPagedPool, sizeof(KAPC_STATE), '2cba');
		if (NULL == m_apc_state)
			return STATUS_MEMORY_NOT_ALLOCATED;
		KeStackAttachProcess(m_process, m_apc_state);
	}
	return STATUS_SUCCESS;
}
