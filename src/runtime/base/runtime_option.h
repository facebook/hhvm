/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
  static void Load(Hdf &config, StringVec *overwrites = NULL);

  static bool Loaded;

  static const char *ExecutionMode;
  static std::string BuildId;
  static std::string PidFile;

  static std::string LogFile;
  static std::string LogFileSymLink;
  static std::string LogAggregatorFile;
  static std::string LogAggregatorDatabase;
  static int LogAggregatorSleepSeconds;
  static bool AlwaysEscapeLog;
  static bool AlwaysLogUnhandledExceptions;
  static bool InjectedStackTrace;
  static int InjectedStackTraceLimit; // limit the size of the backtrace
  static bool NoSilencer;
  static bool EnableApplicationLog;
  static bool CallUserHandlerOnFatals;
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
  static int ServerBacklog;
  static int ServerConnectionLimit;
  static int ServerThreadCount;
  static bool ServerThreadRoundRobin;
  static int ServerThreadDropCacheTimeoutSeconds;
  static bool ServerThreadJobLIFO;
  static int PageletServerThreadCount;
  static bool PageletServerThreadRoundRobin;
  static int PageletServerThreadDropCacheTimeoutSeconds;
  static int FiberCount;
  static int RequestTimeoutSeconds;
  static size_t ServerMemoryMaxActive;
  static int64 RequestMemoryMaxBytes;
  static int64 ImageMemoryMaxBytes;
  static int ResponseQueueCount;
  static int ServerGracefulShutdownWait;
  static int ServerDanglingWait;
  static bool ServerHarshShutdown;
  static bool ServerEvilShutdown;
  static int GzipCompressionLevel;
  static std::string ForceCompressionURL;
  static std::string ForceCompressionCookie;
  static std::string ForceCompressionParam;
  static bool EnableMagicQuotesGpc;
  static bool EnableKeepAlive;
  static bool ExposeHPHP;
  static bool ExposeXFBServer;
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
  static VirtualHostPtrVec VirtualHosts;
  static IpBlockMapPtr IpBlocks;
  static SatelliteServerInfoPtrVec SatelliteServerInfos;

  // If a request has a body over this limit, switch to on-demand reading.
  // -1 for no limit.
  static int RequestBodyReadLimit;

  static bool EnableSSL;
  static int SSLPort;
  static std::string SSLCertificateFile;
  static std::string SSLCertificateKeyFile;

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
  static std::string XboxProcessMessageFunc;
  static std::string XboxPassword;

  static std::string SourceRoot;
  static std::vector<std::string> IncludeSearchPaths;
  static std::string FileCache;
  static std::string DefaultDocument;
  static std::string ErrorDocument404;
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

  static std::string TakeoverFilename;
  static int AdminServerPort;
  static int AdminThreadCount;
  static std::string AdminPassword;

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
  static bool EnableMemoryManager;
  static bool CheckMemory;
  static bool UseHphpArray;
  static bool UseSmallArray;
  static bool UseDirectCopy;
  static bool EnableApc;
  static bool EnableConstLoad;
  static bool ForceConstLoadToAPC;
  static std::string ApcPrimeLibrary;
  static int ApcLoadThread;
  static std::set<std::string> ApcCompletionKeys;
  enum ApcTableTypes {
    ApcHashTable,
    ApcLfuTable,
    ApcConcurrentTable
  };
  static ApcTableTypes ApcTableType;
  enum ApcTableLockTypes {
    ApcMutex,
    ApcReadWriteLock
  };
  static ApcTableLockTypes ApcTableLockType;
  static time_t ApcKeyMaturityThreshold;
  static size_t ApcMaximumCapacity;
  static int ApcKeyFrequencyUpdatePeriod;
  static bool ApcExpireOnSets;
  static int ApcPurgeFrequency;
  static bool ApcAllowObj;

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
  static bool CheckSymLink;
  static bool NativeXHP;
  static int ScannerType;

  static bool EnableStrict;
  static int StrictLevel;
  static bool StrictFatal;
  static bool RecordCodeCoverage;
  static std::string CodeCoverageOutputFile;

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
  static bool EnableDebuggerServer;
  static int DebuggerServerPort;
  static int DebuggerDefaultRpcPort;
  static std::string DebuggerDefaultRpcAuth;
  static std::string DebuggerRpcHostDomain;
  static int DebuggerDefaultRpcTimeout;
  static std::string DebuggerDefaultSandboxPath;
  static std::string DebuggerStartupDocument;

  // Mail options
  static std::string SendmailPath;
  static std::string MailForceExtraParameters;

  // preg stack depth options
  static int PregBacktraceLimit;
  static int PregRecursionLimit;

  static bool MethodSlotCalls;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __RUNTIME_OPTION_H__
