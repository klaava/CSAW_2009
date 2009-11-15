// -------------------
/*
    CSAW 2009 
    Reversing Final Challenge

    S.A. Ridley
    stephen@sa7ori.org
*/
// ------------------
#include <ntddk.h>
#include "sa7ori.h"
#include "sa7_win.h"

VOID HalDisplayString(PSZ text); 
VOID InbvAcquireDisplayOwnership(VOID);
VOID InbvResetDisplay(VOID);
INT InbvSetTextColor(INT color);
VOID InbvDisplayString(PSZ text);
VOID InbvSolidColorFill(ULONG left,ULONG top,ULONG width,ULONG height,ULONG color);
VOID InbvSetScrollRegion(ULONG left,ULONG top,ULONG width,ULONG height);
VOID InbvInstallDisplayStringFilter(ULONG b);
VOID InbvEnableDisplayString(ULONG b);

char g_neo[] = "You are on the right track, just wrong code.\nFollow the White Rabbit Neo.\nKnock, Knock!\nThis isn't the flag.";
// Ciphertext/Key buffers generated by sa7encode.py
char g_kv[] = "\x65\x35\x06\x7e\x78\x47\x10\x32\x1c\x5c\x51\x49\x26\x14\x5b\x0b\x18\x39\x5c\x5e\x20\x34\x4c\x5b\x45\x1a\x45\x44\x0b\x14\x14\x12\x17\x7b\x60\x5c\x03\x38\x05\x47\x03\x09\x6b\x54\x41\x63\x67\x4f\x4f\x19\x4e\x60\x33\x65\x59\x07\x40\x70\x01\x30\x26\x3c\x2b\x1c\x21\x68\x74\x3e\x52\x04";
char g_ctv[] = "\xa2\x73\x2c\xb9\x1c\x63\xc5\xb8\x96\x54\x87\x41\x72\x01\x31\xee\xb8\x19\x3c\x99\x30\xc5\x57\xee\xc7\xec\x6d\x32\x88\xed\x11\x5f\x04\x3c\x0a\xd6\x9e\xf2\xd2\x03\x51\xf0\xc3\x2f\x66\xa2\xe5\x30\xf1\x61\xa2\xa4\x46\x14\x39\x9f\xdd\xfb\xe4\xc8\x95\x2c\xf9\x9d\xa7\xa2\x19\x79\x8f\xbf"; 
char g_kv2[] = "\x53\x48\x08\x5b\x5d\x79\x70\x09\x49\x7f\x48\x1b\x28\x43\x35\x51\x39\x10\x0d\x56\x43\x3e\x30\x5f\x27\x27\x48\x16\x08\x2e\x08\x48\x57\x4a\x7b\x4f\x7e\x5e\x03\x28\x6c\x49\x09\x35\x3a\x7a\x12\x60\x29\x60\x1b\x69\x1d\x75\x3b\x18\x5d\x6c\x7a";
char g_ctv2[] = "\xe7\xc5\x66\x96\x6e\xa8\x31\xa8\x84\x4e\x7b\x8b\x8b\xf9\x8e\x1d\x0c\xb4\xbd\x25\x3a\x6e\x2f\x1d\x63\xc9\x93\x81\x74\x6a\x8b\x5d\x97\x7e\x6a\x02\x79\x2d\x99\xb2\x3f\x2c\x5c\xd1\xd5\x17\x57\x2e\xad\x1b\xb2\x03\x9e\x4a\x95\xfa\x9e\x1c\x97";

void taunt()
{
    InbvAcquireDisplayOwnership(); //Takes control of screen
    InbvResetDisplay(); //Clears screen
    InbvSolidColorFill(0,0,639,479,4); //Colors the screen blue
    InbvSetTextColor(15); //Sets text color to white
    InbvInstallDisplayStringFilter(0); //Not sure but nessecary
    InbvEnableDisplayString(1); //Enables printing text to screen
    InbvSetScrollRegion(0,0,639,475); //Not sure, would recommend keeping
    InbvDisplayString("\n\n\n\tKnock Knock Neo. Follow The White Rabbit.\n\tNice try, but nope thats the wrong IOCTL.\n"); //Prints text
//    HalDisplayString("And so do I!!!"); //Prints text
}

NTSTATUS SA7_IoControl(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp)
{
	PIO_STACK_LOCATION irpStack;
	PVOID inBuf, outBuf;
	ULONG inLen, outLen;
	ULONG ioControl;
	NTSTATUS status = STATUS_SUCCESS;
	ULONG information = 0;
    char *ct; 
    char *key; 
    sa7_key *keyptr;
       
	irpStack = IoGetCurrentIrpStackLocation(Irp);
	inBuf = Irp->AssociatedIrp.SystemBuffer;
	inLen = irpStack->Parameters.DeviceIoControl.InputBufferLength;
	outBuf = Irp->AssociatedIrp.SystemBuffer;
	outLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
	ioControl = irpStack->Parameters.DeviceIoControl.IoControlCode;

	switch(ioControl)
	{

	case IOCTL_GET_VERSION:
        key=(char *)&g_kv;
        ct=(char *)&g_ctv;
		DbgPrint("IOCTL_GET_VERSION...Congrats!\n");
        keyptr = (sa7_key *) ExAllocatePool(PagedPool, sizeof(sa7_key));
        prepare_key((unsigned char *)key, strlen(key), keyptr);
        sa7((unsigned char *)ct, strlen(ct), keyptr);
		inLen = strlen(ct);
		if(outLen < inLen + 1)
		{
			status = STATUS_BUFFER_TOO_SMALL;
            ExFreePool(keyptr);
			break;
		} 
		RtlCopyMemory(outBuf, (char *)ct, inLen);
		((char*)outBuf)[inLen] = 0;
		information = inLen + 1;
        ExFreePool(keyptr);
		break;

	case IOCTL_TAUNT: //this uses a decryption routines to copy back to them an
                      //and incorrect string
        key=(char *)&g_kv2;
        ct=(char *)&g_ctv2;
		DbgPrint("IOCTL_TAUNT...Good job!, a correct IOCTL.\n");
        keyptr = (sa7_key *) ExAllocatePool(PagedPool, sizeof(sa7_key));
        prepare_key((unsigned char *)key, strlen(key), keyptr);
        sa7((unsigned char *)ct, strlen(ct), keyptr);
		inLen = strlen(ct);
		if(outLen < inLen + 1)
		{
			status = STATUS_BUFFER_TOO_SMALL;
            ExFreePool(keyptr);
			break;
		} 
		RtlCopyMemory(outBuf, (char *)ct, inLen);
		((char*)outBuf)[inLen] = 0;
		information = inLen + 1;
        ExFreePool(keyptr);
		break;

	case IOCTL_SAYHI: //This just copies a string back to them from globals. an incorrect one.
        #include "evil.h"
		DbgPrint("IOCTL_SAYHI...Good job, that's a correct IOCTL!\n");
		inLen = strlen(g_neo);
		if(outLen < inLen + 1)
		{
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		} 
		RtlCopyMemory(outBuf, (char *)&g_neo, inLen);
		((char*)outBuf)[inLen] = 0;
		information = inLen + 1;
		break;

	default:
		DbgPrint("Unknown IOCTL\n");
        taunt();
		status = STATUS_INVALID_DEVICE_REQUEST;
	}

	// complete IRP
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = information;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}

void SA7_Unload(IN PDRIVER_OBJECT DriverObject)
{
    DbgPrint("Unloading...\n");
}

NTSTATUS SA7_StubDispatch(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

PDEVICE_OBJECT g_devObj;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;
	UNICODE_STRING devName, devLink;
	int i;

	DbgPrint("Sa7ori is loaded....\n");

	RtlInitUnicodeString(&devName, L"\\Device\\sa7ori");
	RtlInitUnicodeString(&devLink, L"\\DosDevices\\sa7ori");

    #include "evil.h"
	status = IoCreateDevice(DriverObject,
							0,
							&devName,
							SA7ORICDE,
							0,
							TRUE,
							&g_devObj);
	if(!NT_SUCCESS(status))
	{
		IoDeleteDevice(DriverObject->DeviceObject);
		DbgPrint("Failed to create device\n");
		return status;
	}

	status = IoCreateSymbolicLink(&devLink, &devName);
	if(!NT_SUCCESS(status))
	{
		IoDeleteDevice(DriverObject->DeviceObject);
		DbgPrint("Failed to create symbolic link\n");
		return status;
	}

	for(i=0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverObject->MajorFunction[i] = SA7_StubDispatch;
	}
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = SA7_IoControl;
	DriverObject->DriverUnload = SA7_Unload;
	
	return STATUS_SUCCESS;
}