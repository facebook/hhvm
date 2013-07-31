/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_RUNTIME_OPTION_H_
#define incl_HPHP_RUNTIME_OPTION_H_

#include <boost/container/flat_set.hpp>
#include "hphp/runtime/server/files_match.h"
#include "hphp/runtime/server/satellite_server.h"
#include "hphp/runtime/server/virtual_host.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class AccessLogFileData;
/**
 * Configurable options set from command line or configurable file at startup
 * time.
 */
class RuntimeOption {
public:
  static void Load(Hdf &config, StringVec *overwrites = nullptr,
                   bool empty = false);

  static bool Loaded;

  static bool ServerExecutionMode() {
    return strcmp(ExecutionMode, "srv") == 0;
  }

  static bool ClientExecutionMode() {
    return strcmp(ExecutionMode, "cli") == 0;
  }

  static const char *ExecutionMode;
  static std::string BuildId;
  static std::string PidFile;

  static std::string LogFile;
  static std::string LogFileSymLink;
  static int LogHeaderMangle;
  static bool AlwaysEscapeLog;
  static bool AlwaysLogUnhandledExceptions;
  static bool InjectedStackTrace;
  static int InjectedStackTraceLimit; // limit the size of the backtrace
  static bool NoSilencer;
  static bool EnableApplicationLog;
  static bool CallUserHandlerOnFatals;
  static bool ThrowExceptionOnBadMethodCall;
  static int RuntimeErrorReportingLevel;

  static std::string ServerUser; // run server under this user account

  static int  MaxLoopCount;
  static int  MaxSerializedStringSize;
  static bool NoInfiniteRecursionDetection;
  static bool ThrowBadTypeExceptions;
  static bool ThrowTooManyArguments;
  static bool WarnTooManyArguments;
  static bool ThrowMissingArguments;
  static bool ThrowInvalidArguments;
  static bool EnableHipHopErrors;
  static bool AssertActive;
  static bool AssertWarning;
  static int NoticeFrequency; // output 1 out of NoticeFrequency notices
  static int WarningFrequency;
  static int64_t SerializationSizeLimit;
  static int64_t StringOffsetLimit;

  static std::string AccessLogDefaultFormat;
  static std::vector<AccessLogFileData> AccessLogs;

  static std::string AdminLogFormat;
  static std::string AdminLogFile;
  static std::string AdminLogSymLink;

  static std::string Tier;
  static std::string Host;
  static std::string DefaultServerNameSuffix;
  static std::string ServerType;
  static std::string ServerIP;
  static std::string ServerPrimaryIP;
  static int ServerPort;
  static int ServerPortFd;
  static int ServerBacklog;
  static int ServerConnectionLimit;
  static int ServerThreadCount;
  static int ServerWarmupThrottleRequestCount;
  static bool ServerThreadRoundRobin;
  static int ServerThreadDropCacheTimeoutSeconds;
  static int ServerThreadJobLIFOSwitchThreshold;
  static int ServerThreadJobMaxQueuingMilliSeconds;
  static bool ServerThreadDropStack;
  static bool ServerHttpSafeMode;
  static bool ServerStatCache;
  static std::vector<std::string> ServerWarmupRequests;
  static boost::container::flat_set<std::string> ServerHighPriorityEndPoints;
  static int PageletServerThreadCount;
  static bool PageletServerThreadRoundRobin;
  static int PageletServerThreadDropCacheTimeoutSeconds;
  static int PageletServerQueueLimit;
  static bool PageletServerThreadDropStack;

  static int FiberCount;
  static int RequestTimeoutSeconds;
  static size_t ServerMemoryHeadRoom;
  static int64_t RequestMemoryMaxBytes;
  static int64_t ImageMemoryMaxBytes;
  static int ResponseQueueCount;
  static int ServerGracefulShutdownWait;
  static int ServerDanglingWait;
  static bool ServerHarshShutdown;
  static bool ServerEvilShutdown;
  static int ServerShutdownListenWait;
  static int ServerShutdownListenNoWork;
  static int GzipCompressionLevel;
  static std::string ForceCompressionURL;
  static std::string ForceCompressionCookie;
  static std::string ForceCompressionParam;
  static bool EnableMagicQuotesGpc;
  static bool EnableKeepAlive;
  static bool ExposeHPHP;
  static bool ExposeXFBServer;
  static bool ExposeXFBDebug;
  static std::string XFBDebugSSLKey;
  static int ConnectionTimeoutSeconds;
  static bool EnableOutputBuffering;
  static std::string OutputHandler;
  static bool ImplicitFlush;
  static bool EnableEarlyFlush;
  static bool ForceChunkedEncoding;
  static int64_t MaxPostSize;
  static bool AlwaysPopulateRawPostData;
  static int64_t UploadMaxFileSize;
  static std::string UploadTmpDir;
  static bool EnableFileUploads;
  static bool EnableUploadProgress;
  static int Rfc1867Freq;
  static std::string Rfc1867Prefix;
  static std::string Rfc1867Name;
  static bool LibEventSyncSend;
  static bool ExpiresActive;
  static int ExpiresDefault;
  static std::string DefaultCharsetName;
  static bool ForceServerNameToHeader;
  static bool EnableCufAsync;
  static VirtualHostPtrVec VirtualHosts;
  static IpBlockMapPtr IpBlocks;
  static SatelliteServerInfoPtrVec SatelliteServerInfos;

  // If a request has a body over this limit, switch to on-demand reading.
  // -1 for no limit.
  static int RequestBodyReadLimit;

  static bool EnableSSL;
  static int SSLPort;
  static int SSLPortFd;
  static std::string SSLCertificateFile;
  static std::string SSLCertificateKeyFile;
  static std::string SSLCertificateDir;
  static bool TLSDisableTLS1_2;

  static int XboxServerThreadCount;
  static int XboxServerMaxQueueLength;
  static int XboxServerPort;
  static int XboxDefaultLocalTimeoutMilliSeconds;
  static int XboxDefaultRemoteTimeoutSeconds;
  static int XboxServerInfoMaxRequest;
  static int XboxServerInfoDuration;
  static std::string XboxServerInfoWarmupDoc;
  static std::string XboxServerInfoReqInitFunc;
  static std::string XboxServerInfoReqInitDoc;
  static bool XboxServerInfoAlwaysReset;
  static bool XboxServerLogInfo;
  static std::string XboxProcessMessageFunc;
  static std::string XboxPassword;
  static std::set<std::string> XboxPasswords;

  static std::string SourceRoot;
  static std::vector<std::string> IncludeSearchPaths;
  static std::string FileCache;
  static std::string DefaultDocument;
  static std::string ErrorDocument404;
  static bool ForbiddenAs404;
  static std::string ErrorDocument500;
  static std::string FatalErrorMessage;
  static std::string FontPath;
  static bool EnableStaticContentCache;
  static bool EnableStaticContentFromDisk;
  static bool EnableOnDemandUncompress;
  static bool EnableStaticContentMMap;

  static bool Utf8izeReplace;

  static std::string StartupDocument;
  static std::string WarmupDocument;
  static std::string RequestInitFunction;
  static std::string RequestInitDocument;
  static std::vector<std::string> ThreadDocuments;
  static std::vector<std::string> ThreadLoopDocuments;

  static bool SafeFileAccess;
  static std::vector<std::string> AllowedDirectories;
  static std::set<std::string> AllowedFiles;
  static hphp_string_imap<std::string> StaticFileExtensions;
  static std::set<std::string> ForbiddenFileExtensions;
  static std::set<std::string> StaticFileGenerators;
  static FilesMatchPtrVec FilesMatches;

  static bool WhitelistExec;
  static bool WhitelistExecWarningOnly;
  static std::vector<std::string> AllowedExecCmds;

  static bool UnserializationWhitelistCheck;
  static bool UnserializationWhitelistCheckWarningOnly;

  static std::string TakeoverFilename;
  static int AdminServerPort;
  static int AdminThreadCount;
  static std::string AdminPassword;
  static std::set<std::string> AdminPasswords;

  static std::string ProxyOrigin;
  static int ProxyRetry;
  static bool UseServeURLs;
  static std::set<std::string> ServeURLs;
  static bool UseProxyURLs;
  static int ProxyPercentage;
  static std::set<std::string> ProxyURLs;
  static std::vector<std::string> ProxyPatterns;
  static bool AlwaysUseRelativePath;

  static bool MySQLReadOnly;
#ifdef FACEBOOK
  static bool MySQLLocalize;  // whether to localize MySQL query results
#endif
  static int  MySQLConnectTimeout;
  static int  MySQLReadTimeout;
  static int  MySQLWaitTimeout;
  static int  MySQLSlowQueryThreshold;
  static bool MySQLKillOnTimeout;
  static int  MySQLMaxRetryOpenOnFail;
  static int  MySQLMaxRetryQueryOnFail;
  static std::string MySQLSocket;

  static int  HttpDefaultTimeout;
  static int  HttpSlowQueryThreshold;

  static bool TranslateLeakStackTrace;
  static bool NativeStackTrace;
  static bool FullBacktrace;
  static bool ServerStackTrace;
  static bool ServerErrorMessage;
  static bool TranslateSource;
  static bool RecordInput;
  static bool ClearInputOnSuccess;
  static std::string ProfilerOutputDir;
  static std::string CoreDumpEmail;
  static bool CoreDumpReport;
  static std::string CoreDumpReportDirectory;
  static bool LocalMemcache;
  static bool MemcacheReadOnly;

  static bool EnableStats;
  static bool EnableWebStats;
  static bool EnableMemoryStats;
  static bool EnableMallocStats;
  static bool EnableAPCStats;
  static bool EnableAPCKeyStats;
  static bool EnableMemcacheStats;
  static bool EnableMemcacheKeyStats;
  static bool EnableSQLStats;
  static bool EnableSQLTableStats;
  static bool EnableNetworkIOStatus;
  static std::string StatsXSL;
  static std::string StatsXSLProxy;
  static int StatsSlotDuration;
  static int StatsMaxSlot;

  static bool EnableAPCSizeStats;
  static bool EnableAPCSizeGroup;
  static std::vector<std::string> APCSizeSpecialPrefix;
  static std::vector<std::string> APCSizePrefixReplace;
  static std::vector<std::string> APCSizeSpecialMiddle;
  static std::vector<std::string> APCSizeMiddleReplace;
  static std::vector<std::string> APCSizeSkipPrefix;
  static bool EnableAPCSizeDetail;
  static bool EnableAPCFetchStats;
  static bool APCSizeCountPrime;
  static bool EnableHotProfiler;
  static int32_t ProfilerTraceBuffer;
  static double ProfilerTraceExpansion;
  static int32_t ProfilerMaxTraceBuffer;

  static int64_t MaxRSS;
  static int64_t MaxRSSPollingCycle;
  static int64_t DropCacheCycle;
  static int64_t MaxSQLRowCount;
  static int64_t MaxMemcacheKeyCount;
  static int  SocketDefaultTimeout;
  static bool LockCodeMemory;
  static bool EnableMemoryManager;
  static bool CheckMemory;
  static int MaxArrayChain;
  static bool StrictCollections;
  static bool WarnOnCollectionToArray;
  static bool UseDirectCopy;

  static bool EnableDnsCache;
  static int DnsCacheTTL;
  static time_t DnsCacheKeyMaturityThreshold;
  static size_t DnsCacheMaximumCapacity;
  static int DnsCacheKeyFrequencyUpdatePeriod;

  static std::map<std::string, std::string> ServerVariables;

  static std::map<std::string, std::string> EnvVariables;

  // The file name that is used by LightProcess to bind the socket
  // is the following prefix followed by the pid of the hphp process.
  static std::string LightProcessFilePrefix;
  static int LightProcessCount;

  // Eval options
  static bool EnableHipHopSyntax;
  static bool EnableHipHopExperimentalSyntax;
  static bool EnableShortTags;
  static bool EnableAspTags;
  static bool EnableXHP;
  static bool EnableObjDestructCall;
  static bool EnableEmitSwitch;
  static bool EnableEmitterStats;
  static bool EnableInstructionCounts;
  static bool CheckSymLink;
  static int MaxUserFunctionId;
  static bool EnableFinallyStatement;
  static bool EnableArgsInBacktraces;

  static int GetScannerType();

  static std::set<std::string, stdltistr> DynamicInvokeFunctions;

  static const uint32_t kPCREInitialTableSize = 96 * 1024;

#define EVALFLAGS()                                                     \
  /* F(type, name, defaultVal) */                                       \
  /*                                                                    \
   * Maximum number of elements on the VM execution stack.              \
   */                                                                   \
  F(uint64_t, VMStackElms, kEvalVMStackElmsDefault)                     \
  /*                                                                    \
   * Initial space reserved for the global variable environment (in     \
   * number of global variables).                                       \
   */                                                                   \
  F(uint32_t, VMInitialGlobalTableSize,                                 \
    kEvalVMInitialGlobalTableSizeDefault)                               \
  F(bool, Jit,                         evalJitDefault())                \
  F(bool, AllowHhas,                   false)                           \
  F(bool, CheckExtendedTypeHints,      false)                           \
  F(bool, JitNoGdb,                    true)                            \
  F(bool, SpinOnCrash,                 false)                           \
  F(bool, PerfPidMap,                  true)                            \
  F(bool, KeepPerfPidMap,              false)                           \
  F(uint32_t, JitTargetCacheSize,      64 << 20)                        \
  F(uint32_t, HHBCArenaChunkSize,      64 << 20)                        \
  F(bool, ProfileBC,                   false)                           \
  F(bool, ProfileHeapAcrossRequests,   false)                           \
  F(bool, ProfileHWEnable,             true)                            \
  F(string, ProfileHWEvents,           string(""))                      \
  F(bool, JitAlwaysInterpOne,          false)                           \
  F(uint32_t, JitMaxTranslations,      12)                              \
  F(uint64_t, JitGlobalTranslationLimit, -1)                            \
  F(bool, JitTrampolines,              true)                            \
  F(string, JitProfilePath,            string(""))                      \
  F(bool, JitTypePrediction,           true)                            \
  F(int32_t, JitStressTypePredPercent, 0)                               \
  F(uint32_t, JitWarmupRequests,       kDefaultWarmupRequests)          \
  F(bool, JitProfileRecord,            false)                           \
  F(uint32_t, GdbSyncChunks,           128)                             \
  F(bool, JitStressLease,              false)                           \
  F(bool, JitKeepDbgFiles,             false)                           \
  F(bool, JitEnableRenameFunction,     false)                           \
                                                                        \
  F(bool, JitDisabledByHphpd,          false)                           \
  F(bool, ThreadingJit,                false)                           \
  F(bool, JitTransCounters,            false)                           \
  F(bool, JitMGeneric,                 true)                            \
  F(double, JitCompareHHIR,            0)                               \
  F(bool, HHIRGenericDtorHelper,       true)                            \
  F(bool, HHIRCse,                     true)                            \
  F(bool, HHIRSimplification,          true)                            \
  F(uint32_t, HHIRSimplificationMaxBlocks,1000)                         \
  F(bool, HHIRGenOpts,                 true)                            \
  F(bool, HHIRJumpOpts,                true)                            \
  F(bool, HHIRExtraOptPass,            true)                            \
  F(uint32_t, HHIRNumFreeRegs,         -1)                              \
  F(bool, HHIREnableGenTimeInlining,   true)                            \
  F(bool, HHIREnableCalleeSavedOpt,    true)                            \
  F(bool, HHIREnablePreColoring,       true)                            \
  F(bool, HHIREnableCoalescing,        true)                            \
  F(bool, HHIREnableRefCountOpt,       true)                            \
  F(bool, HHIREnableSinking,           true)                            \
  F(bool, HHIRAllocXMMRegs,            true)                            \
  F(bool, HHIRGenerateAsserts,         debug)                           \
  F(bool, HHIRDirectExit,              true)                            \
  F(bool, HHIRDeadCodeElim,            true)                            \
  F(bool, HHIRPredictionOpts,          true)                            \
  F(bool, HHIRStressCodegenBlocks,     false)                           \
  F(string, JitRegionSelector,         regionSelectorDefault())         \
  F(bool,     JitPGO,                  false)                           \
  F(uint64_t, JitPGOThreshold,         2)                               \
  /* DumpBytecode =1 dumps user php, =2 dumps systemlib & user php */   \
  F(int32_t, DumpBytecode,             0)                               \
  F(bool, DumpTC,                      false)                           \
  F(bool, DumpTCAnchors,               false)                           \
  F(bool, DumpAst,                     false)                           \
  F(bool, MapTCHuge,                   true)                            \
  F(uint32_t, TCNumHugeHotMB,          16)                              \
  F(uint32_t, TCNumHugeColdMB,         4)                               \
  F(bool, RandomHotFuncs,              false)                           \
  F(bool, DisableSomeRepoAuthNotices,  true)                            \
  F(uint32_t, InitialNamedEntityTableSize,  30000)                      \
  F(uint32_t, InitialStaticStringTableSize, 100000)                     \
  F(uint32_t, PCRETableSize, kPCREInitialTableSize)                     \
  /* */                                                                 \

#define F(type, name, unused) \
  static type Eval ## name;
  EVALFLAGS()

#undef F

  static bool RecordCodeCoverage;
  static std::string CodeCoverageOutputFile;

  // TranslatorX64 allocation options
  static size_t VMTranslASize;
  static size_t VMTranslAHotSize;
  static size_t VMTranslAProfSize;
  static size_t VMTranslAStubsSize;
  static size_t VMTranslGDataSize;

  // Repo (hhvm bytecode repository) options
  static std::string RepoLocalMode;
  static std::string RepoLocalPath;
  static std::string RepoCentralPath;
  static std::string RepoEvalMode;
  static std::string RepoJournal;
  static bool RepoCommit;
  static bool RepoDebugInfo;
  static bool RepoAuthoritative;

  // Sandbox options
  static bool SandboxMode;
  static std::string SandboxPattern;
  static std::string SandboxHome;
  static std::string SandboxFallback;
  static std::string SandboxConfFile;
  static std::map<std::string, std::string> SandboxServerVariables;
  static bool SandboxFromCommonRoot;
  static std::string SandboxDirectoriesRoot;
  static std::string SandboxLogsRoot;

  // Debugger options
  static bool EnableDebugger;
  static bool EnableDebuggerColor;
  static bool EnableDebuggerPrompt;
  static bool EnableDebuggerServer;
  static bool EnableDebuggerUsageLog;
  static int DebuggerServerPort;
  static int DebuggerDefaultRpcPort;
  static std::string DebuggerDefaultRpcAuth;
  static std::string DebuggerRpcHostDomain;
  static int DebuggerDefaultRpcTimeout;
  static std::string DebuggerDefaultSandboxPath;
  static std::string DebuggerStartupDocument;
  static int DebuggerSignalTimeout;

  // Mail options
  static std::string SendmailPath;
  static std::string MailForceExtraParameters;

  // preg stack depth and debug support options
  static long PregBacktraceLimit;
  static long PregRecursionLimit;
  static bool EnablePregErrorLog;

#ifdef FACEBOOK
  // fb303 server
  static bool EnableFb303Server;
  static int Fb303ServerPort;
  static int Fb303ServerThreadStackSizeMb;
  static int Fb303ServerWorkerThreads;
  static int Fb303ServerPoolThreads;
#endif

  // Convenience switch to turn on/off code alternatives via command-line
  // Do not commit code guarded by this flag, for evaluation only.
  static int EnableAlternative;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RUNTIME_OPTION_H_
