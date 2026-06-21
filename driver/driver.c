/**
 * Stealth Kernel Driver
 *
 * Provides Ring-0 memory operations that are invisible to user-mode
 * anti-cheats (Hyperion/Byfron, QSec, etc.).
 *
 * Uses MmCopyVirtualMemory for cross-process memory access, which
 * operates entirely in kernel mode and cannot be intercepted by
 * user-mode hooks or Instrumentation Callbacks.
 *
 * Communication: IOCTL via \\.\StealthDrv
 * 
 * Build: WDK + MSBuild
 *   msbuild /p:Configuration=Release /p:Platform=x64 driver.vcxproj
 */

#include <ntifs.h>
#include <ntstrsafe.h>
#include "driver.h"

// ======================================================================
// Device names
// ======================================================================
#define DEVICE_NAME      L"\\Device\\StealthDrv"
#define SYMLINK_NAME     L"\\DosDevices\\StealthDrv"
#define DEVICE_DOS_NAME  L"\\\\.\\StealthDrv"

// ======================================================================
// Forward declarations
// ======================================================================
DRIVER_UNLOAD   DriverUnload;
DRIVER_DISPATCH DriverCreateClose;
DRIVER_DISPATCH DriverControl;

// ======================================================================
// Helper: Read memory from target process
// ======================================================================
NTSTATUS ReadProcessMemory(
    HANDLE  pid,
    ULONG64 address,
    PVOID   buffer,
    ULONG   size,
    SIZE_T* bytes_read
) {
    if (!pid || !address || !buffer || !size)
        return STATUS_INVALID_PARAMETER;

    PEPROCESS target_process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(pid, &target_process);
    if (!NT_SUCCESS(status))
        return status;

    status = MmCopyVirtualMemory(
        target_process, (PVOID)address,   // source = target process
        PsGetCurrentProcess(),            // dest   = our process (the driver is in System context,
                                          //          but we impersonate the caller)
        buffer,                           // buffer
        size,                             // size
        KernelMode,
        bytes_read
    );

    ObDereferenceObject(target_process);
    return status;
}

// ======================================================================
// Helper: Write memory to target process
// ======================================================================
NTSTATUS WriteProcessMemory(
    HANDLE  pid,
    ULONG64 address,
    PVOID   buffer,
    ULONG   size,
    SIZE_T* bytes_written
) {
    if (!pid || !address || !buffer || !size)
        return STATUS_INVALID_PARAMETER;

    PEPROCESS target_process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(pid, &target_process);
    if (!NT_SUCCESS(status))
        return status;

    status = MmCopyVirtualMemory(
        PsGetCurrentProcess(), buffer,    // source = our buffer
        target_process, (PVOID)address,   // dest   = target process
        size,
        KernelMode,
        bytes_written
    );

    ObDereferenceObject(target_process);
    return status;
}

// ======================================================================
// Helper: Find process ID by name
// ======================================================================
HANDLE FindProcessId(const WCHAR* process_name) {
    if (!process_name) return NULL;

    HANDLE pid = NULL;
    PEPROCESS process = NULL;
    NTSTATUS status;

        // Walk the process list
    for (ULONG i = 4; ; i += 4) {
        status = PsLookupProcessByProcessId((HANDLE)(ULONG_PTR)i, &process);
        if (status == STATUS_INVALID_PARAMETER)
            break;
        if (!NT_SUCCESS(status))
            continue;

        // Get process image filename (ANSI)
        const char* imageName = PsGetProcessImageFileName(process);
        if (imageName) {
            if (_stricmp(imageName, process_name) == 0) {
                pid = (HANDLE)(ULONG_PTR)i;
                ObDereferenceObject(process);
                break;
            }
        }
        ObDereferenceObject(process);
    }

    return pid;
}

// ======================================================================
// Helper: Find module base in a process
// NOTE: This is a simplified stub. The usermode client provides
// the module base directly via OpenProcess + CreateToolhelp32Snapshot,
// which is more reliable than kernel-side module enumeration.
// For full kernel enumeration, walk PPEB->Ldr of the target process.
// ======================================================================
ULONG64 FindModuleBase(HANDLE pid, const WCHAR* module_name, PULONG module_size) {
    UNREFERENCED_PARAMETER(pid);
    UNREFERENCED_PARAMETER(module_name);
    // Stub: usermode provides module base directly
    if (module_size) *module_size = 0;
    return 0;
}

// ======================================================================
// DriverEntry
// ======================================================================
NTSTATUS DriverEntry(PDRIVER_OBJECT driver_object, PUNICODE_STRING registry_path) {
    UNREFERENCED_PARAMETER(registry_path);
    NTSTATUS status;
    PDEVICE_OBJECT device_object = NULL;

    // Create device
    UNICODE_STRING device_name;
    RtlInitUnicodeString(&device_name, DEVICE_NAME);

    status = IoCreateDevice(
        driver_object,
        0,
        &device_name,
        FILE_DEVICE_UNKNOWN,
        0,
        FALSE,
        &device_object
    );

    if (!NT_SUCCESS(status)) {
        DbgPrint("[StealthDrv] IoCreateDevice failed: 0x%X\n", status);
        return status;
    }

    // Create symbolic link
    UNICODE_STRING symlink_name;
    RtlInitUnicodeString(&symlink_name, SYMLINK_NAME);

    status = IoCreateSymbolicLink(&symlink_name, &device_name);
    if (!NT_SUCCESS(status)) {
        DbgPrint("[StealthDrv] IoCreateSymbolicLink failed: 0x%X\n", status);
        IoDeleteDevice(device_object);
        return status;
    }

    // Set dispatch routines
    driver_object->MajorFunction[IRP_MJ_CREATE] = DriverCreateClose;
    driver_object->MajorFunction[IRP_MJ_CLOSE]  = DriverCreateClose;
    driver_object->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverControl;
    driver_object->DriverUnload = DriverUnload;

    // Set device flags
    device_object->Flags |= DO_DIRECT_IO;
    device_object->Flags &= ~DO_DEVICE_INITIALIZING;

    DbgPrint("[StealthDrv] Driver loaded successfully\n");
    return STATUS_SUCCESS;
}

// ======================================================================
// DriverUnload
// ======================================================================
VOID DriverUnload(PDRIVER_OBJECT driver_object) {
    // Delete symbolic link
    UNICODE_STRING symlink_name;
    RtlInitUnicodeString(&symlink_name, SYMLINK_NAME);
    IoDeleteSymbolicLink(&symlink_name);

    // Delete device
    IoDeleteDevice(driver_object->DeviceObject);

    DbgPrint("[StealthDrv] Driver unloaded\n");
}

// ======================================================================
// Create/Close dispatch
// ======================================================================
NTSTATUS DriverCreateClose(PDEVICE_OBJECT device_object, PIRP irp) {
    UNREFERENCED_PARAMETER(device_object);

    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = 0;
    IoCompleteRequest(irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

// ======================================================================
// IOCTL dispatch
// ======================================================================
NTSTATUS DriverControl(PDEVICE_OBJECT device_object, PIRP irp) {
    UNREFERENCED_PARAMETER(device_object);

    NTSTATUS status = STATUS_SUCCESS;
    ULONG info = 0;

    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
    ULONG ioctl_code = stack->Parameters.DeviceIoControl.IoControlCode;

    PVOID input_buffer = irp->AssociatedIrp.SystemBuffer;
    ULONG input_size   = stack->Parameters.DeviceIoControl.InputBufferLength;
    PVOID output_buffer = irp->AssociatedIrp.SystemBuffer;  // METHOD_BUFFERED
    ULONG output_size  = stack->Parameters.DeviceIoControl.OutputBufferLength;

    switch (ioctl_code) {

    case IOCTL_STEALTH_PING:
        // Simple ping to verify driver is loaded
        DbgPrint("[StealthDrv] Ping received\n");
        status = STATUS_SUCCESS;
        info = 0;
        break;

    case IOCTL_STEALTH_READ_MEMORY: {
        // Read memory from target process
        if (input_size < sizeof(StealthReadRequest) || !input_buffer) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        StealthReadRequest* req = (StealthReadRequest*)input_buffer;
        ULONG read_size = min(req->Size, output_size - sizeof(StealthReadRequest));

        SIZE_T bytes_read = 0;
        status = ReadProcessMemory(
            (HANDLE)req->ProcessId,
            req->Address,
            req->Data,     // Output starts at req->Data
            read_size,
            &bytes_read
        );

        if (NT_SUCCESS(status))
            info = sizeof(StealthReadRequest) + (ULONG)bytes_read;
        else
            info = 0;

        DbgPrint("[StealthDrv] Read: PID=%llu Addr=0x%llX Size=%lu -> 0x%X (%zu bytes)\n",
            req->ProcessId, req->Address, read_size, status, bytes_read);
        break;
    }

    case IOCTL_STEALTH_WRITE_MEMORY: {
        // Write memory to target process
        if (input_size < sizeof(StealthWriteRequest) || !input_buffer) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        StealthWriteRequest* req = (StealthWriteRequest*)input_buffer;
        ULONG write_size = min(req->Size, input_size - sizeof(StealthWriteRequest));

        SIZE_T bytes_written = 0;
        status = WriteProcessMemory(
            (HANDLE)req->ProcessId,
            req->Address,
            req->Data,     // Input starts at req->Data
            write_size,
            &bytes_written
        );

        if (NT_SUCCESS(status))
            info = sizeof(StealthWriteRequest);
        else
            info = 0;

        DbgPrint("[StealthDrv] Write: PID=%llu Addr=0x%llX Size=%lu -> 0x%X (%zu bytes)\n",
            req->ProcessId, req->Address, write_size, status, bytes_written);
        break;
    }

    case IOCTL_STEALTH_GET_PID: {
        // Get PID by process name
        if (input_size < sizeof(StealthPidRequest) || !input_buffer) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        StealthPidRequest* req = (StealthPidRequest*)input_buffer;
        req->ProcessId = (ULONG64)(ULONG_PTR)FindProcessId(req->ProcessName);

        if (req->ProcessId)
            status = STATUS_SUCCESS;
        else
            status = STATUS_NOT_FOUND;

        info = sizeof(StealthPidRequest);
        break;
    }

    case IOCTL_STEALTH_GET_MODULE: {
        // Get module base (simplified - caller usually knows it)
        if (input_size < sizeof(StealthModuleRequest) || !input_buffer) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        StealthModuleRequest* req = (StealthModuleRequest*)input_buffer;
        req->BaseAddress = FindModuleBase(
            (HANDLE)req->ProcessId,
            req->ModuleName,
            &req->ModuleSize
        );

        if (req->BaseAddress)
            status = STATUS_SUCCESS;
        else
            status = STATUS_NOT_FOUND;

        info = sizeof(StealthModuleRequest);
        break;
    }

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    // Complete the request
    irp->IoStatus.Status = status;
    irp->IoStatus.Information = info;
    IoCompleteRequest(irp, IO_NO_INCREMENT);

    return status;
}
