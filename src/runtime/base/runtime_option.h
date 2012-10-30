/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __RUNTIME_OPTION_H__
#define __RUNTIME_OPTION_H__

#include <runtime/base/server/virtual_host.h>
#include <runtime/base/server/satellite_server.h>
#include <runtime/base/server/files_match.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class AccessLogFileData;
/**
 * Configurable options set from command line or configurable file at startup
 * time.
 */
class RuntimeOption {
public:
  static void Load(Hdf &config, StringVec *overwrites = NULL,
                   bool empty = false);

  static bool Loaded;

  static bool serverExecutionMode() {
    return strcmp(ExecutionMode, "srv") == 0;
  }

  static bool clientExecutionMode() {
    return strcmp(ExecutionMode, "cli") == 0;
  }

  static const char *ExecutionMode;
  static std::string BuildId;
  static std::string PidFile;

  static std::string LogFile;
  static std::string LogFileSymLink;
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
  static int64 SerializationSizeLimit;
  static int64 StringOffsetLimit;

  static std::string AccessLogDefaultFormat;
  static std::vector<AccessLogFileData> AccessLogs;

  static std::string AdminLogFormat;
  static std::string AdminLogFile;
  static std::string AdminLogSymLink;

  static std::string Tier;
  static std::string Host;
  static std::string DefaultServerNameSuffix;
  static std::string ServerIP;
  static std::string ServerPrimaryIP;
  static int ServerPort;
  static int ServerPortFd;
  static int ServerBacklog;
  static int ServerConnectionLimit;
  static int ServerThreadCount;
  static bool ServerThreadRoundRobin;
  static int ServerThreadDropCacheTimeoutSeconds;
  static bool ServerThreadJobLIFO;
  static bool ServerThreadDropStack;
  static bool ServerHttpSafeMode;
  static bool ServerStatCache;
  static int PageletServerThreadCount;
  static bool PageletServerThreadRoundRobin;
  static int PageletServerThreadDropCacheTimeoutSeconds;
  static int PageletServerQueueLimit;
  static bool PageletServerThreadDropStack;

  static int FiberCount;
  static int RequestTimeoutSeconds;
  static size_t ServerMemoryHeadRoom;
  static int64 RequestMemoryMaxBytes;
  static int64 ImageMemoryMaxBytes;
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
  static int64 MaxPostSize;
  static bool AlwaysPopulateRawPostData;
  static int64 UploadMaxFileSize;
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

  static std::string RTTIDirectory;
  static bool EnableCliRTTI;
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
  static bool MySQLLocalize;  // whether to localize MySQL query results
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
  static int32 ProfilerTraceBuffer;
  static double ProfilerTraceExpansion;
  static int32 ProfilerMaxTraceBuffer;

  static int64 MaxRSS;
  static int64 MaxRSSPollingCycle;
  static int64 DropCacheCycle;
  static int64 MaxSQLRowCount;
  static int64 MaxMemcacheKeyCount;
  static int  SocketDefaultTimeout;
  static bool LockCodeMemory;
  static bool EnableMemoryManager;
  static bool CheckMemory;
  static int MaxArrayChain;
  static bool UseHphpArray;
  static bool UseSmallArray;
  static bool UseVectorArray;
  static bool StrictCollections;
  static bool WarnOnCollectionToArray;
  static bool UseDirectCopy;
  static bool EnableApc;
  static bool EnableConstLoad;
  static bool ForceConstLoadToAPC;
  static std::string ApcPrimeLibrary;
  static int ApcLoadThread;
  static std::set<std::string> ApcCompletionKeys;
  enum ApcTableTypes {
    ApcConcurrentTable
  };
  static ApcTableTypes ApcTableType;
  static bool EnableApcSerialize;
  static time_t ApcKeyMaturityThreshold;
  static size_t ApcMaximumCapacity;
  static int ApcKeyFrequencyUpdatePeriod;
  static bool ApcExpireOnSets;
  static int ApcPurgeFrequency;
  static int ApcPurgeRate;
  static bool ApcAllowObj;
  static int ApcTTLLimit;
  static bool ApcUseFileStorage;
  static int64 ApcFileStorageChunkSize;
  static int64 ApcFileStorageMaxSize;
  static std::string ApcFileStoragePrefix;
  static int ApcFileStorageAdviseOutPeriod;
  static std::string ApcFileStorageFlagKey;
  static bool ApcConcurrentTableLockFree;
  static bool ApcFileStorageKeepFileLinked;
  static std::vector<std::string> ApcNoTTLPrefix;

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
  static bool EnableEvalOptimization;
  static int  EvalScalarValueExprLimit;
  static bool CheckSymLink;
  static bool NativeXHP;
  static int ScannerType;
  static int MaxUserFunctionId;
  static bool EnableFinallyStatement;

#ifdef TAINTED
  static bool EnableTaintWarnings;
  static int TaintTraceMaxStrlen;
#endif

  static std::set<std::string, stdltistr> DynamicInvokeFunctions;
  static bool EnableStrict;
  static int StrictLevel;
  static bool StrictFatal;

  /*
   * Maximum number of elements on the VM execution stack.
   */
  static uint64 EvalVMStackElms;

  /*
   * Initial space reserved for the global variable environment (in
   * number of global variables).
   */
  static uint32_t EvalVMInitialGlobalTableSize;

  static bool EvalJit;
  static bool EvalAllowHhas;
  static bool EvalJitNoGdb;
  static uint32 EvalJitTargetCacheSize;
  static bool EvalProfileBC;
  static bool EvalProfileHWEnable;
  static std::string EvalProfileHWEvents;
  static bool EvalJitTrampolines;
  static string EvalJitProfilePath;
  static int EvalJitStressTypePredPercent;
  static uint32 EvalJitWarmupRequests;
  static bool EvalJitProfileRecord;
  static uint32 EvalGdbSyncChunks;
  static bool EvalJitStressLease;
  static bool EvalJitKeepDbgFiles;
  static bool EvalJitEnableRenameFunction;

  static bool EvalJitDisabledByHphpd;
  static bool EvalJitCmovVarDeref;
  static bool EvalThreadingJit;
  static bool EvalJitTransCounters;
  static bool EvalJitMGeneric;
  static bool EvalJitUseIR;
  static bool EvalIRPuntDontInterp;
  static bool EvalHHIRMemOpt;
  static uint32 EvalHHIRNumFreeRegs;
  static bool EvalHHIREnableRematerialization;
  static bool EvalHHIREnableCalleeSavedOpt;
  static bool EvalHHIREnablePreColoring;
  static bool EvalHHIREnableCoalescing;
  static bool EvalHHIREnableMmx;
  static bool EvalHHIREnableRefCountOpt;
  static bool EvalHHIREnableSinking;
  static bool EvalHHIRGenerateAsserts;
  static uint64 EvalHHIRDirectExit;
  static uint64 EvalMaxHHIRTrans;
  static bool EvalDumpBytecode;
  static uint32 EvalDumpIR;
  static bool EvalDumpTC;
  static bool EvalDumpAst;
  static bool EvalMapTCHuge;
  static uint32 EvalConstEstimate;
  static bool RecordCodeCoverage;
  static std::string CodeCoverageOutputFile;

  // Repo (hhvm bytecode repository) options
  static std::string RepoLocalMode;
  static std::string RepoLocalPath;
  static std::string RepoCentralPath;
  static std::string RepoEvalMode;
  static bool RepoCommit;
  static bool RepoDebugInfo;
  static bool RepoAuthoritative;

  // Sandbox options
  static bool SandboxMode;
  static bool SandboxCheckMd5;
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
  static bool EnableDebuggerServer;
  static int DebuggerServerPort;
  static int DebuggerDefaultRpcPort;
  static std::string DebuggerDefaultRpcAuth;
  static std::string DebuggerRpcHostDomain;
  static int DebuggerDefaultRpcTimeout;
  static std::string DebuggerDefaultSandboxPath;
  static std::string DebuggerStartupDocument;
  static std::string DebuggerUsageLogFile;

  // Mail options
  static std::string SendmailPath;
  static std::string MailForceExtraParameters;

  // preg stack depth and debug support options
  static int PregBacktraceLimit;
  static int PregRecursionLimit;
  static bool EnablePregErrorLog;

  // Convenience switch to turn on/off code alternatives via command-line
  // Do not commit code guarded by this flag, for evaluation only.
  static int EnableAlternative;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __RUNTIME_OPTION_H__
