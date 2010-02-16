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

#include <cpp/base/server/virtual_host.h>
#include <cpp/base/server/satellite_server.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Configurable options set from command line or configurable file at startup
 * time.
 */
class RuntimeOption {
public:
  static void Load(Hdf &config);

  static const char *ExecutionMode;
  static std::string BuildId;
  static std::string PidFile;

  static std::string LogFile;
  static std::string LogAggregatorFile;
  static std::string LogAggregatorDatabase;
  static int LogAggregatorSleepSeconds;
  static bool AlwaysLogUnhandledExceptions;
  static bool NoSilencer;
  static bool EnableApplicationLog;

  static bool NoInfiniteLoopDetection;
  static bool NoInfiniteRecursionDetection;
  static bool ThrowBadTypeExceptions;
  static bool ThrowNotices;
  static bool AssertActive;
  static bool AssertWarning;

  static std::string AccessLogDefaultFormat;
  static std::vector<std::pair<std::string, std::string> > AccessLogs;

  static std::string AdminLogFormat;
  static std::string AdminLogFile;

  static std::string Tier;
  static std::string Host;
  static std::string DefaultServerNameSuffix;
  static std::string ServerIP;
  static std::string ServerPrimaryIP;
  static int ServerPort;
  static int ServerThreadCount;
  static int PageletServerThreadCount;
  static int RequestTimeoutSeconds;
  static int RequestMemoryMaxBytes;
  static int ResponseQueueCount;
  static int ServerGracefulShutdownWait;
  static int ServerDanglingWait;
  static bool ServerHarshShutdown;
  static bool ServerEvilShutdown;
  static int GzipCompressionLevel;
  static bool EnableKeepAlive;
  static bool EnableEarlyFlush;
  static bool ForceChunkedEncoding;
  static int MaxPostSize;
  static int UploadMaxFileSize;
  static bool EnableFileUploads;
  static bool LibEventSyncSend;
  static bool ExpiresActive;
  static int ExpiresDefault;
  static std::string DefaultCharsetName;
  static bool ForceServerNameToHeader;
  static VirtualHostPtrVec VirtualHosts;
  static IpBlockMapPtr IpBlocks;
  static SatelliteServerInfoPtrVec SatelliteServerInfos;

  static int XboxServerThreadCount;
  static int XboxServerPort;
  static int XboxDefaultLocalTimeoutMilliSeconds;
  static int XboxDefaultRemoteTimeoutSeconds;
  static int XboxServerInfoMaxRequest;
  static int XboxServerInfoDuration;
  static std::string XboxServerInfoWarmupDoc;
  static std::string XboxServerInfoReqInitFunc;
  static std::string XboxProcessMessageFunc;
  static std::string XboxPassword;

  static std::string SourceRoot;
  static std::vector<std::string> IncludeSearchPaths;
  static std::string FileCache;
  static std::string DefaultDocument;
  static std::string ErrorDocument404;
  static std::string FatalErrorMessage;
  static std::string FontPath;
  static bool EnableStaticContentCache;
  static bool EnableStaticContentFromDisk;

  static std::string RTTIDirectory;
  static bool EnableCliRTTI;

  static std::string StartupDocument;
  static std::string WarmupDocument;
  static std::string RequestInitFunction;
  static std::vector<std::string> ThreadDocuments;

  static bool SafeFileAccess;
  static std::vector<std::string> AllowedDirectories;
  static std::set<std::string> AllowedFiles;
  static std::map<std::string, std::string> StaticFileExtensions;
  static std::set<std::string> StaticFileGenerators;

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

  static bool MySQLReadOnly;
  static bool MySQLLocalize;  // whether to localize MySQL query results
  static int  MySQLConnectTimeout;
  static int  MySQLReadTimeout;
  static int  MySQLSlowQueryThreshold;
  static bool MySQLKillOnTimeout;

  static int  HttpDefaultTimeout;
  static int  HttpSlowQueryThreshold;

  static int  SocketDefaultTimeout;
  static bool LocalMemcache;
  static bool MemcacheReadOnly;

  static bool FullBacktrace;
  static bool ServerStackTrace;
  static bool ServerErrorMessage;
  static bool TranslateSource;
  static bool RecordInput;
  static bool ClearInputOnSuccess;
  static std::string ProfilerOutputDir;
  static std::string CoreDumpEmail;
  static bool CoreDumpReport;

  static bool EnableStats;
  static bool EnableWebStats;
  static bool EnableMemoryStats;
  static bool EnableMallocStats;
  static bool EnableAPCStats;
  static bool EnableAPCKeyStats;
  static bool EnableMemcacheStats;
  static bool EnableSQLStats;
  static std::string StatsXSL;
  static std::string StatsXSLProxy;
  static int StatsSlotDuration;
  static int StatsMaxSlot;

  static int64 MaxRSS;
  static bool EnableMemoryManager;
  static bool CheckMemory;
  static bool UseZendArray;
  static bool EnableApc;
  static bool ApcUseSharedMemory;
  static int ApcSharedMemorySize;
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
  static bool ApcUseLockedRefs;
  static bool ApcExpireOnSets;
  static int ApcPurgeFrequency;

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

  // Use FrameInjection-based backtrace, which does not contain C++ frames.
  static bool InjectedStacktrace;

  // Eval options
  static bool EnableXHP;
  static bool EnableStrict;
  static int StrictLevel;
  static bool StrictFatal;
  static bool EvalBytecodeInterpreter;
  static bool DumpBytecode;

  // Sandbox options
  static bool SandboxMode;
  static std::string SandboxPattern;
  static std::string SandboxHome;
  static std::string SandboxConfFile;
  static std::map<std::string, std::string> SandboxServerVariables;

  // Mail options
  static std::string SendmailPath;
  static std::string MailForceExtraParameters;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __RUNTIME_OPTION_H__
