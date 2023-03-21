/** @file
  This sample application bases on HelloWorld PCD setting
  to print "UEFI Hello World!" to the UEFI Console.

  Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiApplicationEntryPoint.h>



#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
//#include <Library/ShellLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DevicePathLib.h>
//#include <Library/ShellCEntryLib.h>
//#include <Library/ShellCommandLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Include/Protocol/DiskIo.h>
#include <Include/Protocol/DiskInfo.h>
#include <Include/Protocol/DriverBinding.h>
#include <Include/Protocol/ComponentName2.h>

#include <IndustryStandard/Atapi.h>
#include <IndustryStandard/Scsi.h>
#include <IndustryStandard/Nvme.h>

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/EraseBlock.h>
#include <Protocol/IdeControllerInit.h>
#include <Protocol/NvmExpressPassthru.h>

//#include "NvmExpress.h"


//
// String token ID of help message text.
// Shell supports to find help message in the resource section of an application image if
// .MAN file is not found. This global variable is added to make build tool recognizes
// that the help string is consumed by user and then build tool will add the string into
// the resource section. Thus the application can use '-?' option to show help message in
// Shell.
//
//GLOBAL_REMOVE_IF_UNREFERENCED EFI_STRING_ID  mStringHelpTokenId = STRING_TOKEN (STR_HELLO_WORLD_HELP_INFORMATION);

/**
  The user Entry Point for Application. The user code starts with this function
  as the real entry point for the application.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/


EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE        ImageHandle,
    IN EFI_SYSTEM_TABLE* SystemTable
)
{
    int Index;
    //EFI_GUID gNvmExpressPassThruProtocolGuid = EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL_GUID; 
    //EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL *ptrNvmePassThru = NULL;
    UINTN       NHandles = 0;
    EFI_HANDLE      *Handles = NULL;
    EFI_STATUS Status;

    EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL       *NvmePassthru;
    EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET  CommandPacket;
    EFI_NVM_EXPRESS_COMMAND                   Command;
    EFI_NVM_EXPRESS_COMPLETION                Completion;
    
    NVME_ADMIN_CONTROLLER_DATA               ControllerData;

    Print(L"Search Nvme devices\n");


    Status = gBS->LocateHandleBuffer (
                ByProtocol,
                &gEfiNvmExpressPassThruProtocolGuid,
                NULL,
                &NHandles,
                &Handles
                );

    Print(L"handles: %d\n");

    if (EFI_ERROR (Status)) {
          Print(L"Error LocateHandleBuffer\n");
    }

    if (Status == EFI_NOT_FOUND || NHandles == 0) {
    //
    // If there are no Protocols handles, then return EFI_NOT_FOUND
    //
                return EFI_NOT_FOUND;
    }

    for(Index = 0; Index < NHandles; Index++) {
        Status = gBS->HandleProtocol(
                        Handles[Index],
                        &gEfiNvmExpressPassThruProtocolGuid,
                        (VOID **) &NvmePassthru                      
                        );
        if (EFI_ERROR (Status)) {              
          continue;
        }else{
          Print(L"Success Handle Protocol\n");
          ZeroMem (&CommandPacket, sizeof (EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET));
          ZeroMem (&Command, sizeof (EFI_NVM_EXPRESS_COMMAND));
          ZeroMem (&Completion, sizeof (EFI_NVM_EXPRESS_COMPLETION));

          Command.Cdw0.Opcode = NVME_ADMIN_IDENTIFY_CMD;

          Command.Nsid = 0;

          CommandPacket.NvmeCmd        = &Command;
          CommandPacket.NvmeCompletion = &Completion;
          CommandPacket.TransferBuffer = &ControllerData;
          CommandPacket.TransferLength = sizeof (NVME_ADMIN_CONTROLLER_DATA);
          CommandPacket.CommandTimeout = EFI_TIMER_PERIOD_SECONDS (5);
          CommandPacket.QueueType      = NVME_ADMIN_QUEUE;

          Command.Cdw10 = 1;
          Command.Flags = CDW10_VALID;

          Status = NvmePassthru->PassThru (
                               NvmePassthru,
                               0,     // NVMe Controller ID
                               &CommandPacket,
                               NULL
                               );
  
          if (EFI_ERROR (Status)) {
            return Status;
          }else{
            Print(L"Vendor ID: %x\n", ControllerData.Vid);
          }
        }
    }

    while(1);
    return EFI_SUCCESS;
}
