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

#include <runtime/base/runtime_option.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/shared/shared_store.h>
#include <util/util.h>
#include <util/network.h>
#include <util/logger.h>
#include <util/stack_trace.h>
#include <util/process.h>
#include <util/file_cache.h>
#include <runtime/base/preg.h>
#include <runtime/base/server/access_log.h>
#include <runtime/base/util/extended_logger.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const char *RuntimeOption::ExecutionMode = "";
std::string RuntimeOption::BuildId;
std::string RuntimeOption::PidFile = "www.pid";

std::string RuntimeOption::LogFile;
std::string RuntimeOption::LogAggregatorFile;
std::string RuntimeOption::LogAggregatorDatabase;
int RuntimeOption::LogAggregatorSleepSeconds = 10;
bool RuntimeOption::AlwaysLogUnhandledExceptions = true;
bool RuntimeOption::InjectedStackTrace = true;
bool RuntimeOption::NoSilencer = false;
bool RuntimeOption::EnableApplicationLog = true;
bool RuntimeOption::CallUserHandlerOnFatals = true;
int RuntimeOption::RuntimeErrorReportingLevel = ErrorConstants::ALL;

bool RuntimeOption::NoInfiniteLoopDetection = false;
bool RuntimeOption::NoInfiniteRecursionDetection = false;
bool RuntimeOption::ThrowBadTypeExceptions = false;
bool RuntimeOption::ThrowTooManyArguments = false;
bool RuntimeOption::WarnTooManyArguments = false;
bool RuntimeOption::ThrowMissingArguments = false;
bool RuntimeOption::ThrowInvalidArguments = false;
bool RuntimeOption::AssertActive = false;
bool RuntimeOption::AssertWarning = false;
int RuntimeOption::NoticeFrequency = 1;
int RuntimeOption::WarningFrequency = 1;
int64 RuntimeOption::SerializationSizeLimit = 0;

std::string RuntimeOption::AccessLogDefaultFormat;
std::vector<std::pair<std::string, std::string> >  RuntimeOption::AccessLogs;

std::string RuntimeOption::AdminLogFormat;
std::string RuntimeOption::AdminLogFile;


std::string RuntimeOption::Tier;
std::string RuntimeOption::Host;
std::string RuntimeOption::DefaultServerNameSuffix;
std::string RuntimeOption::ServerIP;
std::string RuntimeOption::ServerPrimaryIP;
int RuntimeOption::ServerPort;
int RuntimeOption::ServerThreadCount = 50;
int RuntimeOption::PageletServerThreadCount = 0;
int RuntimeOption::FiberCount = 0;
int RuntimeOption::RequestTimeoutSeconds = 0;
int RuntimeOption::RequestMemoryMaxBytes = 0;
int RuntimeOption::ImageMemoryMaxBytes = 0;
int RuntimeOption::ResponseQueueCount;
int RuntimeOption::ServerGracefulShutdownWait;
bool RuntimeOption::ServerHarshShutdown = true;
bool RuntimeOption::ServerEvilShutdown = true;
int RuntimeOption::ServerDanglingWait;
int RuntimeOption::GzipCompressionLevel = 3;
bool RuntimeOption::EnableMagicQuotesGpc = false;
bool RuntimeOption::EnableKeepAlive = true;
int RuntimeOption::ConnectionTimeoutSeconds = -1;
bool RuntimeOption::EnableEarlyFlush = true;
bool RuntimeOption::ForceChunkedEncoding = false;
int RuntimeOption::MaxPostSize;
int RuntimeOption::UploadMaxFileSize;
std::string RuntimeOption::UploadTmpDir;
bool RuntimeOption::EnableFileUploads;
bool RuntimeOption::EnableUploadProgress;
int RuntimeOption::Rfc1867Freq;
std::string RuntimeOption::Rfc1867Prefix;
std::string RuntimeOption::Rfc1867Name;
bool RuntimeOption::LibEventSyncSend = true;
bool RuntimeOption::ExpiresActive = true;
int RuntimeOption::ExpiresDefault = 2592000;
std::string RuntimeOption::DefaultCharsetName = "UTF-8";
bool RuntimeOption::ForceServerNameToHeader = false;

bool RuntimeOption::EnableSSL = false;
int RuntimeOption::SSLPort = 443;
std::string RuntimeOption::SSLCertificateFile;
std::string RuntimeOption::SSLCertificateKeyFile;

VirtualHostPtrVec RuntimeOption::VirtualHosts;
IpBlockMapPtr RuntimeOption::IpBlocks;
SatelliteServerInfoPtrVec RuntimeOption::SatelliteServerInfos;

int RuntimeOption::XboxServerThreadCount = 0;
int RuntimeOption::XboxServerPort = 0;
int RuntimeOption::XboxDefaultLocalTimeoutMilliSeconds = 500;
int RuntimeOption::XboxDefaultRemoteTimeoutSeconds = 5;
int RuntimeOption::XboxServerInfoMaxRequest = 500;
int RuntimeOption::XboxServerInfoDuration = 120;
std::string RuntimeOption::XboxServerInfoWarmupDoc;
std::string RuntimeOption::XboxServerInfoReqInitFunc;
std::string RuntimeOption::XboxProcessMessageFunc = "xbox_process_message";
std::string RuntimeOption::XboxPassword;

std::string RuntimeOption::SourceRoot;
std::vector<std::string> RuntimeOption::IncludeSearchPaths;
std::string RuntimeOption::FileCache;
std::string RuntimeOption::DefaultDocument;
std::string RuntimeOption::ErrorDocument404;
std::string RuntimeOption::ErrorDocument500;
std::string RuntimeOption::FatalErrorMessage;
std::string RuntimeOption::FontPath;
bool RuntimeOption::EnableStaticContentCache = true;
bool RuntimeOption::EnableStaticContentFromDisk = true;

std::string RuntimeOption::RTTIDirectory;
bool RuntimeOption::EnableCliRTTI = false;

std::string RuntimeOption::StartupDocument;
std::string RuntimeOption::WarmupDocument;
std::string RuntimeOption::RequestInitFunction;
std::vector<std::string> RuntimeOption::ThreadDocuments;

bool RuntimeOption::SafeFileAccess = false;
std::vector<std::string> RuntimeOption::AllowedDirectories;
std::set<std::string> RuntimeOption::AllowedFiles;
std::map<std::string, std::string> RuntimeOption::StaticFileExtensions;
std::set<std::string> RuntimeOption::ForbiddenFileExtensions;
std::set<std::string> RuntimeOption::StaticFileGenerators;
FilesMatchPtrVec RuntimeOption::FilesMatches;

std::string RuntimeOption::TakeoverFilename;
int RuntimeOption::AdminServerPort;
int RuntimeOption::AdminThreadCount = 1;
std::string RuntimeOption::AdminPassword;

std::string RuntimeOption::ProxyOrigin;
int RuntimeOption::ProxyRetry = 3;
bool RuntimeOption::UseServeURLs;
std::set<std::string> RuntimeOption::ServeURLs;
bool RuntimeOption::UseProxyURLs;
int RuntimeOption::ProxyPercentage = 0;
std::set<std::string> RuntimeOption::ProxyURLs;
std::vector<std::string> RuntimeOption::ProxyPatterns;

bool RuntimeOption::MySQLReadOnly = false;
bool RuntimeOption::MySQLLocalize = false;
int RuntimeOption::MySQLConnectTimeout = 1000;
int RuntimeOption::MySQLReadTimeout = 1000;
int RuntimeOption::MySQLWaitTimeout = -1;
int RuntimeOption::MySQLSlowQueryThreshold = 1000; // ms
bool RuntimeOption::MySQLKillOnTimeout = false;

int RuntimeOption::HttpDefaultTimeout = 30;
int RuntimeOption::HttpSlowQueryThreshold = 5000; // ms

bool RuntimeOption::TranslateLeakStackTrace = false;
bool RuntimeOption::NativeStackTrace = false;
bool RuntimeOption::FullBacktrace = false;
bool RuntimeOption::ServerStackTrace = false;
bool RuntimeOption::ServerErrorMessage = false;
bool RuntimeOption::TranslateSource = false;
bool RuntimeOption::RecordInput = false;
bool RuntimeOption::ClearInputOnSuccess = true;
std::string RuntimeOption::ProfilerOutputDir;
std::string RuntimeOption::CoreDumpEmail;
bool RuntimeOption::CoreDumpReport = true;
bool RuntimeOption::LocalMemcache = false;
bool RuntimeOption::MemcacheReadOnly = false;

bool RuntimeOption::EnableStats = false;
bool RuntimeOption::EnableWebStats = false;
bool RuntimeOption::EnableMemoryStats = false;
bool RuntimeOption::EnableMallocStats = false;
bool RuntimeOption::EnableAPCStats = false;
bool RuntimeOption::EnableAPCKeyStats = false;
bool RuntimeOption::EnableMemcacheStats = false;
bool RuntimeOption::EnableMemcacheKeyStats = false;
bool RuntimeOption::EnableSQLStats = false;
bool RuntimeOption::EnableSQLTableStats = false;
std::string RuntimeOption::StatsXSL;
std::string RuntimeOption::StatsXSLProxy;
int RuntimeOption::StatsSlotDuration = 10 * 60; // 10 minutes
int RuntimeOption::StatsMaxSlot = 12 * 6; // 12 hours

int64 RuntimeOption::MaxRSS = 0;
int64 RuntimeOption::MaxRSSPollingCycle = 0;
int64 RuntimeOption::DropCacheCycle = 0;
int64 RuntimeOption::MaxSQLRowCount = 10000;
int64 RuntimeOption::MaxMemcacheKeyCount = 0;
int RuntimeOption::SocketDefaultTimeout = 5;
bool RuntimeOption::EnableMemoryManager = true;
bool RuntimeOption::CheckMemory = false;
bool RuntimeOption::UseZendArray = true;
bool RuntimeOption::UseSmallArray = true;
bool RuntimeOption::EnableApc = true;
bool RuntimeOption::ApcUseSharedMemory = false;
int RuntimeOption::ApcSharedMemorySize = 1024; // 1GB
std::string RuntimeOption::ApcPrimeLibrary;
int RuntimeOption::ApcLoadThread = 1;
std::set<std::string> RuntimeOption::ApcCompletionKeys;
RuntimeOption::ApcTableTypes RuntimeOption::ApcTableType = ApcHashTable;
RuntimeOption::ApcTableLockTypes RuntimeOption::ApcTableLockType =
  ApcReadWriteLock;
time_t RuntimeOption::ApcKeyMaturityThreshold = 20;
size_t RuntimeOption::ApcMaximumCapacity = 0;
int RuntimeOption::ApcKeyFrequencyUpdatePeriod = 1000;
bool RuntimeOption::ApcUseLockedRefs = false;
bool RuntimeOption::ApcExpireOnSets = false;
int RuntimeOption::ApcPurgeFrequency = 4096;

bool RuntimeOption::EnableDnsCache = false;
int RuntimeOption::DnsCacheTTL = 10 * 60; // 10 minutes
time_t RuntimeOption::DnsCacheKeyMaturityThreshold = 20;
size_t RuntimeOption::DnsCacheMaximumCapacity = 0;
int RuntimeOption::DnsCacheKeyFrequencyUpdatePeriod = 1000;

std::map<std::string, std::string> RuntimeOption::ServerVariables;
std::map<std::string, std::string> RuntimeOption::EnvVariables;

std::string RuntimeOption::LightProcessFilePrefix;
int RuntimeOption::LightProcessCount;

bool RuntimeOption::EnableXHP = true;
bool RuntimeOption::EnableStrict = false;
int RuntimeOption::StrictLevel = 1; // StrictBasic, cf strict_mode.h
bool RuntimeOption::StrictFatal = false;
bool RuntimeOption::RecordCodeCoverage = false;
std::string RuntimeOption::CodeCoverageOutputFile;

bool RuntimeOption::SandboxMode = false;
std::string RuntimeOption::SandboxPattern;
std::string RuntimeOption::SandboxHome;
std::string RuntimeOption::SandboxConfFile;
std::map<std::string, std::string> RuntimeOption::SandboxServerVariables;

std::string RuntimeOption::SendmailPath;
std::string RuntimeOption::MailForceExtraParameters;

int RuntimeOption::PregBacktraceLimit = 100000;
int RuntimeOption::PregRecursionLimit = 100000;

///////////////////////////////////////////////////////////////////////////////
// keep this block after all the above static variables, or we will have
// static variable dependency problems on initialization

class DefaultRuntimeOptionLoader {
public:
  static DefaultRuntimeOptionLoader Loader;

  DefaultRuntimeOptionLoader() {
    Hdf empty;
    RuntimeOption::Load(empty); // so we load defaults defined in Load(Hdf)
  }
};

DefaultRuntimeOptionLoader DefaultRuntimeOptionLoader::Loader;

///////////////////////////////////////////////////////////////////////////////

void RequestInjection::pauseAndExit() {
  // NOTE: This is marked as __attribute__((noreturn)) in base/types.h
  // Signal sent, nothing can be trusted, don't do anything, as we might
  // write bad data, including calling exit handlers or destructors until the
  // signal handler (StackTrace) has had a chance to exit.
  sleep(300);
  // Should abort first, but it not try to exit
  pthread_exit(0);
}

static void setResourceLimit(int resource, Hdf rlimit, const char *nodeName) {
  if (!rlimit[nodeName].getString().empty()) {
    struct rlimit rl;
    getrlimit(resource, &rl);
    rl.rlim_cur = rlimit[nodeName].getInt64();
    if (rl.rlim_max < rl.rlim_cur) {
      rl.rlim_max = rl.rlim_cur;
    }
    int ret = setrlimit(resource, &rl);
    if (ret) {
      Logger::Error("Unable to set %s to %ld: %s (%d)", nodeName, rl.rlim_cur,
                    Util::safe_strerror(errno).c_str(), errno);
    }
  }
}

static void normalizePath(std::string &path) {
  if (!path.empty()) {
    if (path[path.length() - 1] == '/') {
      path = path.substr(0, path.length() - 1);
    }
    if (path[0] != '/') {
      path = std::string("/") + path;
    }
  }
}

void RuntimeOption::Load(Hdf &config) {
  PidFile = config["PidFile"].getString("www.pid");

  // Tier overwrites
  {
    Hdf tiers = config["Tiers"];
    string hostname = Process::GetHostName();
    for (Hdf hdf = tiers.firstChild(); hdf.exists(); hdf = hdf.next()) {
      string pattern = hdf["machine"].getString();
      if (!pattern.empty()) {
        Variant ret = preg_match(String(pattern.c_str(), pattern.size(),
                                        AttachLiteral),
                                 String(hostname.c_str(), hostname.size(),
                                        AttachLiteral));
        if (ret.toInt64() > 0) {
          Tier = hdf.getName();
          config.copy(hdf["overwrite"]);
          // no break here, so we can continue to match more overwrites
        }
      }
    }
  }

  {
    Hdf logger = config["Log"];
    if (logger["Level"] == "None") {
      Logger::LogLevel = Logger::LogNone;
    } else if (logger["Level"] == "Error") {
      Logger::LogLevel = Logger::LogError;
    } else if (logger["Level"] == "Warning") {
      Logger::LogLevel = Logger::LogWarning;
    } else if (logger["Level"] == "Info") {
      Logger::LogLevel = Logger::LogInfo;
    } else if (logger["Level"] == "Verbose") {
      Logger::LogLevel = Logger::LogVerbose;
    }
    Logger::LogHeader = logger["Header"].getBool();
    bool logInjectedStackTrace = logger["InjectedStackTrace"].getBool(false);
    if (logInjectedStackTrace) {
      Logger::SetTheLogger(new ExtendedLogger());
    }
    Logger::LogNativeStackTrace = logger["NativeStackTrace"].getBool(true);
    Logger::MaxMessagesPerRequest =
      logger["MaxMessagesPerRequest"].getInt32(-1);

    Logger::UseLogFile = logger["UseLogFile"].getBool(true);
    if (Logger::UseLogFile) {
      LogFile = logger["File"].getString();
    }

    Hdf aggregator = logger["Aggregator"];
    Logger::UseLogAggregator = aggregator.getBool();
    LogAggregatorFile = aggregator["File"].getString();
    LogAggregatorDatabase = aggregator["Database"].getString();
    LogAggregatorSleepSeconds = aggregator["SleepSeconds"].getInt16(10);

    AlwaysLogUnhandledExceptions =
      logger["AlwaysLogUnhandledExceptions"].getBool(true);
    NoSilencer = logger["NoSilencer"].getBool();
    EnableApplicationLog = logger["ApplicationLog"].getBool(true);
    RuntimeErrorReportingLevel =
      logger["RuntimeErrorReportingLevel"].getInt32(ErrorConstants::ALL);

    AccessLogDefaultFormat = logger["AccessLogDefaultFormat"].
      getString("%h %l %u %t \"%r\" %>s %b");
    {
      Hdf access = logger["Access"];
      for (Hdf hdf = access.firstChild(); hdf.exists();
           hdf = hdf.next()) {
        string fname = hdf["File"].getString();
        if (fname.empty()) {
          continue;
        }
        AccessLogs.
          push_back(pair<string, string>(fname, hdf["Format"].
                                         getString(AccessLogDefaultFormat)));
      }
    }

    AdminLogFormat = logger["AdminLog.Format"].getString("%h %t %s %U");
    AdminLogFile = logger["AdminLog.File"].getString();
  }
  {
    Hdf error = config["ErrorHandling"];
    CallUserHandlerOnFatals = error["CallUserHandlerOnFatals"].getBool(true);
    NoInfiniteLoopDetection = error["NoInfiniteLoopDetection"].getBool();
    NoInfiniteRecursionDetection =
      error["NoInfiniteRecursionDetection"].getBool();
    ThrowBadTypeExceptions = error["ThrowBadTypeExceptions"].getBool();
    ThrowTooManyArguments = error["ThrowTooManyArguments"].getBool();
    WarnTooManyArguments = error["WarnTooManyArguments"].getBool();
    ThrowMissingArguments = error["ThrowMissingArguments"].getBool();
    ThrowInvalidArguments = error["ThrowInvalidArguments"].getBool();
    AssertActive = error["AssertActive"].getBool();
    AssertWarning = error["AssertWarning"].getBool();
    NoticeFrequency = error["NoticeFrequency"].getInt32(1);
    WarningFrequency = error["WarningFrequency"].getInt32(1);
  }
  {
    Hdf rlimit = config["ResourceLimit"];
    setResourceLimit(RLIMIT_CORE,   rlimit, "CoreFileSize");
    setResourceLimit(RLIMIT_NOFILE, rlimit, "MaxSocket");
    setResourceLimit(RLIMIT_DATA,   rlimit, "RSS");
    MaxRSS = rlimit["MaxRSS"].getInt64(0);
    SocketDefaultTimeout = rlimit["SocketDefaultTimeout"].getInt16(5);
    MaxRSSPollingCycle = rlimit["MaxRSSPollingCycle"].getInt64(0);
    DropCacheCycle = rlimit["DropCacheCycle"].getInt64(0);
    MaxSQLRowCount = rlimit["MaxSQLRowCount"].getInt64(0);
    MaxMemcacheKeyCount = rlimit["MaxMemcacheKeyCount"].getInt64(0);
    SerializationSizeLimit = rlimit["SerializationSizeLimit"].getInt64(0);
  }
  {
    Hdf server = config["Server"];
    Host = server["Host"].getString();
    DefaultServerNameSuffix = server["DefaultServerNameSuffix"].getString();
    ServerIP = server["IP"].getString("0.0.0.0");
    ServerPrimaryIP = Util::GetPrimaryIP();
    ServerPort = server["Port"].getInt16(80);
    ServerThreadCount = server["ThreadCount"].getInt32(50);
    RequestTimeoutSeconds = server["RequestTimeoutSeconds"].getInt32(0);
    RequestMemoryMaxBytes = server["RequestMemoryMaxBytes"].getInt32(0);
    ResponseQueueCount = server["ResponseQueueCount"].getInt32(0);
    if (ResponseQueueCount <= 0) {
      ResponseQueueCount = ServerThreadCount / 10;
      if (ResponseQueueCount <= 0) ResponseQueueCount = 1;
    }
    ServerGracefulShutdownWait = server["GracefulShutdownWait"].getInt16(0);
    ServerHarshShutdown = server["HarshShutdown"].getBool(true);
    ServerEvilShutdown = server["EvilShutdown"].getBool(true);
    ServerDanglingWait = server["DanglingWait"].getInt16(0);
    if (ServerGracefulShutdownWait < ServerDanglingWait) {
      ServerGracefulShutdownWait = ServerDanglingWait;
    }
    GzipCompressionLevel = server["GzipCompressionLevel"].getInt16(3);
    EnableMagicQuotesGpc = server["EnableMagicQuotesGpc"].getBool();
    EnableKeepAlive = server["EnableKeepAlive"].getBool(true);
    ConnectionTimeoutSeconds = server["ConnectionTimeoutSeconds"].getInt16(-1);
    EnableEarlyFlush = server["EnableEarlyFlush"].getBool(true);
    ForceChunkedEncoding = server["ForceChunkedEncoding"].getBool();
    MaxPostSize = (server["MaxPostSize"].getInt32(150)) * (1 << 20);
    UploadMaxFileSize = (server["UploadMaxFileSize"].getInt32(10)) * (1 << 20);
    UploadTmpDir = server["UploadTmpDir"].getString("/tmp");
    ImageMemoryMaxBytes = server["ImageMemoryMaxBytes"].getInt32(0);
    if (ImageMemoryMaxBytes == 0) {
      ImageMemoryMaxBytes = UploadMaxFileSize * 2;
    }
    EnableFileUploads = server["EnableFileUploads"].getBool(true);
    EnableUploadProgress = server["EnableUploadProgress"].getBool(false);
    Rfc1867Freq = server["Rfc1867Freq"].getInt32(256 * 1024);
    if (Rfc1867Freq < 0) Rfc1867Freq = 256 * 1024;
    Rfc1867Prefix = server["Rfc1867Prefix"].getString("vupload_");
    Rfc1867Name = server["Rfc1867Name"].getString("video_ptoken");
    LibEventSyncSend = server["LibEventSyncSend"].getBool(true);
    TakeoverFilename = server["TakeoverFilename"].getString();
    ExpiresActive = server["ExpiresActive"].getBool(true);
    ExpiresDefault = server["ExpiresDefault"].getInt32(2592000);
    if (ExpiresDefault < 0) ExpiresDefault = 2592000;
    DefaultCharsetName = server["DefaultCharsetName"].getString("UTF-8");

    EnableSSL = server["EnableSSL"].getBool(false);
    SSLPort = server["SSLPort"].getInt16(443);
    SSLCertificateFile = server["SSLCertificateFile"].getString();
    SSLCertificateKeyFile = server["SSLCertificateKeyFile"].getString();

    SourceRoot = server["SourceRoot"].getString();
    if (!SourceRoot.empty() && SourceRoot[SourceRoot.length() - 1] != '/') {
      SourceRoot += '/';
    }
    if (!SourceRoot.empty()) {
      // Guaranteed empty on empty load so avoid setting FileCache::SourceRoot
      // since it may not be initialized
      FileCache::SourceRoot = SourceRoot;
    }
    server["IncludeSearchPaths"].get(IncludeSearchPaths);
    for (unsigned int i = 0; i < IncludeSearchPaths.size(); i++) {
      string &path = IncludeSearchPaths[i];
      if (!path.empty() && path[path.length() - 1] != '/') {
        path += '/';
      }
    }
    IncludeSearchPaths.insert(IncludeSearchPaths.begin(), "./");

    FileCache = server["FileCache"].getString();
    DefaultDocument = server["DefaultDocument"].getString();
    ErrorDocument404 = server["ErrorDocument404"].getString();
    normalizePath(ErrorDocument404);
    ErrorDocument500 = server["ErrorDocument500"].getString();
    normalizePath(ErrorDocument500);
    FatalErrorMessage = server["FatalErrorMessage"].getString();
    FontPath = server["FontPath"].getString();
    if (!FontPath.empty() && FontPath[FontPath.length() - 1] != '/') {
      FontPath += "/";
    }
    EnableStaticContentCache =
      server["EnableStaticContentCache"].getBool(true);
    EnableStaticContentFromDisk =
      server["EnableStaticContentFromDisk"].getBool(true);

    RTTIDirectory = server["RTTIDirectory"].getString("/tmp/");
    if (!RTTIDirectory.empty() &&
        RTTIDirectory[RTTIDirectory.length() - 1] != '/') {
      RTTIDirectory += "/";
    }
    EnableCliRTTI = server["EnableCliRTTI"].getBool();

    StartupDocument = server["StartupDocument"].getString();
    normalizePath(StartupDocument);
    WarmupDocument = server["WarmupDocument"].getString();
    RequestInitFunction = server["RequestInitFunction"].getString();
    server["ThreadDocuments"].get(ThreadDocuments);
    for (unsigned int i = 0; i < ThreadDocuments.size(); i++) {
      normalizePath(ThreadDocuments[i]);
    }

    SafeFileAccess = server["SafeFileAccess"].getBool();
    server["AllowedDirectories"].get(AllowedDirectories);
    for (unsigned int i = 0; i < AllowedDirectories.size(); i++) {
      string &directory = AllowedDirectories[i];
      char resolved_path[PATH_MAX];
      if (realpath(directory.c_str(), resolved_path) &&
          directory != resolved_path) {
        RuntimeOption::AllowedDirectories.push_back(resolved_path);
      }
    }
    RuntimeOption::AllowedDirectories.push_back(UploadTmpDir);
    server["AllowedFiles"].get(AllowedFiles);

    server["ForbiddenFileExtensions"].get(ForbiddenFileExtensions);

    EnableMemoryManager = server["EnableMemoryManager"].getBool(true);
    CheckMemory = server["CheckMemory"].getBool();
    UseZendArray = server["UseZendArray"].getBool(true);
    UseSmallArray = server["UseSmallArray"].getBool(true);

    Hdf apc = server["APC"];
    EnableApc = apc["EnableApc"].getBool(true);
    ApcUseSharedMemory = apc["UseSharedMemory"].getBool();
    ApcSharedMemorySize = apc["SharedMemorySize"].getInt32(1024 /* 1GB */);
    ApcPrimeLibrary = apc["PrimeLibrary"].getString();
    ApcLoadThread = apc["LoadThread"].getInt16(2);
    apc["CompletionKeys"].get(ApcCompletionKeys);

    string apcTableType = apc["TableType"].getString("hash");
    if (strcasecmp(apcTableType.c_str(), "hash") == 0) {
      ApcTableType = ApcHashTable;
    } else if (strcasecmp(apcTableType.c_str(), "concurrent") == 0) {
      ApcTableType = ApcConcurrentTable;
    } else {
      throw InvalidArgumentException("apc table type",
                                     "Invalid table type");
    }
    string apcLockType = apc["LockType"].getString("readwritelock");
    if (strcasecmp(apcLockType.c_str(), "readwritelock") == 0) {
      ApcTableLockType = ApcReadWriteLock;
    } else if (strcasecmp(apcLockType.c_str(), "mutex") == 0) {
      ApcTableLockType = ApcMutex;
    } else {
      throw InvalidArgumentException("apc lock type",
                                     "Invalid lock type");
    }

    ApcUseLockedRefs = apc["UseLockedRefs"].getBool();
    ApcExpireOnSets = apc["ExpireOnSets"].getBool();
    ApcPurgeFrequency = apc["PurgeFrequency"].getInt32(4096);

    ApcKeyMaturityThreshold = apc["KeyMaturityThreshold"].getInt32(20);
    ApcMaximumCapacity = apc["MaximumCapacity"].getInt64(0);
    ApcKeyFrequencyUpdatePeriod = apc["KeyFrequencyUpdatePeriod"].
      getInt32(1000);


    Hdf dns = server["DnsCache"];
    EnableDnsCache = dns["Enable"].getBool();
    DnsCacheTTL = dns["TTL"].getInt32(600); // 10 minutes
    DnsCacheKeyMaturityThreshold = dns["KeyMaturityThreshold"].getInt32(20);
    DnsCacheMaximumCapacity = dns["MaximumCapacity"].getInt64(0);
    DnsCacheKeyFrequencyUpdatePeriod = dns["KeyFrequencyUpdatePeriod"].
      getInt32(1000);

    SharedStores::Create();

    LightProcessFilePrefix =
      server["LightProcessFilePrefix"].getString("./lightprocess");
    LightProcessCount = server["LightProcessCount"].getInt32(0);

    InjectedStackTrace = server["InjectedStackTrace"].getBool(true);

    ForceServerNameToHeader = server["ForceServerNameToHeader"].getBool();
  }
  {
    Hdf hosts = config["VirtualHost"];
    if (hosts.exists()) {
      for (Hdf hdf = hosts.firstChild(); hdf.exists(); hdf = hdf.next()) {
        if (hdf.getName() == "default") {
          VirtualHost::GetDefault().init(hdf);
        } else {
          VirtualHostPtr host(new VirtualHost(hdf));
          VirtualHosts.push_back(host);
        }
      }
      for (unsigned int i = 0; i < VirtualHosts.size(); i++) {
        if (!VirtualHosts[i]->valid()) {
          throw InvalidArgumentException("virtual host",
                                         "missing prefix or pattern");
        }
      }
    }
  }
  {
    Hdf ipblocks = config["IpBlockMap"];
    IpBlocks = IpBlockMapPtr(new IpBlockMap(ipblocks));
  }
  {
    Hdf satellites = config["Satellites"];
    if (satellites.exists()) {
      for (Hdf hdf = satellites.firstChild(); hdf.exists(); hdf = hdf.next()) {
        SatelliteServerInfoPtr satellite(new SatelliteServerInfo(hdf));
        SatelliteServerInfos.push_back(satellite);
        if (satellite->getType() == SatelliteServer::KindOfRPCServer) {
          XboxPassword = satellite->getPassword();
        }
      }
    }
  }
  {
    Hdf xbox = config["Xbox"];
    XboxServerThreadCount = xbox["ServerInfo.ThreadCount"].getInt32(0);
    XboxServerPort = xbox["ServerInfo.Port"].getInt32(0);
    XboxDefaultLocalTimeoutMilliSeconds =
      xbox["DefaultLocalTimeoutMilliSeconds"].getInt32(500);
    XboxDefaultRemoteTimeoutSeconds =
      xbox["DefaultRemoteTimeoutSeconds"].getInt32(5);
    XboxServerInfoMaxRequest = xbox["ServerInfo.MaxRequest"].getInt32(500);
    XboxServerInfoDuration = xbox["ServerInfo.MaxDuration"].getInt32(120);
    XboxServerInfoWarmupDoc = xbox["ServerInfo.WarmupDocument"].get("");
    XboxServerInfoReqInitFunc = xbox["ServerInfo.RequestInitFunction"].get("");
    XboxProcessMessageFunc =
      xbox["ProcessMessageFunc"].get("xbox_process_message");
  }
  {
    PageletServerThreadCount = config["PageletServer.ThreadCount"].getInt32(0);
    FiberCount = config["Fiber.ThreadCount"].getInt32(0);
  }
  {
    Hdf content = config["StaticFile"];
    content["Extensions"].get(StaticFileExtensions);
    content["Generators"].get(StaticFileGenerators);

    Hdf matches = content["FilesMatch"];
    if (matches.exists()) {
      for (Hdf hdf = matches.firstChild(); hdf.exists(); hdf = hdf.next()) {
        FilesMatches.push_back(FilesMatchPtr(new FilesMatch(hdf)));
      }
    }
  }
  {
    Hdf admin = config["AdminServer"];
    AdminServerPort = admin["Port"].getInt16(8088);
    AdminThreadCount = admin["ThreadCount"].getInt32(1);
    AdminPassword = admin["Password"].getString();
  }
  {
    Hdf proxy = config["Proxy"];
    ProxyOrigin = proxy["Origin"].getString();
    ProxyRetry = proxy["Retry"].getInt16(3);
    UseServeURLs = proxy["ServeURLs"].getBool();
    proxy["ServeURLs"].get(ServeURLs);
    UseProxyURLs = proxy["ProxyURLs"].getBool();
    ProxyPercentage = proxy["Percentage"].getByte(0);
    proxy["ProxyURLs"].get(ProxyURLs);
    proxy["ProxyPatterns"].get(ProxyPatterns);
  }
  {
    Hdf mysql = config["MySQL"];
    MySQLReadOnly = mysql["ReadOnly"].getBool();
    MySQLLocalize = mysql["Localize"].getBool();
    MySQLConnectTimeout = mysql["ConnectTimeout"].getInt32(1000);
    MySQLReadTimeout = mysql["ReadTimeout"].getInt32(1000);
    MySQLWaitTimeout = mysql["WaitTimeout"].getInt32(-1);
    MySQLSlowQueryThreshold = mysql["SlowQueryThreshold"].getInt32(1000);
    MySQLKillOnTimeout = mysql["KillOnTimeout"].getBool();
  }
  {
    Hdf http = config["Http"];
    HttpDefaultTimeout = http["DefaultTimeout"].getInt32(30);
    HttpSlowQueryThreshold = http["SlowQueryThreshold"].getInt32(5000);
  }
  {
    Hdf debug = config["Debug"];
    NativeStackTrace = debug["NativeStackTrace"].getBool();
    StackTrace::Enabled = NativeStackTrace;
    TranslateLeakStackTrace = debug["TranslateLeakStackTrace"].getBool();
    FullBacktrace = debug["FullBacktrace"].getBool();
    ServerStackTrace = debug["ServerStackTrace"].getBool();
    ServerErrorMessage = debug["ServerErrorMessage"].getBool();
    TranslateSource = debug["TranslateSource"].getBool();
    RecordInput = debug["RecordInput"].getBool();
    ClearInputOnSuccess = debug["ClearInputOnSuccess"].getBool(true);
    ProfilerOutputDir = debug["ProfilerOutputDir"].getString("/tmp");
    CoreDumpEmail = debug["CoreDumpEmail"].getString();
    if (!CoreDumpEmail.empty()) {
      StackTrace::ReportEmail = CoreDumpEmail;
    }
    CoreDumpReport = debug["CoreDumpReport"].getBool(true);
    if (CoreDumpReport) {
      StackTrace::InstallReportOnErrors();
    }
    LocalMemcache = debug["LocalMemcache"].getBool();
    MemcacheReadOnly = debug["MemcacheReadOnly"].getBool();
  }
  {
    Hdf stats = config["Stats"];
    EnableStats = stats.getBool(); // main switch

    EnableWebStats = stats["Web"].getBool();
    EnableMemoryStats = stats["Memory"].getBool();
    EnableMallocStats = stats["Malloc"].getBool();
    EnableAPCStats = stats["APC"].getBool();
    EnableAPCKeyStats = stats["APCKey"].getBool();
    EnableMemcacheStats = stats["Memcache"].getBool();
    EnableMemcacheKeyStats = stats["MemcacheKey"].getBool();
    EnableSQLStats = stats["SQL"].getBool();
    EnableSQLTableStats = stats["SQLTable"].getBool();

    if (EnableStats && EnableMallocStats) {
      LeakDetectable::EnableMallocStats(true);
    }

    StatsXSL = stats["XSL"].getString();
    StatsXSLProxy = stats["XSLProxy"].getString();

    StatsSlotDuration = stats["SlotDuration"].getInt32(10 * 60); // 10 minutes
    StatsMaxSlot = stats["MaxSlot"].getInt32(12 * 6); // 12 hours
  }
  {
    config["ServerVariables"].get(ServerVariables);
    config["EnvVariables"].get(EnvVariables);
  }
  {
    Hdf eval = config["Eval"];
    EnableXHP = eval["EnableXHP"].getBool(true);
    EnableStrict = eval["EnableStrict"].getBool(0);
    StrictLevel = eval["StrictLevel"].getInt32(1); // StrictBasic
    StrictFatal = eval["StrictFatal"].getBool();
    RecordCodeCoverage = eval["RecordCodeCoverage"].getBool(false);
    CodeCoverageOutputFile = eval["CodeCoverageOutputFile"].getString();
  }
  {
    Hdf sandbox = config["Sandbox"];
    SandboxMode = sandbox["SandboxMode"].getBool();
    SandboxPattern = format_pattern(sandbox["Pattern"].getString());
    SandboxHome = sandbox["Home"].getString();
    SandboxConfFile = sandbox["ConfFile"].getString();
    sandbox["ServerVariables"].get(SandboxServerVariables);
  }
  {
    Hdf mail = config["Mail"];
    SendmailPath = mail["SendmailPath"].getString("sendmail -t -i");
    MailForceExtraParameters = mail["ForceExtraParameters"].getString();
  }
  {
    Hdf preg = config["Preg"];
    PregBacktraceLimit = preg["BacktraceLimit"].getInt32(100000);
    PregRecursionLimit = preg["RecursionLimit"].getInt32(100000);
  }

  Extension::LoadModules(config);
}

///////////////////////////////////////////////////////////////////////////////
}
