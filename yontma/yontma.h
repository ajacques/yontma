
#define SERVICE_NAME TEXT("yontma")
#define SERVICE_DISPLAY_NAME TEXT("You'll Never Take Me Alive! (YoNTMA)")
#define SERVICE_FRIENDLY_DESCRIPTION TEXT("Protects the data on your laptop by automatically hibernating the machine when the screen is locked and wired Ethernet or AC power is disconnected.")
#define DEFAULT_SLEEP_TIME 500

#define CMD_PARAM_INSTALL               TEXT("-i")
#define CMD_PARAM_UNINSTALL             TEXT("-u")
#define CMD_PARAM_FORCE_INSTALL_SHORT   TEXT("-f")
#define CMD_PARAM_FORCE_INSTALL_LONG    TEXT("--force")
#define CMD_PARAM_RUN_AS_SERVICE        TEXT("as_svc")
#define CMD_PARAM_STARTED_FROM_SS       TEXT("started_from_ss")

typedef struct _PARTITION_TABLE {
    BYTE Status;
    BYTE Ignore1[3];
    BYTE Type;
    BYTE Ignore2[3];
    DWORD FirstSector;
    DWORD NumberOfSectors;
} PARTITION_TABLE, *PPARTITION_TABLE;

HRESULT CheckYontmaRequirements(__in BOOL bSkipEncryptionCheck);

HRESULT InstallYontma(void);
HRESULT RemoveYontma(void);

HRESULT IsUserAdmin(__out PBOOL isAdmin);

HRESULT ProcessCommandLine(int argc, _TCHAR* argv[]);
HRESULT PerformInstall(__in BOOL bSkipEncryptionCheck);
HRESULT PerformUninstall(void);
void PerformRunAsService(void);
void PrintUsage(void);