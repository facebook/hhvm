/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_RUNTIME_OPTION_H_
#define incl_HPHP_RUNTIME_OPTION_H_

#include <unordered_map>
#include <vector>
#include <string>
#include <map>
#include <set>

#include <boost/container/flat_set.hpp>
#include <memory>

#include "hphp/util/hash-map-typedefs.h"
#include "hphp/util/functional.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class AccessLogFileData;
struct VirtualHost;
struct IpBlockMap;
struct SatelliteServerInfo;
struct FilesMatch;
struct Hdf;

constexpr int kDefaultInitialStaticStringTableSize = 500000;

/**
 * Configurable options set from command line or configurable file at startup
 * time.
 */
class RuntimeOption {
public:
  static void Load(Hdf& config,
                   std::vector<std::string>* overwrites = nullptr,
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
  static std::string InstanceId;
  static std::string PidFile;

  static std::string LogFile;
  static std::string LogFileSymLink;
  static int LogHeaderMangle;
  static bool AlwaysEscapeLog;
  static bool AlwaysLogUnhandledExceptions;
  static bool InjectedStackTrace;
  static int InjectedStackTraceLimit; // limit the size of the backtrace
  static bool NoSilencer;
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
  static int64_t NoticeFrequency; // output 1 out of NoticeFrequency notices
  static int64_t WarningFrequency;
  static int RaiseDebuggingFrequency;
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
  static std::string ServerFileSocket;
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
  static bool ServerExitOnBindFail;
  static int PageletServerThreadCount;
  static bool PageletServerThreadRoundRobin;
  static int PageletServerThreadDropCacheTimeoutSeconds;
  static int PageletServerQueueLimit;
  static bool PageletServerThreadDropStack;

  static int FiberCount;
  static int RequestTimeoutSeconds;
  static int PspTimeoutSeconds;
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
  static std::vector<std::string> ServerNextProtocols;
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
  static bool PathDebug;
  static std::vector<std::shared_ptr<VirtualHost>> VirtualHosts;
  static std::shared_ptr<IpBlockMap> IpBlocks;
  static std::vector<std::shared_ptr<SatelliteServerInfo>>
         SatelliteServerInfos;

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
  static std::string TLSClientCipherSpec;

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
  static bool EnableStaticContentFromDisk;
  static bool EnableOnDemandUncompress;
  static bool EnableStaticContentMMap;

  static bool Utf8izeReplace;

  static std::string StartupDocument;
  static std::string WarmupDocument;
  static std::string RequestInitFunction;
  static std::string RequestInitDocument;
  static std::string AutoPrependFile;
  static std::string AutoAppendFile;
  static std::vector<std::string> ThreadDocuments;
  static std::vector<std::string> ThreadLoopDocuments;

  static bool SafeFileAccess;
  static std::vector<std::string> AllowedDirectories;
  static std::set<std::string> AllowedFiles;
  static hphp_string_imap<std::string> StaticFileExtensions;
  static hphp_string_imap<std::string> PhpFileExtensions;
  static std::set<std::string> ForbiddenFileExtensions;
  static std::set<std::string> StaticFileGenerators;
  static std::vector<std::shared_ptr<FilesMatch>> FilesMatches;

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
  static bool EnableMemcacheStats;
  static bool EnableMemcacheKeyStats;
  static bool EnableSQLStats;
  static bool EnableSQLTableStats;
  static bool EnableNetworkIOStatus;
  static std::string StatsXSL;
  static std::string StatsXSLProxy;
  static int StatsSlotDuration;
  static int StatsMaxSlot;

  static bool EnableHotProfiler;
  static int32_t ProfilerTraceBuffer;
  static double ProfilerTraceExpansion;
  static int32_t ProfilerMaxTraceBuffer;

  static int64_t MaxRSS;
  static int64_t MaxRSSPollingCycle;
  static int64_t DropCacheCycle;
  static int64_t MaxSQLRowCount;
  static int64_t MaxMemcacheKeyCount;
  static int64_t SocketDefaultTimeout;
  static bool LockCodeMemory;
  static int MaxArrayChain;
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
  static bool EnableArgsInBacktraces;
  static bool EnableZendCompat;
  static bool TimeoutsUseWallTime;
  static bool CheckFlushOnUserClose;
  static bool EvalAuthoritativeMode;
  static bool IntsOverflowToInts;

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
  F(bool, SimulateARM,                 simulateARMDefault())            \
  F(bool, JitRequireWriteLease,        false)                           \
  F(uint64_t, JitAHotSize,             4 << 20)                         \
  F(uint64_t, JitASize,                60 << 20)                        \
  F(uint64_t, JitAMaxUsage,            maxUsageDef())                   \
  F(uint64_t, JitAProfSize,            64 << 20)                        \
  F(uint64_t, JitAStubsSize,           64 << 20)                        \
  F(uint64_t, JitGlobalDataSize,       kJitGlobalDataDef)               \
  F(bool, AllowHhas,                   false)                           \
  /* CheckReturnTypeHints:
     0 - no checks or enforcement
     1 - raises E_WARNING if regular type hint or soft type hint fails
     2 - raises E_RECOVERABLE_ERROR if regular type hint fails, raises
         E_WARNING if soft type hint fails; note that in repo mode the
         error handler is not allowed to resume on recoverable errors */ \
  F(int32_t, CheckReturnTypeHints,     0)                               \
  F(bool, JitNoGdb,                    true)                            \
  F(bool, SpinOnCrash,                 false)                           \
  F(uint32_t, DumpRingBufferOnCrash,   0)                               \
  F(bool, PerfPidMap,                  true)                            \
  F(bool, KeepPerfPidMap,              false)                           \
  F(bool, RuntimeTypeProfile,          false)                           \
  F(int32_t, RuntimeTypeProfileLoggingFreq,  0)                         \
  F(uint32_t, JitTargetCacheSize,      64 << 20)                        \
  F(uint32_t, HHBCArenaChunkSize,      64 << 20)                        \
  F(bool, ProfileBC,                   false)                           \
  F(bool, ProfileHeapAcrossRequests,   false)                           \
  F(bool, ProfileHWEnable,             true)                            \
  F(string, ProfileHWEvents,      std::string(""))                      \
  F(bool, JitAlwaysInterpOne,          false)                           \
  F(uint32_t, JitMaxTranslations,      12)                              \
  F(uint64_t, JitGlobalTranslationLimit, -1)                            \
  F(bool, JitTrampolines,              true)                            \
  F(string, JitProfilePath,       std::string(""))                      \
  F(bool, JitTypePrediction,           true)                            \
  F(int32_t, JitStressTypePredPercent, 0)                               \
  F(uint32_t, JitWarmupRequests,       kDefaultWarmupRequests)          \
  F(uint32_t, JitProfileRequests,      kDefaultProfileRequests)         \
  F(bool, JitProfileRecord,            false)                           \
  F(uint32_t, GdbSyncChunks,           128)                             \
  F(bool, JitStressLease,              false)                           \
  F(bool, JitKeepDbgFiles,             false)                           \
  /* despite the unfortunate name, this enables function renaming and
   * interception in the interpreter as well as the jit */              \
  F(bool, JitEnableRenameFunction,     false)                           \
  F(bool, JitUseVtuneAPI,              false)                           \
                                                                        \
  F(bool, JitDisabledByHphpd,          false)                           \
  F(bool, JitTransCounters,            false)                           \
  F(bool, HHIRBytecodeControlFlow,     hhirBytecodeControlFlowDefault())\
  F(bool, HHIRCse,                     true)                            \
  F(bool, HHIRSimplification,          true)                            \
  F(bool, HHIRGenOpts,                 true)                            \
  F(bool, HHIRJumpOpts,                true)                            \
  F(bool, HHIRRefcountOpts,            hhirRefcountOptsDefault())       \
  F(bool, HHIRRefcountOptsAlwaysSink,  false)                           \
  F(bool, HHIRExtraOptPass,            true)                            \
  F(uint32_t, HHIRNumFreeRegs,         64)                              \
  F(bool, HHIREnableGenTimeInlining,   true)                            \
  F(uint32_t, HHIRInliningMaxCost,     13)                              \
  F(uint32_t, HHIRAlwaysInlineMaxCost, 10)                              \
  F(uint32_t, HHIRInliningMaxDepth,    4)                               \
  F(uint32_t, HHIRInliningMaxReturnDecRefs, 3)                          \
  F(bool, HHIRInlineFrameOpts,         false)                           \
  /* 1 (the default) gives most asserts. 2 adds less commonly           \
   * useful/more expensive asserts. */                                  \
  F(uint32_t, HHIRGenerateAsserts,     debug)                           \
  F(bool, HHIRDirectExit,              true)                            \
  F(bool, HHIRDeadCodeElim,            true)                            \
  F(bool, HHIRPredictionOpts,          true)                            \
  F(bool, HHIRStressCodegenBlocks,     false)                           \
  /* Register allocation flags */                                       \
  F(bool, HHIREnableCalleeSavedOpt,    true)                            \
  F(bool, HHIREnablePreColoring,       true)                            \
  F(bool, HHIREnableCoalescing,        true)                            \
  F(bool, HHIRAllocSIMDRegs,           true)                            \
  /* Region compiler flags */                                           \
  F(string,   JitRegionSelector,       regionSelectorDefault())         \
  F(bool,     JitCompareRegions,       false)                           \
  F(bool,     JitPGO,                  pgoDefault())                    \
  F(string,   JitPGORegionSelector,    "hottrace")                      \
  F(uint64_t, JitPGOThreshold,         kDefaultJitPGOThreshold)         \
  F(bool,     JitPGOHotOnly,           ServerExecutionMode())           \
  F(bool,     JitPGOUsePostConditions, true)                            \
  F(uint32_t, HotFuncThreshold,        10)                              \
  F(bool, HHIRValidateRefCount,        debug)                           \
  F(bool, HHIRRelaxGuards,             hhirRelaxGuardsDefault())        \
  F(bool, HHBCRelaxGuards,             hhbcRelaxGuardsDefault())        \
  /* DumpBytecode =1 dumps user php, =2 dumps systemlib & user php */   \
  F(int32_t, DumpBytecode,             0)                               \
  F(bool, DumpHhas,                    false)                           \
  F(bool, DumpTC,                      false)                           \
  F(bool, DumpTCAnchors,               false)                           \
  F(bool, DumpAst,                     false)                           \
  F(bool, MapTCHuge,                   hugePagesSoundNice())            \
  F(bool, MapHotTextHuge,              hugePagesSoundNice())            \
  F(bool, MapTgtCacheHuge,             false)                           \
  F(int32_t, MaxLowMemHugePages,       hugePagesSoundNice() ? 8 : 0)    \
  F(uint32_t, TCNumHugeHotMB,          16)                              \
  F(uint32_t, TCNumHugeColdMB,         4)                               \
  F(bool, RandomHotFuncs,              false)                           \
  F(bool, DisableSomeRepoAuthNotices,  true)                            \
  F(bool, CheckRepoAuthDeserialize,    false)                           \
  F(uint32_t, InitialNamedEntityTableSize,  30000)                      \
  F(uint32_t, InitialStaticStringTableSize,                             \
                        kDefaultInitialStaticStringTableSize)           \
  F(uint32_t, PCRETableSize, kPCREInitialTableSize)                     \
  F(bool, EnableNuma, ServerExecutionMode())                            \
  F(bool, EnableNumaLocal, ServerExecutionMode())                       \
  F(bool, DecRefUsePlainDecl,          true)                            \
  F(bool, DecRefUsePlainDeclWithDestroy,true)                           \
  F(bool, DecRefUseScratch,            false)                           \
  /* */

private:
  using string = std::string;
public:
#define F(type, name, unused) \
  static type Eval ## name;
  EVALFLAGS()

#undef F

  static bool RecordCodeCoverage;
  static std::string CodeCoverageOutputFile;

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
  static bool DebuggerDisableIPv6;
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

  // pprof/hhprof server options
  static bool HHProfServerEnabled;
  static int HHProfServerPort;
  static int HHProfServerThreads;
  static int HHProfServerTimeoutSeconds;
  static bool HHProfServerProfileClientMode;
  static bool HHProfServerAllocationProfile;
  static int HHProfServerFilterMinAllocPerReq;
  static int HHProfServerFilterMinBytesPerReq;

  // SimpleXML options
  static bool SimpleXMLEmptyNamespaceMatchesAll;

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
static_assert(sizeof(RuntimeOption) == 1, "no instance variables");

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RUNTIME_OPTION_H_
