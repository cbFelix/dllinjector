#include <Windows.h>
#include <winternl.h>

typedef NTSTATUS(NTAPI* NtCreateThreadExFunc)(
	OUT PHANDLE hThread,
	IN ACCESS_MASK DesiredAccess,
	IN PVOID ObjectAttributes,
	IN HANDLE ProcessHandle,
	IN PVOID lpStartAddress,
	IN PVOID lpParameter,
	IN ULONG Flags,
	IN SIZE_T StackZeroBits,
	IN SIZE_T SizeOfStackCommit,
	IN SIZE_T SizeOfStackReserve,
	OUT PVOID lpBytesBuffer
);

typedef NTSTATUS(NTAPI* pNtAllocateVirtualMemory)(
	HANDLE    ProcessHandle,
	PVOID* BaseAddress,
	ULONG_PTR ZeroBits,
	PSIZE_T   RegionSize,
	ULONG     AllocationType,
	ULONG     Protect
);

typedef NTSTATUS(NTAPI* pNtFreeVirtualMemory)(
	HANDLE    ProcessHandle,
	PVOID* BaseAddress,
	PSIZE_T   RegionSize,
	ULONG     FreeType
	);
