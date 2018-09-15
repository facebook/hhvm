/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/runtime-option.h"

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <folly/CPortability.h>
#include <folly/FileUtil.h>
#include <folly/String.h>
#include <folly/portability/SysResource.h>
#include <folly/portability/SysTime.h>
#include <folly/portability/Unistd.h>

#include "hphp/util/arch.h"
#include "hphp/util/atomic-vector.h"
#include "hphp/util/build-info.h"
#include "hphp/util/cpuid.h"
#include "hphp/util/current-executable.h"
#include "hphp/util/hdf.h"
#include "hphp/util/hphp-config.h"
#include "hphp/util/text-util.h"
#include "hphp/util/network.h"
#include "hphp/util/hardware-counter.h"
#include "hphp/util/logger.h"
#include "hphp/util/stack-trace.h"
#include "hphp/util/process.h"
#include "hphp/util/file-cache.h"
#include "hphp/util/log-file-flusher.h"

#if defined (__linux__) && defined (__aarch64__)
#include <sys/auxv.h>
#include <asm/hwcap.h>
#endif

#include "hphp/parser/scanner.h"

#include "hphp/runtime/server/satellite-server.h"
#include "hphp/runtime/server/virtual-host.h"
#include "hphp/runtime/server/files-match.h"
#include "hphp/runtime/server/access-log.h"
#include "hphp/runtime/server/cli-server.h"

#include "hphp/runtime/base/apc-file-storage.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/config.h"
#include "hphp/runtime/base/crash-reporter.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/file-util-defs.h"
#include "hphp/runtime/base/hphp-system.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/simple-counter.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/thread-info.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/ext/extension-registry.h"

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/mcgen-translate.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

std::string RuntimeOption::BuildId;
std::string RuntimeOption::InstanceId;
std::string RuntimeOption::DeploymentId;
std::string RuntimeOption::PidFile = "www.pid";

bool RuntimeOption::ServerMode = false;

bool RuntimeOption::EnableHipHopSyntax = false;
bool RuntimeOption::EnableHipHopExperimentalSyntax = false;
bool RuntimeOption::EnableShortTags = true;
bool RuntimeOption::EnableAspTags = false;
bool RuntimeOption::EnableXHP = false;
bool RuntimeOption::EnableObjDestructCall = true;
bool RuntimeOption::EnableIntrinsicsExtension = false;
bool RuntimeOption::CheckSymLink = true;
bool RuntimeOption::EnableArgsInBacktraces = true;
bool RuntimeOption::EnableContextInErrorHandler = true;
bool RuntimeOption::EnableZendSorting = false;
bool RuntimeOption::EnableZendIniCompat = true;
bool RuntimeOption::TimeoutsUseWallTime = true;
bool RuntimeOption::CheckFlushOnUserClose = true;
bool RuntimeOption::EvalAuthoritativeMode = false;
bool RuntimeOption::IntsOverflowToInts = false;
bool RuntimeOption::AutoprimeGenerators = true;
bool RuntimeOption::EnableIsExprPrimitiveMigration = true;
bool RuntimeOption::EnableReifiedGenerics = false;
bool RuntimeOption::Hacksperimental = false;
bool RuntimeOption::CheckParamTypeInvariance = true;
bool RuntimeOption::DumpPreciseProfileData = true;
bool RuntimeOption::EnableCoroutines = true;
uint32_t RuntimeOption::EvalInitialStaticStringTableSize =
  kDefaultInitialStaticStringTableSize;
uint32_t RuntimeOption::EvalInitialNamedEntityTableSize = 30000;
JitSerdesMode RuntimeOption::EvalJitSerdesMode{};
std::string RuntimeOption::EvalJitSerdesFile;

std::map<std::string, ErrorLogFileData> RuntimeOption::ErrorLogs = {
  {Logger::DEFAULT, ErrorLogFileData()},
};
// these hold the DEFAULT logger
std::string RuntimeOption::LogFile;
std::string RuntimeOption::LogFileSymLink;
uint16_t RuntimeOption::LogFilePeriodMultiplier;

int RuntimeOption::LogHeaderMangle = 0;
bool RuntimeOption::AlwaysLogUnhandledExceptions =
  RuntimeOption::EnableHipHopSyntax;
bool RuntimeOption::AlwaysEscapeLog = true;
bool RuntimeOption::NoSilencer = false;
int RuntimeOption::ErrorUpgradeLevel = 0;
bool RuntimeOption::CallUserHandlerOnFatals = false;
bool RuntimeOption::ThrowExceptionOnBadMethodCall = true;
bool RuntimeOption::LogNativeStackOnOOM = true;
int RuntimeOption::RuntimeErrorReportingLevel =
  static_cast<int>(ErrorMode::HPHP_ALL);
int RuntimeOption::ForceErrorReportingLevel = 0;

std::string RuntimeOption::ServerUser;
std::vector<std::string> RuntimeOption::TzdataSearchPaths;

int RuntimeOption::MaxSerializedStringSize = 64 * 1024 * 1024; // 64MB
bool RuntimeOption::NoInfiniteRecursionDetection = false;
bool RuntimeOption::AssertEmitted = true;
int64_t RuntimeOption::NoticeFrequency = 1;
int64_t RuntimeOption::WarningFrequency = 1;
int RuntimeOption::RaiseDebuggingFrequency = 1;
int64_t RuntimeOption::SerializationSizeLimit = StringData::MaxSize;

std::string RuntimeOption::AccessLogDefaultFormat = "%h %l %u %t \"%r\" %>s %b";
std::map<std::string, AccessLogFileData> RuntimeOption::AccessLogs;

std::string RuntimeOption::AdminLogFormat = "%h %t %s %U";
std::string RuntimeOption::AdminLogFile;
std::string RuntimeOption::AdminLogSymLink;

std::map<std::string, AccessLogFileData> RuntimeOption::RPCLogs;

std::string RuntimeOption::Host;
std::string RuntimeOption::DefaultServerNameSuffix;
std::string RuntimeOption::ServerType = "proxygen";
std::string RuntimeOption::ServerIP;
std::string RuntimeOption::ServerFileSocket;
int RuntimeOption::ServerPort = 80;
int RuntimeOption::ServerPortFd = -1;
int RuntimeOption::ServerBacklog = 128;
int RuntimeOption::ServerConnectionLimit = 0;
int RuntimeOption::ServerThreadCount = 50;
int RuntimeOption::ServerQueueCount = 50;
int RuntimeOption::ServerHugeThreadCount = 0;
int RuntimeOption::ServerHugeStackKb = 384;
uint32_t RuntimeOption::ServerLoopSampleRate = 0;
int RuntimeOption::ServerWarmupThrottleRequestCount = 0;
int RuntimeOption::ServerThreadDropCacheTimeoutSeconds = 0;
int RuntimeOption::ServerThreadJobLIFOSwitchThreshold = INT_MAX;
int RuntimeOption::ServerThreadJobMaxQueuingMilliSeconds = -1;
bool RuntimeOption::AlwaysDecodePostDataDefault = true;
bool RuntimeOption::ServerThreadDropStack = false;
bool RuntimeOption::ServerHttpSafeMode = false;
bool RuntimeOption::ServerStatCache = false;
bool RuntimeOption::ServerFixPathInfo = false;
bool RuntimeOption::ServerAddVaryEncoding = true;
bool RuntimeOption::ServerLogSettingsOnStartup = false;
bool RuntimeOption::ServerForkEnabled = true;
bool RuntimeOption::ServerForkLogging = false;
std::vector<std::string> RuntimeOption::ServerWarmupRequests;
std::string RuntimeOption::ServerCleanupRequest;
int RuntimeOption::ServerInternalWarmupThreads = 0;
boost::container::flat_set<std::string>
RuntimeOption::ServerHighPriorityEndPoints;
bool RuntimeOption::ServerExitOnBindFail;
int RuntimeOption::PageletServerThreadCount = 0;
int RuntimeOption::PageletServerHugeThreadCount = 0;
int RuntimeOption::PageletServerThreadDropCacheTimeoutSeconds = 0;
int RuntimeOption::PageletServerQueueLimit = 0;
bool RuntimeOption::PageletServerThreadDropStack = false;
int RuntimeOption::RequestTimeoutSeconds = 0;
int RuntimeOption::PspTimeoutSeconds = 0;
int RuntimeOption::PspCpuTimeoutSeconds = 0;
int64_t RuntimeOption::MaxRequestAgeFactor = 0;
int64_t RuntimeOption::RequestMemoryMaxBytes =
  std::numeric_limits<int64_t>::max();
int64_t RuntimeOption::RequestMemoryOOMKillBytes =
  std::numeric_limits<int64_t>::max();
int64_t RuntimeOption::RequestHugeMaxBytes = 0;
int64_t RuntimeOption::ImageMemoryMaxBytes = 0;
int RuntimeOption::ServerGracefulShutdownWait = 0;
bool RuntimeOption::ServerHarshShutdown = true;
bool RuntimeOption::ServerEvilShutdown = true;
bool RuntimeOption::ServerKillOnSIGTERM = false;
bool RuntimeOption::ServerKillOnTimeout = true;
int RuntimeOption::ServerPreShutdownWait = 0;
int RuntimeOption::ServerShutdownListenWait = 0;
int RuntimeOption::ServerShutdownEOMWait = 0;
int RuntimeOption::ServerPrepareToStopTimeout = 0;
int RuntimeOption::ServerPartialPostStatusCode = -1;
bool RuntimeOption::StopOldServer = false;
int RuntimeOption::OldServerWait = 30;
int RuntimeOption::CacheFreeFactor = 50;
int64_t RuntimeOption::ServerRSSNeededMb = 4096;
int64_t RuntimeOption::ServerCriticalFreeMb = 512;
std::vector<std::string> RuntimeOption::ServerNextProtocols;
bool RuntimeOption::ServerEnableH2C = false;
int RuntimeOption::BrotliCompressionEnabled = -1;
int RuntimeOption::BrotliChunkedCompressionEnabled = -1;
int RuntimeOption::BrotliCompressionMode = 0;
int RuntimeOption::BrotliCompressionQuality = 6;
int RuntimeOption::BrotliCompressionLgWindowSize = 20;
int RuntimeOption::ZstdCompressionEnabled = -1;
int RuntimeOption::ZstdCompressionLevel = 3;
int RuntimeOption::GzipCompressionLevel = 3;
int RuntimeOption::GzipMaxCompressionLevel = 9;
std::string RuntimeOption::ForceCompressionURL;
std::string RuntimeOption::ForceCompressionCookie;
std::string RuntimeOption::ForceCompressionParam;
bool RuntimeOption::EnableKeepAlive = true;
bool RuntimeOption::ExposeHPHP = true;
bool RuntimeOption::ExposeXFBServer = false;
bool RuntimeOption::ExposeXFBDebug = false;
std::string RuntimeOption::XFBDebugSSLKey;
int RuntimeOption::ConnectionTimeoutSeconds = -1;
bool RuntimeOption::EnableOutputBuffering = false;
std::string RuntimeOption::OutputHandler;
bool RuntimeOption::ImplicitFlush = false;
bool RuntimeOption::EnableEarlyFlush = true;
bool RuntimeOption::ForceChunkedEncoding = false;
int64_t RuntimeOption::MaxPostSize = 100;
int64_t RuntimeOption::LowestMaxPostSize = LLONG_MAX;
bool RuntimeOption::AlwaysPopulateRawPostData = false;
int64_t RuntimeOption::UploadMaxFileSize = 100;
std::string RuntimeOption::UploadTmpDir = "/tmp";
bool RuntimeOption::EnableFileUploads = true;
bool RuntimeOption::EnableUploadProgress = false;
int64_t RuntimeOption::MaxFileUploads = 20;
int RuntimeOption::Rfc1867Freq = 256 * 1024;
std::string RuntimeOption::Rfc1867Prefix = "vupload_";
std::string RuntimeOption::Rfc1867Name = "video_ptoken";
bool RuntimeOption::ExpiresActive = true;
int RuntimeOption::ExpiresDefault = 2592000;
std::string RuntimeOption::DefaultCharsetName = "";
bool RuntimeOption::ForceServerNameToHeader = false;
bool RuntimeOption::PathDebug = false;

int64_t RuntimeOption::RequestBodyReadLimit = -1;

bool RuntimeOption::EnableSSL = false;
int RuntimeOption::SSLPort = 443;
int RuntimeOption::SSLPortFd = -1;
std::string RuntimeOption::SSLCertificateFile;
std::string RuntimeOption::SSLCertificateKeyFile;
std::string RuntimeOption::SSLCertificateDir;
std::string RuntimeOption::SSLTicketSeedFile;
bool RuntimeOption::TLSDisableTLS1_2 = false;
std::string RuntimeOption::TLSClientCipherSpec;
bool RuntimeOption::EnableSSLWithPlainText = false;

std::vector<std::shared_ptr<VirtualHost>> RuntimeOption::VirtualHosts;
std::shared_ptr<IpBlockMap> RuntimeOption::IpBlocks;
std::vector<std::shared_ptr<SatelliteServerInfo>>
  RuntimeOption::SatelliteServerInfos;

bool RuntimeOption::AllowRunAsRoot = false; // Allow running hhvm as root.

int RuntimeOption::XboxServerThreadCount = 10;
int RuntimeOption::XboxServerMaxQueueLength = INT_MAX;
int RuntimeOption::XboxServerPort = 0;
int RuntimeOption::XboxDefaultLocalTimeoutMilliSeconds = 500;
int RuntimeOption::XboxDefaultRemoteTimeoutSeconds = 5;
int RuntimeOption::XboxServerInfoMaxRequest = 500;
int RuntimeOption::XboxServerInfoDuration = 120;
std::string RuntimeOption::XboxServerInfoWarmupDoc;
std::string RuntimeOption::XboxServerInfoReqInitFunc;
std::string RuntimeOption::XboxServerInfoReqInitDoc;
bool RuntimeOption::XboxServerInfoAlwaysReset = false;
bool RuntimeOption::XboxServerLogInfo = false;
std::string RuntimeOption::XboxProcessMessageFunc = "xbox_process_message";
std::string RuntimeOption::XboxPassword;
std::set<std::string> RuntimeOption::XboxPasswords;

std::string RuntimeOption::SourceRoot = Process::GetCurrentDirectory() + '/';
std::vector<std::string> RuntimeOption::IncludeSearchPaths;
std::map<std::string, std::string> RuntimeOption::IncludeRoots;
std::map<std::string, std::string> RuntimeOption::AutoloadRoots;
std::string RuntimeOption::FileCache;
std::string RuntimeOption::DefaultDocument;
std::string RuntimeOption::ErrorDocument404;
bool RuntimeOption::ForbiddenAs404 = false;
std::string RuntimeOption::ErrorDocument500;
std::string RuntimeOption::FatalErrorMessage;
std::string RuntimeOption::FontPath;
bool RuntimeOption::EnableStaticContentFromDisk = true;
bool RuntimeOption::EnableOnDemandUncompress = true;
bool RuntimeOption::EnableStaticContentMMap = true;

bool RuntimeOption::Utf8izeReplace = true;

std::string RuntimeOption::RequestInitFunction;
std::string RuntimeOption::RequestInitDocument;
std::string RuntimeOption::AutoPrependFile;
std::string RuntimeOption::AutoAppendFile;

bool RuntimeOption::SafeFileAccess = false;
std::vector<std::string> RuntimeOption::AllowedDirectories;
std::set<std::string> RuntimeOption::AllowedFiles;
hphp_string_imap<std::string> RuntimeOption::StaticFileExtensions;
hphp_string_imap<std::string> RuntimeOption::PhpFileExtensions;
std::set<std::string> RuntimeOption::ForbiddenFileExtensions;
std::vector<std::shared_ptr<FilesMatch>> RuntimeOption::FilesMatches;

bool RuntimeOption::WhitelistExec = false;
bool RuntimeOption::WhitelistExecWarningOnly = false;
std::vector<std::string> RuntimeOption::AllowedExecCmds;

bool RuntimeOption::UnserializationWhitelistCheck = false;
bool RuntimeOption::UnserializationWhitelistCheckWarningOnly = true;
int64_t RuntimeOption::UnserializationBigMapThreshold = 1 << 16;

std::string RuntimeOption::TakeoverFilename;
std::string RuntimeOption::AdminServerIP;
int RuntimeOption::AdminServerPort = 0;
int RuntimeOption::AdminThreadCount = 1;
std::string RuntimeOption::AdminPassword;
std::set<std::string> RuntimeOption::AdminPasswords;
std::set<std::string> RuntimeOption::HashedAdminPasswords;

std::string RuntimeOption::ProxyOriginRaw;
int RuntimeOption::ProxyPercentageRaw = 0;
int RuntimeOption::ProxyRetry = 3;
bool RuntimeOption::UseServeURLs;
std::set<std::string> RuntimeOption::ServeURLs;
bool RuntimeOption::UseProxyURLs;
std::set<std::string> RuntimeOption::ProxyURLs;
std::vector<std::string> RuntimeOption::ProxyPatterns;
bool RuntimeOption::AlwaysUseRelativePath = false;

int RuntimeOption::HttpDefaultTimeout = 30;
int RuntimeOption::HttpSlowQueryThreshold = 5000; // ms

bool RuntimeOption::NativeStackTrace = false;
bool RuntimeOption::ServerErrorMessage = false;
bool RuntimeOption::RecordInput = false;
bool RuntimeOption::ClearInputOnSuccess = true;
std::string RuntimeOption::ProfilerOutputDir = "/tmp";
std::string RuntimeOption::CoreDumpEmail;
bool RuntimeOption::CoreDumpReport = true;
std::string RuntimeOption::CoreDumpReportDirectory =
#if defined(HPHP_OSS)
  "/tmp";
#else
  "/var/tmp/cores";
#endif
std::string RuntimeOption::StackTraceFilename;
int RuntimeOption::StackTraceTimeout = 0; // seconds; 0 means unlimited
std::string RuntimeOption::RemoteTraceOutputDir = "/tmp";
std::set<std::string, stdltistr> RuntimeOption::TraceFunctions;
uint32_t RuntimeOption::TraceFuncId = InvalidFuncId;

bool RuntimeOption::EnableStats = false;
bool RuntimeOption::EnableAPCStats = false;
bool RuntimeOption::EnableWebStats = false;
bool RuntimeOption::EnableMemoryStats = false;
bool RuntimeOption::EnableSQLStats = false;
bool RuntimeOption::EnableSQLTableStats = false;
bool RuntimeOption::EnableNetworkIOStatus = false;
std::string RuntimeOption::StatsXSL;
std::string RuntimeOption::StatsXSLProxy;
int RuntimeOption::StatsSlotDuration = 10 * 60; // 10 minutes
int RuntimeOption::StatsMaxSlot = 12 * 6; // 12 hours

int64_t RuntimeOption::MaxSQLRowCount = 0;
int64_t RuntimeOption::SocketDefaultTimeout = 60;
bool RuntimeOption::LockCodeMemory = false;
int RuntimeOption::MaxArrayChain = INT_MAX;
bool RuntimeOption::WarnOnCollectionToArray = false;
bool RuntimeOption::UseDirectCopy = false;

#if FOLLY_SANITIZE
bool RuntimeOption::DisableSmallAllocator = true;
#else
bool RuntimeOption::DisableSmallAllocator = false;
#endif

int RuntimeOption::PerAllocSampleF = 100 * 1000 * 1000;
int RuntimeOption::TotalAllocSampleF = 1 * 1000 * 1000;

std::map<std::string, std::string> RuntimeOption::ServerVariables;
std::map<std::string, std::string> RuntimeOption::EnvVariables;

std::string RuntimeOption::LightProcessFilePrefix = "./lightprocess";
int RuntimeOption::LightProcessCount = 0;

int64_t RuntimeOption::HeapSizeMB = 4096; // 4gb
int64_t RuntimeOption::HeapResetCountBase = 1;
int64_t RuntimeOption::HeapResetCountMultiple = 2;
int64_t RuntimeOption::HeapLowWaterMark = 16;
int64_t RuntimeOption::HeapHighWaterMark = 1024;

uint64_t RuntimeOption::DisableCompact = 0;
uint64_t RuntimeOption::DisableExtract = 0;
uint64_t RuntimeOption::DisableForwardStaticCall = 0;
uint64_t RuntimeOption::DisableForwardStaticCallArray = 0;

#ifdef HHVM_DYNAMIC_EXTENSION_DIR
std::string RuntimeOption::ExtensionDir = HHVM_DYNAMIC_EXTENSION_DIR;
#else
std::string RuntimeOption::ExtensionDir = "";
#endif

std::vector<std::string> RuntimeOption::Extensions;
std::vector<std::string> RuntimeOption::DynamicExtensions;
std::string RuntimeOption::DynamicExtensionPath = ".";

HackStrictOption
  RuntimeOption::StrictArrayFillKeys = HackStrictOption::OFF,
  RuntimeOption::DisallowDynamicVarEnvFuncs = HackStrictOption::OFF,
  RuntimeOption::IconvIgnoreCorrect = HackStrictOption::OFF,
  RuntimeOption::MinMaxAllowDegenerate = HackStrictOption::OFF;

// defaults set when the INI option is bound - values below are irrelevant.
bool RuntimeOption::LookForTypechecker = false;
bool RuntimeOption::AutoTypecheck = false;

#ifdef FACEBOOK
const static bool s_PHP7_default = false;
#else
const static bool s_PHP7_default = true;
#endif
// PHP7 is off by default (false). s_PHP7_master is not a static member of
// RuntimeOption so that it's private to this file and not exposed -- it's a
// master switch only, and not to be used for any actual gating, use the more
// granular options instead. (It can't be a local since Config::Bind will take
// and store a pointer to it.)
static bool s_PHP7_master = s_PHP7_default;
bool RuntimeOption::PHP7_DeprecationWarnings = false;
bool RuntimeOption::PHP7_EngineExceptions = false;
bool RuntimeOption::PHP7_IntSemantics = false;
bool RuntimeOption::PHP7_LTR_assign = false;
bool RuntimeOption::PHP7_NoHexNumerics = false;
bool RuntimeOption::PHP7_Builtins = false;
bool RuntimeOption::PHP7_ScalarTypes = false;
bool RuntimeOption::PHP7_Substr = false;
bool RuntimeOption::PHP7_UVS = false;
bool RuntimeOption::PHP7_DisallowUnsafeCurlUploads = false;

std::map<std::string, std::string> RuntimeOption::AliasedNamespaces;
std::vector<std::string> s_RelativeConfigs;

int RuntimeOption::GetScannerType() {
  int type = 0;
  if (EnableShortTags) type |= Scanner::AllowShortTags;
  if (EnableAspTags) type |= Scanner::AllowAspTags;
  if (EnableXHP) type |= Scanner::AllowXHPSyntax;
  if (EnableHipHopSyntax) type |= Scanner::AllowHipHopSyntax;
  return type;
}

const std::string& RuntimeOption::GetServerPrimaryIPv4() {
   static std::string serverPrimaryIPv4 = GetPrimaryIPv4();
   return serverPrimaryIPv4;
}

const std::string& RuntimeOption::GetServerPrimaryIPv6() {
   static std::string serverPrimaryIPv6 = GetPrimaryIPv6();
   return serverPrimaryIPv6;
}

static inline std::string regionSelectorDefault() {
  return "tracelet";
}

static inline bool pgoDefault() {
#ifdef HHVM_NO_DEFAULT_PGO
  return false;
#else
  return true;
#endif
}

static inline bool eagerGcDefault() {
#ifdef HHVM_EAGER_GC
  return true;
#else
  return false;
#endif
}

static inline std::string hackCompilerArgsDefault() {
  return RuntimeOption::RepoAuthoritative
    ? "-v Hack.Compiler.SourceMapping=1 --daemon --dump-symbol-refs"
    : "-v Hack.Compiler.SourceMapping=1 --daemon";
}

static inline std::string hackCompilerCommandDefault() {
#ifdef FACEBOOK
  return "";
#else
  std::string hackc = folly::sformat(
    "{}/hh_single_compile",
    current_executable_directory()
  );
  if (::access(hackc.data(), X_OK) != 0) {
#ifndef HACKC_FALLBACK_PATH
    return "";
#else
    hackc = HACKC_FALLBACK_PATH;
    if (::access(hackc.data(), X_OK) != 0) {
      return "";
    }
#endif
  }

  return folly::sformat(
    "{} {}",
    hackc,
    hackCompilerArgsDefault()
  );
#endif
}

static inline bool enableGcDefault() {
  return RuntimeOption::EvalEagerGC || one_bit_refcount;
}

static inline uint64_t pgoThresholdDefault() {
  return debug ? 2 : 2000;
}

static inline bool alignMacroFusionPairs() {
  switch (getProcessorFamily()) {
    case ProcessorFamily::Intel_SandyBridge:
    case ProcessorFamily::Intel_IvyBridge:
    case ProcessorFamily::Intel_Haswell:
    case ProcessorFamily::Intel_Broadwell:
    case ProcessorFamily::Intel_Skylake:
      return true;
    case ProcessorFamily::Unknown:
      return false;
  }
  return false;
}

static inline bool armLseDefault() {
#if defined (__linux__) && defined (__aarch64__) && defined (HWCAP_ATOMICS)
  return (getauxval(AT_HWCAP) & HWCAP_ATOMICS) != 0;
#else
  return false;
#endif
}

static inline bool evalJitDefault() {
#ifdef _MSC_VER
  return false;
#else
  return true;
#endif
}

static inline bool reuseTCDefault() {
  return hhvm_reuse_tc && !RuntimeOption::RepoAuthoritative;
}

static inline bool hugePagesSoundNice() {
  return RuntimeOption::ServerExecutionMode();
}

static inline uint32_t hotTextHugePagesDefault() {
  if (!hugePagesSoundNice()) return 0;
  return arch() == Arch::ARM ? 12 : 8;
}

static inline int nsjrDefault() {
  return RuntimeOption::ServerExecutionMode() ? 20 : 0;
}

static inline uint32_t profileRequestsDefault() {
  return debug ? std::numeric_limits<uint32_t>::max() : 2500;
}

static inline uint32_t profileBCSizeDefault() {
  return debug ? std::numeric_limits<uint32_t>::max()
    : RuntimeOption::EvalJitConcurrently ? 3750000
    : 4300000;
}

static inline uint32_t resetProfCountersDefault() {
  return RuntimeOption::EvalJitPGORacyProfiling
    ? std::numeric_limits<uint32_t>::max()
    : RuntimeOption::EvalJitConcurrently ? 250 : 1000;
}

static inline int retranslateAllRequestDefault() {
  return RuntimeOption::ServerExecutionMode() ? 1000000 : 0;
}

static inline int retranslateAllSecondsDefault() {
  return RuntimeOption::ServerExecutionMode() ? 180 : 0;
}

static inline bool layoutSplitHotColdDefault() {
  return arch() != Arch::ARM;
}

uint64_t ahotDefault() {
  return RuntimeOption::RepoAuthoritative ? 4 << 20 : 0;
}

std::string RuntimeOption::getTraceOutputFile() {
  return folly::sformat("{}/hphp.{}.log",
                        RuntimeOption::RemoteTraceOutputDir, (int64_t)getpid());
}

const uint64_t kEvalVMStackElmsDefault =
#if defined(VALGRIND) && !FOLLY_SANITIZE
 0x800
#else
 0x4000
#endif
 ;

constexpr uint32_t kEvalVMInitialGlobalTableSizeDefault = 512;
constexpr int kDefaultProfileInterpRequests = debug ? 1 : 11;
constexpr uint64_t kJitRelocationSizeDefault = 1 << 20;

static const bool kJitTimerDefault =
#ifdef ENABLE_JIT_TIMER_DEFAULT
  true
#else
  false
#endif
;

using std::string;
#define F(type, name, def) \
  type RuntimeOption::Eval ## name = type(def);
EVALFLAGS();
#undef F
std::set<string, stdltistr> RuntimeOption::DynamicInvokeFunctions;
hphp_string_imap<Cell> RuntimeOption::ConstantFunctions;

bool RuntimeOption::RecordCodeCoverage = false;
std::string RuntimeOption::CodeCoverageOutputFile;

std::string RuntimeOption::RepoLocalMode;
std::string RuntimeOption::RepoLocalPath;
std::string RuntimeOption::RepoCentralPath;
int32_t RuntimeOption::RepoCentralFileMode;
std::string RuntimeOption::RepoCentralFileUser;
std::string RuntimeOption::RepoCentralFileGroup;
bool RuntimeOption::RepoAllowFallbackPath = true;
std::string RuntimeOption::RepoEvalMode;
std::string RuntimeOption::RepoJournal = "delete";
bool RuntimeOption::RepoCommit = true;
bool RuntimeOption::RepoDebugInfo = true;
// Missing: RuntimeOption::RepoAuthoritative's physical location is
// perf-sensitive.
bool RuntimeOption::RepoPreload;
int64_t RuntimeOption::RepoLocalReadaheadRate = 0;
bool RuntimeOption::RepoLocalReadaheadConcurrent = false;

bool RuntimeOption::HHProfEnabled = false;
bool RuntimeOption::HHProfActive = false;
bool RuntimeOption::HHProfAccum = false;
bool RuntimeOption::HHProfRequest = false;

bool RuntimeOption::SandboxMode = false;
std::string RuntimeOption::SandboxPattern;
std::string RuntimeOption::SandboxHome;
std::string RuntimeOption::SandboxFallback;
std::string RuntimeOption::SandboxConfFile;
std::map<std::string, std::string> RuntimeOption::SandboxServerVariables;
bool RuntimeOption::SandboxFromCommonRoot = false;
std::string RuntimeOption::SandboxDirectoriesRoot;
std::string RuntimeOption::SandboxLogsRoot;

bool RuntimeOption::EnableHphpdDebugger = false;
bool RuntimeOption::EnableVSDebugger = false;
int RuntimeOption::VSDebuggerListenPort = -1;
bool RuntimeOption::VSDebuggerNoWait = false;
bool RuntimeOption::EnableDebuggerColor = true;
bool RuntimeOption::EnableDebuggerPrompt = true;
bool RuntimeOption::EnableDebuggerServer = false;
bool RuntimeOption::EnableDebuggerUsageLog = false;
bool RuntimeOption::DebuggerDisableIPv6 = false;
std::string RuntimeOption::DebuggerServerIP;
int RuntimeOption::DebuggerServerPort = 8089;
int RuntimeOption::DebuggerDefaultRpcPort = 8083;
std::string RuntimeOption::DebuggerDefaultRpcAuth;
std::string RuntimeOption::DebuggerRpcHostDomain;
int RuntimeOption::DebuggerDefaultRpcTimeout = 30;
std::string RuntimeOption::DebuggerDefaultSandboxPath;
std::string RuntimeOption::DebuggerStartupDocument;
int RuntimeOption::DebuggerSignalTimeout = 1;
std::string RuntimeOption::DebuggerAuthTokenScriptBin;

std::string RuntimeOption::SendmailPath = "sendmail -t -i";
std::string RuntimeOption::MailForceExtraParameters;

int64_t RuntimeOption::PregBacktraceLimit = 1000000;
int64_t RuntimeOption::PregRecursionLimit = 100000;
bool RuntimeOption::EnablePregErrorLog = true;

bool RuntimeOption::SimpleXMLEmptyNamespaceMatchesAll = false;

bool RuntimeOption::AllowDuplicateCookies = true;

bool RuntimeOption::EnableHotProfiler = true;
int RuntimeOption::ProfilerTraceBuffer = 2000000;
double RuntimeOption::ProfilerTraceExpansion = 1.2;
int RuntimeOption::ProfilerMaxTraceBuffer = 0;

#ifdef FACEBOOK
bool RuntimeOption::EnableFb303Server = false;
int RuntimeOption::Fb303ServerPort = 0;
int RuntimeOption::Fb303ServerThreadStackSizeMb = 8;
int RuntimeOption::Fb303ServerWorkerThreads = 1;
int RuntimeOption::Fb303ServerPoolThreads = 1;
#endif

double RuntimeOption::XenonPeriodSeconds = 0.0;
uint32_t RuntimeOption::XenonRequestFreq = 1;
bool RuntimeOption::XenonForceAlwaysOn = false;
bool RuntimeOption::TrackPerUnitMemory = false;

std::map<std::string, std::string> RuntimeOption::CustomSettings;

#ifdef NDEBUG
  #ifdef ALWAYS_ASSERT
    const StaticString s_hhvm_build_type("Release with asserts");
  #else
    const StaticString s_hhvm_build_type("Release");
  #endif
#else
  const StaticString s_hhvm_build_type("Debug");
#endif

///////////////////////////////////////////////////////////////////////////////

static void setResourceLimit(int resource, const IniSetting::Map& ini,
                             const Hdf& rlimit, const char* nodeName) {
  if (!Config::GetString(ini, rlimit, nodeName).empty()) {
    struct rlimit rl;
    getrlimit(resource, &rl);
    rl.rlim_cur = Config::GetInt64(ini, rlimit, nodeName);
    if (rl.rlim_max < rl.rlim_cur) {
      rl.rlim_max = rl.rlim_cur;
    }
    int ret = setrlimit(resource, &rl);
    if (ret) {
      Logger::Error("Unable to set %s to %" PRId64 ": %s (%d)",
                    nodeName, (int64_t)rl.rlim_cur,
                    folly::errnoStr(errno).c_str(), errno);
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

static bool matchHdfPattern(const std::string &value,
                            const IniSetting::Map& ini, Hdf hdfPattern,
                            const std::string& name,
                            const std::string& suffix = "") {
  string pattern = Config::GetString(ini, hdfPattern, name, "", false);
  if (!pattern.empty()) {
    if (!suffix.empty()) pattern += suffix;
    Variant ret = preg_match(String(pattern.c_str(), pattern.size(),
                                    CopyString),
                             String(value.c_str(), value.size(),
                                    CopyString));
    if (ret.toInt64() <= 0) {
      return false;
    }
  }
  return true;
}

// A machine can belong to a tier, which can overwrite
// various settings, even if they are set in the same
// hdf file. However, CLI overrides still win the day over
// everything.
static std::vector<std::string> getTierOverwrites(IniSetting::Map& ini,
                                                  Hdf& config) {

  // Machine metrics
  string hostname, tier, cpu, tiers;
  {
    hostname = Config::GetString(ini, config, "Machine.name");
    if (hostname.empty()) {
      hostname = Process::GetHostName();
    }

    tier = Config::GetString(ini, config, "Machine.tier");

    cpu = Config::GetString(ini, config, "Machine.cpu");
    if (cpu.empty()) {
      cpu = Process::GetCPUModel();
    }

    tiers = Config::GetString(ini, config, "Machine.tiers");
    if (!tiers.empty()) {
      if (!folly::readFile(tiers.c_str(), tiers)) {
        tiers.clear();
      }
    }
  }

  std::vector<std::string> messages;
  // Tier overwrites
  {
    for (Hdf hdf = config["Tiers"].firstChild(); hdf.exists();
         hdf = hdf.next()) {
      if (messages.empty()) {
        messages.emplace_back(folly::sformat(
                                "Matching tiers using: "
                                "machine='{}', tier='{}', "
                                "cpu = '{}', tiers = '{}'",
                                hostname, tier, cpu, tiers));
      }
      // Check the patterns using "&" rather than "&&" so they all get
      // evaluated; otherwise with multiple patterns, if an earlier
      // one fails to match, the later one is reported as unused.
      if (matchHdfPattern(hostname, ini, hdf, "machine") &
          matchHdfPattern(tier, ini, hdf, "tier") &
          matchHdfPattern(tiers, ini, hdf, "tiers", "m") &
          matchHdfPattern(cpu, ini, hdf, "cpu")) {
        messages.emplace_back(folly::sformat(
                                "Matched tier: {}", hdf.getName()));
        if (hdf.exists("clear")) {
          std::vector<std::string> list;
          hdf["clear"].configGet(list);
          for (auto const& s : list) {
            config.remove(s);
          }
        }
        config.copy(hdf["overwrite"]);
        // no break here, so we can continue to match more overwrites
      }
      hdf["overwrite"].setVisited(); // avoid lint complaining
      if (hdf.exists("clear")) {
        // when the tier does not match, "clear" is not accessed
        // mark it visited, so the linter does not complain
        hdf["clear"].setVisited();
      }
    }
  }
  return messages;
}

void RuntimeOption::ReadSatelliteInfo(
    const IniSettingMap& ini,
    const Hdf& hdf,
    std::vector<std::shared_ptr<SatelliteServerInfo>>& infos,
    std::string& xboxPassword,
    std::set<std::string>& xboxPasswords) {
  auto ss_callback = [&] (const IniSettingMap &ini_ss, const Hdf &hdf_ss,
                         const std::string &ini_ss_key) {
    auto satellite = std::make_shared<SatelliteServerInfo>(ini_ss, hdf_ss,
                                                           ini_ss_key);
    infos.push_back(satellite);
    if (satellite->getType() == SatelliteServer::Type::KindOfRPCServer) {
      xboxPassword = satellite->getPassword();
      xboxPasswords = satellite->getPasswords();
    }
  };
  Config::Iterate(ss_callback, ini, hdf, "Satellites");
}

extern void initialize_apc();
void RuntimeOption::Load(
  IniSetting::Map& ini, Hdf& config,
  const std::vector<std::string>& iniClis /* = std::vector<std::string>() */,
  const std::vector<std::string>& hdfClis /* = std::vector<std::string>() */,
  std::vector<std::string>* messages /* = nullptr */,
  std::string cmd /* = "" */) {

  // Intialize the memory manager here because various settings and
  // initializations that we do here need it
  tl_heap.getCheck();

  // Get the ini (-d) and hdf (-v) strings, which may override some
  // of options that were set from config files. We also do these
  // now so we can override Tier.*.[machine | tier | cpu] on the
  // command line, along with any fields within a Tier (e.g.,
  // CoreFileSize)
  for (auto& istr : iniClis) {
    Config::ParseIniString(istr, ini);
  }
  for (auto& hstr : hdfClis) {
    Config::ParseHdfString(hstr, config);
  }

  // See if there are any Tier-based overrides
  auto m = getTierOverwrites(ini, config);
  if (messages) *messages = std::move(m);

  // RelativeConfigs can be set by commandline flags and tier overwrites, they
  // may also contain tier overwrites. They are, however, only included once, so
  // relative configs may not specify other relative configs which must to be
  // loaded. If RelativeConfigs is modified while loading configs an error is
  // raised, but we defer doing so until the logger is initialized below. If a
  // relative config cannot be found it is silently skipped (this is to allow
  // configs to be conditionally applied to scripts based on their location). By
  // reading the "hhvm.relative_configs" ini setting at runtime it is possible
  // to determine which configs were actually loaded.
  std::string relConfigsError;
  Config::Bind(s_RelativeConfigs, ini, config, "RelativeConfigs");
  if (!cmd.empty() && !s_RelativeConfigs.empty()) {
    String strcmd(cmd, CopyString);
    Process::InitProcessStatics();
    auto const currentDir = Process::CurrentWorkingDirectory.data();
    std::vector<std::string> newConfigs;
    auto const original = s_RelativeConfigs;
    for (auto& str : original) {
      if (str.empty()) continue;

      std::string fullpath;
      auto const found = FileUtil::runRelative(
        str, strcmd, currentDir,
        [&] (const String& f) {
          if (access(f.data(), R_OK) == 0) {
            fullpath = f.toCppString();
            Config::ParseConfigFile(fullpath, ini, config);
            return true;
          }
          return false;
        }
      );
      if (found) newConfigs.emplace_back(std::move(fullpath));
    }
    if (!newConfigs.empty()) {
      auto m2 = getTierOverwrites(ini, config);
      if (messages) *messages = std::move(m2);
      if (s_RelativeConfigs != original) {
        relConfigsError = folly::sformat(
          "RelativeConfigs node was modified while loading configs from [{}] "
          "to [{}]",
          folly::join(", ", original),
          folly::join(", ", s_RelativeConfigs)
        );
      }
    }
    s_RelativeConfigs.swap(newConfigs);
  } else {
    s_RelativeConfigs.clear();
  }

  // Then get the ini and hdf cli strings again, in case the tier overwrites
  // overrode any non-tier based command line option we set. The tier-based
  // command line overwrites will already have been set in the call above.
  // This extra call is for the other command line options that may have been
  // overridden by a tier, but shouldn't have been.
  for (auto& istr : iniClis) {
    Config::ParseIniString(istr, ini);
  }
  for (auto& hstr : hdfClis) {
    Config::ParseHdfString(hstr, config);
  }

  Config::Bind(PidFile, ini, config, "PidFile", "www.pid");
  Config::Bind(DeploymentId, ini, config, "DeploymentId");

  Config::Bind(DynamicInvokeFunctions, ini, config, "DynamicInvokeFunctions",
               DynamicInvokeFunctions);

  {
    // Logging
    auto setLogLevel = [](const std::string& value) {
      // ini parsing treats "None" as ""
      if (value == "None" || value == "") {
        Logger::LogLevel = Logger::LogNone;
      } else if (value == "Error") {
        Logger::LogLevel = Logger::LogError;
      } else if (value == "Warning") {
        Logger::LogLevel = Logger::LogWarning;
      } else if (value == "Info") {
        Logger::LogLevel = Logger::LogInfo;
      } else if (value == "Verbose") {
        Logger::LogLevel = Logger::LogVerbose;
      } else {
        return false;
      }
      return true;
    };
    auto str = Config::GetString(ini, config, "Log.Level");
    if (!str.empty()) {
      setLogLevel(str);
    }
    IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                     "hhvm.log.level", IniSetting::SetAndGet<std::string>(
      setLogLevel,
      []() {
        switch (Logger::LogLevel) {
          case Logger::LogNone:
            return "None";
          case Logger::LogError:
            return "Error";
          case Logger::LogWarning:
            return "Warning";
          case Logger::LogInfo:
            return "Info";
          case Logger::LogVerbose:
            return "Verbose";
        }
        return "";
      }
    ));

    Config::Bind(Logger::UseLogFile, ini, config, "Log.UseLogFile", true);
    Config::Bind(LogFile, ini, config, "Log.File");
    Config::Bind(LogFileSymLink, ini, config, "Log.SymLink");
    Config::Bind(LogFilePeriodMultiplier, ini,
                 config, "Log.PeriodMultiplier", 0);
    if (Logger::UseLogFile && RuntimeOption::ServerExecutionMode()) {
      RuntimeOption::ErrorLogs[Logger::DEFAULT] =
        ErrorLogFileData(LogFile, LogFileSymLink, LogFilePeriodMultiplier);
    }
    if (Config::GetBool(ini, config, "Log.AlwaysPrintStackTraces")) {
      Logger::SetTheLogger(Logger::DEFAULT, new ExtendedLogger());
      ExtendedLogger::EnabledByDefault = true;
    }

    Config::Bind(Logger::LogHeader, ini, config, "Log.Header");
    Config::Bind(Logger::LogNativeStackTrace, ini, config,
                 "Log.NativeStackTrace", true);
    Config::Bind(Logger::UseSyslog, ini, config, "Log.UseSyslog", false);
    Config::Bind(Logger::UseRequestLog, ini, config, "Log.UseRequestLog",
                 false);
    Config::Bind(Logger::AlwaysEscapeLog, ini, config, "Log.AlwaysEscapeLog",
                 true);
    Config::Bind(Logger::UseCronolog, ini, config, "Log.UseCronolog", false);
    Config::Bind(Logger::MaxMessagesPerRequest, ini,
                 config, "Log.MaxMessagesPerRequest", -1);
    Config::Bind(LogFileFlusher::DropCacheChunkSize, ini,
                 config, "Log.DropCacheChunkSize", 1 << 20);
    Config::Bind(RuntimeOption::LogHeaderMangle, ini, config,
                 "Log.HeaderMangle", 0);
    Config::Bind(AlwaysLogUnhandledExceptions, ini,
                 config, "Log.AlwaysLogUnhandledExceptions",
                 RuntimeOption::EnableHipHopSyntax);
    Config::Bind(NoSilencer, ini, config, "Log.NoSilencer");
    Config::Bind(RuntimeErrorReportingLevel, ini,
                 config, "Log.RuntimeErrorReportingLevel",
                 static_cast<int>(ErrorMode::HPHP_ALL));
    Config::Bind(ForceErrorReportingLevel, ini,
                 config, "Log.ForceErrorReportingLevel", 0);
    Config::Bind(AccessLogDefaultFormat, ini, config,
                 "Log.AccessLogDefaultFormat", "%h %l %u %t \"%r\" %>s %b");

    auto parseLogs = [] (const Hdf &config, const IniSetting::Map& ini,
                         const std::string &name,
                         std::map<std::string, AccessLogFileData> &logs) {
      auto parse_logs_callback = [&] (const IniSetting::Map &ini_pl,
                                      const Hdf &hdf_pl,
                                      const std::string &ini_pl_key) {
        string logName = hdf_pl.exists() && !hdf_pl.isEmpty()
                       ? hdf_pl.getName()
                       : ini_pl_key;
        string fname = Config::GetString(ini_pl, hdf_pl, "File", "", false);
        if (!fname.empty()) {
          string symlink = Config::GetString(ini_pl, hdf_pl, "SymLink", "",
                                             false);
          string format = Config::GetString(ini_pl, hdf_pl, "Format",
                                            AccessLogDefaultFormat, false);
          auto periodMultiplier = Config::GetUInt16(ini_pl, hdf_pl,
                                                    "PeriodMultiplier",
                                                    0, false);
          logs[logName] = AccessLogFileData(fname, symlink,
                                            format, periodMultiplier);


        }
      };
      Config::Iterate(parse_logs_callback, ini, config, name);
    };

    parseLogs(config, ini, "Log.Access", AccessLogs);
    RPCLogs = AccessLogs;
    parseLogs(config, ini, "Log.RPC", RPCLogs);

    Config::Bind(AdminLogFormat, ini, config, "Log.AdminLog.Format",
                 "%h %t %s %U");
    Config::Bind(AdminLogFile, ini, config, "Log.AdminLog.File");
    Config::Bind(AdminLogSymLink, ini, config, "Log.AdminLog.SymLink");
  }
  {
    // Error Handling

    Config::Bind(ErrorUpgradeLevel, ini, config, "ErrorHandling.UpgradeLevel",
                 0);
    Config::Bind(MaxSerializedStringSize, ini,
                 config, "ErrorHandling.MaxSerializedStringSize",
                 64 * 1024 * 1024);
    Config::Bind(CallUserHandlerOnFatals, ini,
                 config, "ErrorHandling.CallUserHandlerOnFatals", false);
    Config::Bind(ThrowExceptionOnBadMethodCall, ini,
                 config, "ErrorHandling.ThrowExceptionOnBadMethodCall", true);
    Config::Bind(LogNativeStackOnOOM, ini,
                 config, "ErrorHandling.LogNativeStackOnOOM", false);
    Config::Bind(NoInfiniteRecursionDetection, ini,
                 config, "ErrorHandling.NoInfiniteRecursionDetection");
    Config::Bind(NoticeFrequency, ini, config, "ErrorHandling.NoticeFrequency",
                 1);
    Config::Bind(WarningFrequency, ini, config,
                 "ErrorHandling.WarningFrequency", 1);
  }

  // If we generated errors while loading RelativeConfigs report those now that
  // error reporting is initialized
  if (!relConfigsError.empty()) Logger::Error(relConfigsError);

  {
    if (Config::GetInt64(ini, config, "ResourceLimit.CoreFileSizeOverride")) {
      setResourceLimit(RLIMIT_CORE, ini,  config,
                       "ResourceLimit.CoreFileSizeOverride");
    } else {
      setResourceLimit(RLIMIT_CORE, ini, config, "ResourceLimit.CoreFileSize");
    }
    setResourceLimit(RLIMIT_NOFILE, ini, config, "ResourceLimit.MaxSocket");
    setResourceLimit(RLIMIT_DATA, ini, config, "ResourceLimit.RSS");
    // These don't have RuntimeOption::xxx bindings, but we still want to be
    // able to use ini_xxx functionality on them; so directly bind to a local
    // static via Config::Bind.
    static int64_t s_core_file_size_override, s_core_file_size, s_rss = 0;
    static int32_t s_max_socket = 0;
    Config::Bind(s_core_file_size_override, ini, config,
                 "ResourceLimit.CoreFileSizeOverride", 0);
    Config::Bind(s_core_file_size, ini, config, "ResourceLimit.CoreFileSize",
                 0);
    Config::Bind(s_max_socket, ini, config, "ResourceLimit.MaxSocket", 0);
    Config::Bind(s_rss, ini, config, "ResourceLimit.RSS", 0);

    Config::Bind(SocketDefaultTimeout, ini, config,
                 "ResourceLimit.SocketDefaultTimeout", 60);
    Config::Bind(MaxSQLRowCount, ini, config, "ResourceLimit.MaxSQLRowCount",
                 0);
    Config::Bind(SerializationSizeLimit, ini, config,
                 "ResourceLimit.SerializationSizeLimit", StringData::MaxSize);
    Config::Bind(HeapSizeMB, ini, config, "ResourceLimit.HeapSizeMB",
                 HeapSizeMB);
    Config::Bind(HeapResetCountBase, ini, config,
                 "ResourceLimit.HeapResetCountBase", HeapResetCountBase);
    Config::Bind(HeapResetCountMultiple, ini, config,
                 "ResourceLimit.HeapResetCountMultiple",
                 HeapResetCountMultiple);
    Config::Bind(HeapLowWaterMark , ini, config,
                 "ResourceLimit.HeapLowWaterMark", HeapLowWaterMark);
    Config::Bind(HeapHighWaterMark , ini, config,
                 "ResourceLimit.HeapHighWaterMark",HeapHighWaterMark);
  }
  {
    Config::Bind(DisableCompact, ini, config,
                 "Hack.Lang.Phpism.DisableCompact", DisableCompact);
    Config::Bind(DisableExtract, ini, config,
                 "Hack.Lang.Phpism.DisableExtract", DisableExtract);
    Config::Bind(DisableForwardStaticCall, ini, config,
                 "Hack.Lang.Phpism.DisableForwardStaticCall",
                 DisableForwardStaticCall);
    Config::Bind(DisableForwardStaticCallArray, ini, config,
                 "Hack.Lang.Phpism.DisableForwardStaticCallArray",
                 DisableForwardStaticCallArray);
  }
  {
    // Repo
    // Local Repo
    Config::Bind(RepoLocalMode, ini, config, "Repo.Local.Mode", RepoLocalMode);
    if (RepoLocalMode.empty()) {
      const char* HHVM_REPO_LOCAL_MODE = getenv("HHVM_REPO_LOCAL_MODE");
      if (HHVM_REPO_LOCAL_MODE != nullptr) {
        RepoLocalMode = HHVM_REPO_LOCAL_MODE;
      }
      RepoLocalMode = "r-";
    }
    if (RepoLocalMode.compare("rw")
        && RepoLocalMode.compare("r-")
        && RepoLocalMode.compare("--")) {
      Logger::Error("Bad config setting: Repo.Local.Mode=%s",
                    RepoLocalMode.c_str());
      RepoLocalMode = "rw";
    }
    // Repo.Local.Path
    Config::Bind(RepoLocalPath, ini, config, "Repo.Local.Path");
    if (RepoLocalPath.empty()) {
      const char* HHVM_REPO_LOCAL_PATH = getenv("HHVM_REPO_LOCAL_PATH");
      if (HHVM_REPO_LOCAL_PATH != nullptr) {
        RepoLocalPath = HHVM_REPO_LOCAL_PATH;
      }
    }

    // Central Repo
    Config::Bind(RepoCentralPath, ini, config, "Repo.Central.Path");
    Config::Bind(RepoCentralFileMode, ini, config, "Repo.Central.FileMode");
    Config::Bind(RepoCentralFileUser, ini, config, "Repo.Central.FileUser");
    Config::Bind(RepoCentralFileGroup, ini, config, "Repo.Central.FileGroup");

    Config::Bind(RepoAllowFallbackPath, ini, config, "Repo.AllowFallbackPath",
                 RepoAllowFallbackPath);

    // Repo - Eval
    Config::Bind(RepoEvalMode, ini, config, "Repo.Eval.Mode");
    if (RepoEvalMode.empty()) {
      RepoEvalMode = "readonly";
    } else if (RepoEvalMode.compare("local")
               && RepoEvalMode.compare("central")
               && RepoEvalMode.compare("readonly")) {
      Logger::Error("Bad config setting: Repo.Eval.Mode=%s",
                    RepoEvalMode.c_str());
      RepoEvalMode = "readonly";
    }

    Config::Bind(RepoJournal, ini, config, "Repo.Journal", RepoJournal);
    Config::Bind(RepoCommit, ini, config, "Repo.Commit",
                 RepoCommit);
    Config::Bind(RepoDebugInfo, ini, config, "Repo.DebugInfo", RepoDebugInfo);
    Config::Bind(RepoAuthoritative, ini, config, "Repo.Authoritative",
                 RepoAuthoritative);
    Config::Bind(RepoPreload, ini, config, "Repo.Preload", false);
    Config::Bind(RepoLocalReadaheadRate, ini, config,
                 "Repo.LocalReadaheadRate", 0);
    Config::Bind(RepoLocalReadaheadConcurrent, ini, config,
                 "Repo.LocalReadaheadConcurrent", false);
  }

  if (use_jemalloc) {
    // HHProf
    Config::Bind(HHProfEnabled, ini, config, "HHProf.Enabled", false);
    Config::Bind(HHProfActive, ini, config, "HHProf.Active", false);
    Config::Bind(HHProfAccum, ini, config, "HHProf.Accum", false);
    Config::Bind(HHProfRequest, ini, config, "HHProf.Request", false);
  }
  {
    // Eval
    Config::Bind(EnableHipHopSyntax, ini, config, "Eval.EnableHipHopSyntax",
                 EnableHipHopSyntax);
    Config::Bind(EnableHipHopExperimentalSyntax, ini,
                 config, "Eval.EnableHipHopExperimentalSyntax");
    Config::Bind(EnableShortTags, ini, config, "Eval.EnableShortTags", true);
    Config::Bind(EnableAspTags, ini, config, "Eval.EnableAspTags");
    Config::Bind(EnableXHP, ini, config, "Eval.EnableXHP", EnableXHP);
    Config::Bind(EnableZendSorting, ini, config, "Eval.EnableZendSorting",
                 false);
    Config::Bind(TimeoutsUseWallTime, ini, config, "Eval.TimeoutsUseWallTime",
                 true);
    Config::Bind(CheckFlushOnUserClose, ini, config,
                 "Eval.CheckFlushOnUserClose", true);
    Config::Bind(EvalInitialNamedEntityTableSize, ini, config,
                 "Eval.InitialNamedEntityTableSize",
                 EvalInitialNamedEntityTableSize);
    Config::Bind(EvalInitialStaticStringTableSize, ini, config,
                 "Eval.InitialStaticStringTableSize",
                 EvalInitialStaticStringTableSize);
    Config::Bind(CheckParamTypeInvariance, ini, config,
                 "Eval.CheckParamTypeInvariance",
                 !EnableHipHopSyntax);

    static std::string jitSerdesMode;
    Config::Bind(jitSerdesMode, ini, config, "Eval.JitSerdesMode", "Off");

    EvalJitSerdesMode = [&] {
      #define X(x) if (jitSerdesMode == #x) return JitSerdesMode::x
      X(Serialize);
      X(SerializeAndExit);
      X(Deserialize);
      X(DeserializeOrFail);
      X(DeserializeOrGenerate);
      X(DeserializeAndExit);
      #undef X
      return JitSerdesMode::Off;
    }();
    Config::Bind(EvalJitSerdesFile, ini, config,
                 "Eval.JitSerdesFile", EvalJitSerdesFile);
    // DumpPreciseProfileData defaults to true only when we can possibly write
    // profile data to disk.  It can be set to false to avoid the performance
    // penalty of only running the interpreter during retranslateAll.  We will
    // assume that DumpPreciseProfileData implies (JitSerdesMode::Serialize ||
    // JitSerdesMode::SerializeAndExit), to avoid checking too many flags in a
    // few places.  The config file should never need to explicitly set
    // DumpPreciseProfileData to true.
    auto const couldDump = !EvalJitSerdesFile.empty() &&
      ((EvalJitSerdesMode == JitSerdesMode::Serialize) ||
       (EvalJitSerdesMode == JitSerdesMode::SerializeAndExit) ||
       (EvalJitSerdesMode == JitSerdesMode::DeserializeOrGenerate));
    Config::Bind(DumpPreciseProfileData, ini, config,
                 "Eval.DumpPreciseProfileData", couldDump);

    if (EnableHipHopSyntax) {
      // If EnableHipHopSyntax is true, it forces EnableXHP to true
      // regardless of how it was set in the config
      EnableXHP = true;
    }

    Config::Bind(EnableObjDestructCall, ini, config,
                 "Eval.EnableObjDestructCall", true);
    Config::Bind(CheckSymLink, ini, config, "Eval.CheckSymLink", true);

#define F(type, name, defaultVal) \
    Config::Bind(Eval ## name, ini, config, "Eval."#name, defaultVal);
    EVALFLAGS()
#undef F

    EvalHackCompilerExtractPath = insertSchema(
      EvalHackCompilerExtractPath.data()
    );

    EvalHackCompilerFallbackPath = insertSchema(
      EvalHackCompilerFallbackPath.data()
    );

    EvalEmbeddedDataExtractPath = insertSchema(
      EvalEmbeddedDataExtractPath.data()
    );

    EvalEmbeddedDataFallbackPath = insertSchema(
      EvalEmbeddedDataFallbackPath.data()
    );

    if (EvalPerfRelocate > 0) {
      setRelocateRequests(EvalPerfRelocate);
    }
    if (!jit::mcgen::retranslateAllEnabled()) {
      EvalJitWorkerThreads = 0;
      if (EvalJitSerdesMode != JitSerdesMode::Off) {
        if (ServerMode) {
          Logger::Warning("Eval.JitSerdesMode reset from " + jitSerdesMode +
                          " to off, becasue JitRetranslateAll isn't enabled.");
        }
        EvalJitSerdesMode = JitSerdesMode::Off;
      }
      EvalJitSerdesFile.clear();
      DumpPreciseProfileData = false;
    }
    low_malloc_huge_pages(EvalMaxLowMemHugePages);
    HardwareCounter::Init(EvalProfileHWEnable,
                          url_decode(EvalProfileHWEvents.data(),
                                     EvalProfileHWEvents.size()).toCppString(),
                          false,
                          EvalProfileHWExcludeKernel,
                          EvalProfileHWFastReads,
                          EvalProfileHWExportInterval);

    Config::Bind(EnableIntrinsicsExtension, ini,
                 config, "Eval.EnableIntrinsicsExtension",
                 EnableIntrinsicsExtension);
    Config::Bind(RecordCodeCoverage, ini, config, "Eval.RecordCodeCoverage");
    if (EvalJit && RecordCodeCoverage) {
      throw std::runtime_error("Code coverage is not supported with "
        "Eval.Jit=true");
    }
    Config::Bind(DisableSmallAllocator, ini, config,
                 "Eval.DisableSmallAllocator", DisableSmallAllocator);
    SetArenaSlabAllocBypass(DisableSmallAllocator);

    Config::Bind(PerAllocSampleF, ini, config, "Eval.PerAllocSampleF",
                 PerAllocSampleF);
    Config::Bind(TotalAllocSampleF, ini, config, "Eval.TotalAllocSampleF",
                 TotalAllocSampleF);

    if (RecordCodeCoverage) CheckSymLink = true;
    Config::Bind(CodeCoverageOutputFile, ini, config,
                 "Eval.CodeCoverageOutputFile");
    // NB: after we know the value of RepoAuthoritative.
    Config::Bind(EnableArgsInBacktraces, ini, config,
                 "Eval.EnableArgsInBacktraces", !RepoAuthoritative);
    Config::Bind(EnableContextInErrorHandler, ini, config,
                 "Eval.EnableContextInErrorHandler", !RepoAuthoritative);
    Config::Bind(EvalAuthoritativeMode, ini, config, "Eval.AuthoritativeMode",
                 false);
    if (RepoAuthoritative) {
      EvalAuthoritativeMode = true;
    }
    {
      // Debugger (part of Eval)
      Config::Bind(EnableHphpdDebugger, ini, config,
                   "Eval.Debugger.EnableDebugger");
      Config::Bind(EnableDebuggerColor, ini, config,
                   "Eval.Debugger.EnableDebuggerColor", true);
      Config::Bind(EnableDebuggerPrompt, ini, config,
                   "Eval.Debugger.EnableDebuggerPrompt", true);
      Config::Bind(EnableDebuggerServer, ini, config,
                   "Eval.Debugger.EnableDebuggerServer");
      Config::Bind(EnableDebuggerUsageLog, ini, config,
                   "Eval.Debugger.EnableDebuggerUsageLog");
      Config::Bind(DebuggerServerIP, ini, config, "Eval.Debugger.IP");
      Config::Bind(DebuggerServerPort, ini, config, "Eval.Debugger.Port", 8089);
      Config::Bind(DebuggerDisableIPv6, ini, config,
                   "Eval.Debugger.DisableIPv6", false);
      Config::Bind(DebuggerDefaultSandboxPath, ini, config,
                   "Eval.Debugger.DefaultSandboxPath");
      Config::Bind(DebuggerStartupDocument, ini, config,
                   "Eval.Debugger.StartupDocument");
      Config::Bind(DebuggerSignalTimeout, ini, config,
                   "Eval.Debugger.SignalTimeout", 1);
      Config::Bind(DebuggerDefaultRpcPort, ini, config,
                   "Eval.Debugger.RPC.DefaultPort", 8083);
      Config::Bind(DebuggerDefaultRpcAuth, ini, config,
                   "Eval.Debugger.RPC.DefaultAuth");
      Config::Bind(DebuggerRpcHostDomain, ini, config,
                   "Eval.Debugger.RPC.HostDomain");
      Config::Bind(DebuggerDefaultRpcTimeout, ini, config,
                   "Eval.Debugger.RPC.DefaultTimeout", 30);
      Config::Bind(DebuggerAuthTokenScriptBin, ini, config,
                   "Eval.Debugger.Auth.TokenScriptBin");
    }
  }
  {
    // CodeCache
    using jit::CodeCache;
    Config::Bind(CodeCache::AHotSize, ini, config, "Eval.JitAHotSize",
                 ahotDefault());
    Config::Bind(CodeCache::ASize, ini, config, "Eval.JitASize", 60 << 20);

    if (RuntimeOption::EvalJitPGO) {
      Config::Bind(CodeCache::AProfSize, ini, config, "Eval.JitAProfSize",
                   64 << 20);
    } else {
      // Avoid "Possible bad confg node" warning for unused keys.
      config["Eval.JitAProfSize"].configGetUInt64();
      CodeCache::AProfSize = 0;
    }
    Config::Bind(CodeCache::AColdSize, ini, config, "Eval.JitAColdSize",
                 24 << 20);
    Config::Bind(CodeCache::AFrozenSize, ini, config, "Eval.JitAFrozenSize",
                 40 << 20);
    Config::Bind(CodeCache::GlobalDataSize, ini, config,
                 "Eval.JitGlobalDataSize", CodeCache::ASize >> 2);

    Config::Bind(CodeCache::AMaxUsage, ini, config,
                 "Eval.JitAMaxUsage", CodeCache::ASize);
    Config::Bind(CodeCache::AColdMaxUsage, ini, config,
                 "Eval.JitAColdMaxUsage", CodeCache::AColdSize);
    Config::Bind(CodeCache::AFrozenMaxUsage, ini, config,
                 "Eval.JitAFrozenMaxUsage", CodeCache::AFrozenSize);

    Config::Bind(CodeCache::MapTCHuge, ini, config, "Eval.MapTCHuge",
                 hugePagesSoundNice());

    Config::Bind(CodeCache::TCNumHugeHotMB, ini, config,
                 "Eval.TCNumHugeHotMB", 16);
    Config::Bind(CodeCache::TCNumHugeColdMB, ini, config,
                 "Eval.TCNumHugeColdMB", 4);

    Config::Bind(CodeCache::AutoTCShift, ini, config, "Eval.JitAutoTCShift", 1);
  }
  {
    // Hack Language
    Config::Bind(IntsOverflowToInts, ini, config,
                 "Hack.Lang.IntsOverflowToInts", EnableHipHopSyntax);
    auto const def = RuntimeOption::EnableHipHopSyntax ?
      HackStrictOption::ON : HackStrictOption::OFF;

    Config::Bind(StrictArrayFillKeys, ini, config,
                 "Hack.Lang.StrictArrayFillKeys", def);
    Config::Bind(DisallowDynamicVarEnvFuncs, ini, config,
                 "Hack.Lang.DisallowDynamicVarEnvFuncs", def);
    Config::Bind(IconvIgnoreCorrect, ini, config,
                 "Hack.Lang.IconvIgnoreCorrect", def);
    Config::Bind(MinMaxAllowDegenerate, ini, config,
                 "Hack.Lang.MinMaxAllowDegenerate", def);

    Config::Bind(LookForTypechecker, ini, config,
                 "Hack.Lang.LookForTypechecker", false);

    // If you turn off LookForTypechecker, you probably want to turn this off
    // too -- basically, make the two look like the same option to external
    // users, unless you really explicitly want to set them differently for
    // some reason.
    Config::Bind(AutoTypecheck, ini, config, "Hack.Lang.AutoTypecheck",
                 LookForTypechecker);

    // The default behavior in PHP is to auto-prime generators. For now we leave
    // this disabled in HipHop syntax mode to deal with incompatibilities in
    // existing code-bases.
    Config::Bind(AutoprimeGenerators, ini, config,
                 "Hack.Lang.AutoprimeGenerators",
                 true);
    Config::Bind(EnableIsExprPrimitiveMigration, ini, config,
                 "Hack.Lang.EnableIsExprPrimitiveMigration",
                 true);
    Config::Bind(EnableReifiedGenerics, ini, config,
                 "Hack.Lang.EnableReifiedGenerics",
                 false);
    Config::Bind(Hacksperimental, ini, config,
                "Hack.Lang.Hacksperimental",
                false);
    Config::Bind(EnableCoroutines, ini, config,
                "Hack.Lang.EnableCoroutines",
                true);
  }
  {
    // Options for PHP7 features which break BC. (Features which do not break
    // BC don't need options here and can just always be turned on.)
    //
    // NB that the "PHP7.all" option is intended to be only a master switch;
    // all runtime behavior gating should be based on sub-options (that's why
    // it's a file static not a static member of RuntimeOption). Also don't
    // forget to update mangleUnitPHP7Options if needed.
    //
    // TODO: we may eventually want to make an option which specifies
    // directories or filenames to exclude from PHP7 behavior, and so checking
    // these may want to be per-file. We originally planned to do this from the
    // get-go, but threading that through turns out to be kind of annoying and
    // of questionable value, so just doing this for now.
    Config::Bind(s_PHP7_master, ini, config, "PHP7.all", s_PHP7_default);
    Config::Bind(PHP7_DeprecationWarnings, ini, config,
                 "PHP7.DeprecationWarnings", s_PHP7_master);
    Config::Bind(PHP7_EngineExceptions, ini, config, "PHP7.EngineExceptions",
                 s_PHP7_master);
    Config::Bind(PHP7_IntSemantics, ini, config, "PHP7.IntSemantics",
                 s_PHP7_master);
    Config::Bind(PHP7_LTR_assign, ini, config, "PHP7.LTRAssign",
                 s_PHP7_master);
    Config::Bind(PHP7_NoHexNumerics, ini, config, "PHP7.NoHexNumerics",
                 s_PHP7_master);
    Config::Bind(PHP7_Builtins, ini, config, "PHP7.Builtins", s_PHP7_master);
    Config::Bind(PHP7_ScalarTypes, ini, config, "PHP7.ScalarTypes",
                 s_PHP7_master);
    Config::Bind(PHP7_Substr, ini, config, "PHP7.Substr",
                 s_PHP7_master);
    Config::Bind(PHP7_UVS, ini, config, "PHP7.UVS", s_PHP7_master);
    Config::Bind(PHP7_DisallowUnsafeCurlUploads, ini, config,
                 "PHP7.DisallowUnsafeCurlUploads", s_PHP7_master);
  }
  {
    // Server
    Config::Bind(Host, ini, config, "Server.Host");
    Config::Bind(DefaultServerNameSuffix, ini, config,
                 "Server.DefaultServerNameSuffix");
    Config::Bind(AlwaysDecodePostDataDefault, ini, config,
                 "Server.AlwaysDecodePostDataDefault",
                 AlwaysDecodePostDataDefault);
    Config::Bind(ServerType, ini, config, "Server.Type", ServerType);
    Config::Bind(ServerIP, ini, config, "Server.IP");
    Config::Bind(ServerFileSocket, ini, config, "Server.FileSocket");

#ifdef FACEBOOK
    //Do not cause slowness on startup -- except for Facebook
    if (GetServerPrimaryIPv4().empty() && GetServerPrimaryIPv6().empty()) {
      throw std::runtime_error("Unable to resolve the server's "
          "IPv4 or IPv6 address");
    }
#endif

    Config::Bind(ServerPort, ini, config, "Server.Port", 80);
    Config::Bind(ServerBacklog, ini, config, "Server.Backlog", 128);
    Config::Bind(ServerConnectionLimit, ini, config,
                 "Server.ConnectionLimit", 0);
    Config::Bind(ServerThreadCount, ini, config, "Server.ThreadCount",
                 Process::GetCPUCount() * 2);
    Config::Bind(ServerQueueCount, ini, config, "Server.QueueCount",
                 ServerThreadCount);
    Config::Bind(ServerHugeThreadCount, ini, config,
                 "Server.HugeThreadCount", 0);
    Config::Bind(ServerHugeStackKb, ini, config, "Server.HugeStackSizeKb", 384);
    Config::Bind(ServerLoopSampleRate, ini, config,
                 "Server.LoopSampleRate", 0);
    Config::Bind(ServerWarmupThrottleRequestCount, ini, config,
                 "Server.WarmupThrottleRequestCount",
                 ServerWarmupThrottleRequestCount);
    Config::Bind(ServerThreadDropCacheTimeoutSeconds, ini, config,
                 "Server.ThreadDropCacheTimeoutSeconds", 0);
    if (Config::GetBool(ini, config, "Server.ThreadJobLIFO")) {
      ServerThreadJobLIFOSwitchThreshold = 0;
    }
    Config::Bind(ServerThreadJobLIFOSwitchThreshold, ini, config,
                 "Server.ThreadJobLIFOSwitchThreshold",
                 ServerThreadJobLIFOSwitchThreshold);
    Config::Bind(ServerThreadJobMaxQueuingMilliSeconds, ini, config,
                 "Server.ThreadJobMaxQueuingMilliSeconds", -1);
    Config::Bind(ServerThreadDropStack, ini, config, "Server.ThreadDropStack");
    Config::Bind(ServerHttpSafeMode, ini, config, "Server.HttpSafeMode");
    Config::Bind(ServerStatCache, ini, config, "Server.StatCache", false);
    Config::Bind(ServerFixPathInfo, ini, config, "Server.FixPathInfo", false);
    Config::Bind(ServerAddVaryEncoding, ini, config, "Server.AddVaryEncoding",
                 ServerAddVaryEncoding);
    Config::Bind(ServerLogSettingsOnStartup, ini, config,
                 "Server.LogSettingsOnStartup", false);
    Config::Bind(ServerForkEnabled, ini, config,
                 "Server.Forking.Enabled", true);
    Config::Bind(ServerForkLogging, ini, config,
                 "Server.Forking.LogForkAttempts", false);
    Config::Bind(ServerWarmupRequests, ini, config, "Server.WarmupRequests");
    Config::Bind(ServerCleanupRequest, ini, config, "Server.CleanupRequest");
    Config::Bind(ServerInternalWarmupThreads, ini, config,
                 "Server.InternalWarmupThreads", 0);  // 0 = skip
    Config::Bind(ServerHighPriorityEndPoints, ini, config,
                 "Server.HighPriorityEndPoints");
    Config::Bind(ServerExitOnBindFail, ini, config, "Server.ExitOnBindFail",
                 false);

    Config::Bind(RequestTimeoutSeconds, ini, config,
                 "Server.RequestTimeoutSeconds", 0);
    Config::Bind(MaxRequestAgeFactor, ini, config, "Server.MaxRequestAgeFactor",
                 0);
    Config::Bind(PspTimeoutSeconds, ini, config, "Server.PspTimeoutSeconds", 0);
    Config::Bind(PspCpuTimeoutSeconds, ini, config,
                 "Server.PspCpuTimeoutSeconds", 0);
    Config::Bind(RequestMemoryMaxBytes, ini, config,
                 "Server.RequestMemoryMaxBytes", (16LL << 30)); // 16GiB
    Config::Bind(RequestMemoryOOMKillBytes, ini, config,
                 "Server.RequestMemoryOOMKillBytes", 128LL << 20);
    Config::Bind(RequestHugeMaxBytes, ini, config,
                 "Server.RequestHugeMaxBytes", (24LL << 20));
    Config::Bind(ServerGracefulShutdownWait, ini,
                 config, "Server.GracefulShutdownWait", 0);
    Config::Bind(ServerHarshShutdown, ini, config, "Server.HarshShutdown",
                 true);
    Config::Bind(ServerKillOnSIGTERM, ini, config, "Server.KillOnSIGTERM",
                 false);
    Config::Bind(ServerKillOnTimeout, ini, config, "Server.KillOnTimeout",
                 true);
    Config::Bind(ServerEvilShutdown, ini, config, "Server.EvilShutdown", true);
    Config::Bind(ServerPreShutdownWait, ini, config,
                 "Server.PreShutdownWait", 0);
    Config::Bind(ServerShutdownListenWait, ini, config,
                 "Server.ShutdownListenWait", 0);
    Config::Bind(ServerShutdownEOMWait, ini, config,
                 "Server.ShutdownEOMWait", 0);
    Config::Bind(ServerPrepareToStopTimeout, ini, config,
                 "Server.PrepareToStopTimeout", 240);
    Config::Bind(ServerPartialPostStatusCode, ini, config,
                 "Server.PartialPostStatusCode", -1);
    Config::Bind(StopOldServer, ini, config, "Server.StopOld", false);
    Config::Bind(OldServerWait, ini, config, "Server.StopOldWait", 30);
    Config::Bind(ServerRSSNeededMb, ini, config, "Server.RSSNeededMb", 4096);
    Config::Bind(ServerCriticalFreeMb, ini, config,
                 "Server.CriticalFreeMb", 512);
    Config::Bind(CacheFreeFactor, ini, config, "Server.CacheFreeFactor", 50);
    if (CacheFreeFactor > 100) CacheFreeFactor = 100;
    if (CacheFreeFactor < 0) CacheFreeFactor = 0;

    Config::Bind(ServerNextProtocols, ini, config, "Server.SSLNextProtocols");
    Config::Bind(ServerEnableH2C, ini, config, "Server.EnableH2C");
    Config::Bind(BrotliCompressionEnabled, ini, config,
                 "Server.BrotliCompressionEnabled", -1);
    Config::Bind(BrotliChunkedCompressionEnabled, ini, config,
                 "Server.BrotliChunkedCompressionEnabled", -1);
    Config::Bind(BrotliCompressionLgWindowSize, ini, config,
                 "Server.BrotliCompressionLgWindowSize", 20);
    Config::Bind(BrotliCompressionMode, ini, config,
                 "Server.BrotliCompressionMode", 0);
    Config::Bind(BrotliCompressionQuality, ini, config,
                 "Server.BrotliCompressionQuality", 6);
    Config::Bind(ZstdCompressionEnabled, ini, config,
                 "Server.ZstdCompressionEnabled", -1);
    Config::Bind(ZstdCompressionLevel, ini, config,
                 "Server.ZstdCompressionLevel", 3);
    Config::Bind(GzipCompressionLevel, ini, config,
                 "Server.GzipCompressionLevel", 3);
    Config::Bind(GzipMaxCompressionLevel, ini, config,
                 "Server.GzipMaxCompressionLevel", 9);
    Config::Bind(ForceCompressionURL, ini, config,
                 "Server.ForceCompression.URL");
    Config::Bind(ForceCompressionCookie, ini, config,
                 "Server.ForceCompression.Cookie");
    Config::Bind(ForceCompressionParam, ini, config,
                 "Server.ForceCompression.Param");
    Config::Bind(EnableKeepAlive, ini, config, "Server.EnableKeepAlive", true);
    Config::Bind(ExposeHPHP, ini, config, "Server.ExposeHPHP", true);
    Config::Bind(ExposeXFBServer, ini, config, "Server.ExposeXFBServer", false);
    Config::Bind(ExposeXFBDebug, ini, config, "Server.ExposeXFBDebug", false);
    Config::Bind(XFBDebugSSLKey, ini, config, "Server.XFBDebugSSLKey", "");
    Config::Bind(ConnectionTimeoutSeconds, ini, config,
                 "Server.ConnectionTimeoutSeconds", -1);
    Config::Bind(EnableOutputBuffering, ini, config,
                 "Server.EnableOutputBuffering");
    Config::Bind(OutputHandler, ini, config, "Server.OutputHandler");
    Config::Bind(ImplicitFlush, ini, config, "Server.ImplicitFlush");
    Config::Bind(EnableEarlyFlush, ini, config, "Server.EnableEarlyFlush",
                 true);
    Config::Bind(ForceChunkedEncoding, ini, config,
                 "Server.ForceChunkedEncoding");
    Config::Bind(MaxPostSize, ini, config, "Server.MaxPostSize", 100);
    MaxPostSize <<= 20;
    Config::Bind(AlwaysPopulateRawPostData, ini, config,
                 "Server.AlwaysPopulateRawPostData", false);
    Config::Bind(TakeoverFilename, ini, config, "Server.TakeoverFilename");
    Config::Bind(ExpiresActive, ini, config, "Server.ExpiresActive", true);
    Config::Bind(ExpiresDefault, ini, config, "Server.ExpiresDefault", 2592000);
    if (ExpiresDefault < 0) ExpiresDefault = 2592000;
    Config::Bind(DefaultCharsetName, ini, config, "Server.DefaultCharsetName",
                 "");
    Config::Bind(RequestBodyReadLimit, ini, config,
                 "Server.RequestBodyReadLimit", -1);
    Config::Bind(EnableSSL, ini, config, "Server.EnableSSL");
    Config::Bind(SSLPort, ini, config, "Server.SSLPort", 443);
    Config::Bind(SSLCertificateFile, ini, config, "Server.SSLCertificateFile");
    Config::Bind(SSLCertificateKeyFile, ini, config,
                 "Server.SSLCertificateKeyFile");
    Config::Bind(SSLCertificateDir, ini, config, "Server.SSLCertificateDir");
    Config::Bind(SSLTicketSeedFile, ini, config, "Server.SSLTicketSeedFile");
    Config::Bind(TLSDisableTLS1_2, ini, config, "Server.TLSDisableTLS1_2",
                 false);
    Config::Bind(TLSClientCipherSpec, ini, config,
                 "Server.TLSClientCipherSpec");
    Config::Bind(EnableSSLWithPlainText, ini, config,
                 "Server.EnableSSLWithPlainText");

    // SourceRoot has been default to: Process::GetCurrentDirectory() + '/'
    auto defSourceRoot = SourceRoot;
    Config::Bind(SourceRoot, ini, config, "Server.SourceRoot", SourceRoot);
    SourceRoot = FileUtil::normalizeDir(SourceRoot);
    if (SourceRoot.empty()) {
      SourceRoot = defSourceRoot;
    }
    FileCache::SourceRoot = SourceRoot;

    Config::Bind(IncludeSearchPaths, ini, config, "Server.IncludeSearchPaths");
    for (unsigned int i = 0; i < IncludeSearchPaths.size(); i++) {
      IncludeSearchPaths[i] = FileUtil::normalizeDir(IncludeSearchPaths[i]);
    }
    IncludeSearchPaths.insert(IncludeSearchPaths.begin(), ".");

    Config::Bind(FileCache, ini, config, "Server.FileCache");
    Config::Bind(DefaultDocument, ini, config, "Server.DefaultDocument",
                 "index.php");
    Config::Bind(ErrorDocument404, ini, config, "Server.ErrorDocument404");
    normalizePath(ErrorDocument404);
    Config::Bind(ForbiddenAs404, ini, config, "Server.ForbiddenAs404");
    Config::Bind(ErrorDocument500, ini, config, "Server.ErrorDocument500");
    normalizePath(ErrorDocument500);
    Config::Bind(FatalErrorMessage, ini, config, "Server.FatalErrorMessage");
    FontPath = FileUtil::normalizeDir(
      Config::GetString(ini, config, "Server.FontPath"));
    Config::Bind(EnableStaticContentFromDisk, ini, config,
                 "Server.EnableStaticContentFromDisk", true);
    Config::Bind(EnableOnDemandUncompress, ini, config,
                 "Server.EnableOnDemandUncompress", true);
    Config::Bind(EnableStaticContentMMap, ini, config,
                 "Server.EnableStaticContentMMap", true);
    if (EnableStaticContentMMap) {
      EnableOnDemandUncompress = true;
    }
    Config::Bind(Utf8izeReplace, ini, config, "Server.Utf8izeReplace", true);

    Config::Bind(RequestInitFunction, ini, config,
                 "Server.RequestInitFunction");
    Config::Bind(RequestInitDocument, ini, config,
                 "Server.RequestInitDocument");
    Config::Bind(SafeFileAccess, ini, config, "Server.SafeFileAccess");
    Config::Bind(AllowedDirectories, ini, config, "Server.AllowedDirectories");
    Config::Bind(WhitelistExec, ini, config, "Server.WhitelistExec");
    Config::Bind(WhitelistExecWarningOnly, ini, config,
                 "Server.WhitelistExecWarningOnly");
    Config::Bind(AllowedExecCmds, ini, config, "Server.AllowedExecCmds");
    Config::Bind(UnserializationWhitelistCheck, ini, config,
                 "Server.UnserializationWhitelistCheck", false);
    Config::Bind(UnserializationWhitelistCheckWarningOnly, ini, config,
                 "Server.UnserializationWhitelistCheckWarningOnly", true);
    Config::Bind(UnserializationBigMapThreshold, ini, config,
                 "Server.UnserializationBigMapThreshold", 1 << 16);
    Config::Bind(AllowedFiles, ini, config, "Server.AllowedFiles");
    Config::Bind(ForbiddenFileExtensions, ini, config,
                 "Server.ForbiddenFileExtensions");
    Config::Bind(LockCodeMemory, ini, config, "Server.LockCodeMemory", false);
    Config::Bind(MaxArrayChain, ini, config, "Server.MaxArrayChain", INT_MAX);
    if (MaxArrayChain != INT_MAX) {
      // MixedArray needs a higher threshold to avoid false-positives.
      // (and we always use MixedArray)
      MaxArrayChain *= 2;
    }

    Config::Bind(WarnOnCollectionToArray, ini, config,
                 "Server.WarnOnCollectionToArray", false);
    Config::Bind(UseDirectCopy, ini, config, "Server.UseDirectCopy", false);
    Config::Bind(AlwaysUseRelativePath, ini, config,
                 "Server.AlwaysUseRelativePath", false);
    {
      // Server Upload
      Config::Bind(UploadMaxFileSize, ini, config,
                   "Server.Upload.UploadMaxFileSize", 100);
      UploadMaxFileSize <<= 20;
      Config::Bind(UploadTmpDir, ini, config, "Server.Upload.UploadTmpDir",
                   "/tmp");
      Config::Bind(EnableFileUploads, ini, config,
                   "Server.Upload.EnableFileUploads", true);
      Config::Bind(MaxFileUploads, ini, config, "Server.Upload.MaxFileUploads",
                   20);
      Config::Bind(EnableUploadProgress, ini, config,
                   "Server.Upload.EnableUploadProgress");
      Config::Bind(Rfc1867Freq, ini, config, "Server.Upload.Rfc1867Freq",
                   256 * 1024);
      if (Rfc1867Freq < 0) Rfc1867Freq = 256 * 1024;
      Config::Bind(Rfc1867Prefix, ini, config, "Server.Upload.Rfc1867Prefix",
                   "vupload_");
      Config::Bind(Rfc1867Name, ini, config, "Server.Upload.Rfc1867Name",
                   "video_ptoken");
    }
    Config::Bind(ImageMemoryMaxBytes, ini, config,
                 "Server.ImageMemoryMaxBytes", 0);
    if (ImageMemoryMaxBytes == 0) {
      ImageMemoryMaxBytes = UploadMaxFileSize * 2;
    }
    Config::Bind(LightProcessFilePrefix, ini, config,
                 "Server.LightProcessFilePrefix", "./lightprocess");
    Config::Bind(LightProcessCount, ini, config,
                 "Server.LightProcessCount", 0);
    Config::Bind(ForceServerNameToHeader, ini, config,
                 "Server.ForceServerNameToHeader");
    Config::Bind(AllowDuplicateCookies, ini, config,
                 "Server.AllowDuplicateCookies", !EnableHipHopSyntax);
    Config::Bind(PathDebug, ini, config, "Server.PathDebug", false);
    Config::Bind(ServerUser, ini, config, "Server.User", "");
    Config::Bind(AllowRunAsRoot, ini, config, "Server.AllowRunAsRoot", false);
  }

  VirtualHost::SortAllowedDirectories(AllowedDirectories);
  {
    auto vh_callback = [] (const IniSettingMap &ini_vh, const Hdf &hdf_vh,
                           const std::string &ini_vh_key) {
      if (VirtualHost::IsDefault(ini_vh, hdf_vh, ini_vh_key)) {
        VirtualHost::GetDefault().init(ini_vh, hdf_vh, ini_vh_key);
        VirtualHost::GetDefault().addAllowedDirectories(AllowedDirectories);
      } else {
        auto host = std::make_shared<VirtualHost>(ini_vh, hdf_vh, ini_vh_key);
        host->addAllowedDirectories(AllowedDirectories);
        VirtualHosts.push_back(host);
      }
    };
    // Virtual Hosts have to be iterated in order. Because only the first
    // one that matches in the VirtualHosts vector gets applied and used.
    // Hdf's and ini (via Variant arrays) internal storage handles ordering
    // naturally (as specified top to bottom in the file and left to right on
    // the command line.
    Config::Iterate(vh_callback, ini, config, "VirtualHost");
    LowestMaxPostSize = VirtualHost::GetLowestMaxPostSize();
  }
  {
    // IpBlocks
    IpBlocks = std::make_shared<IpBlockMap>(ini, config);
  }
  {
    ReadSatelliteInfo(ini, config, SatelliteServerInfos,
                      XboxPassword, XboxPasswords);
  }
  {
    // Xbox
    Config::Bind(XboxServerThreadCount, ini, config,
                 "Xbox.ServerInfo.ThreadCount", 10);
    Config::Bind(XboxServerMaxQueueLength, ini, config,
                 "Xbox.ServerInfo.MaxQueueLength", INT_MAX);
    if (XboxServerMaxQueueLength < 0) XboxServerMaxQueueLength = INT_MAX;
    Config::Bind(XboxServerPort, ini, config, "Xbox.ServerInfo.Port", 0);
    Config::Bind(XboxDefaultLocalTimeoutMilliSeconds, ini, config,
                 "Xbox.DefaultLocalTimeoutMilliSeconds", 500);
    Config::Bind(XboxDefaultRemoteTimeoutSeconds, ini, config,
                 "Xbox.DefaultRemoteTimeoutSeconds", 5);
    Config::Bind(XboxServerInfoMaxRequest, ini, config,
                 "Xbox.ServerInfo.MaxRequest", 500);
    Config::Bind(XboxServerInfoDuration, ini, config,
                 "Xbox.ServerInfo.MaxDuration", 120);
    Config::Bind(XboxServerInfoWarmupDoc, ini, config,
                 "Xbox.ServerInfo.WarmupDocument", "");
    Config::Bind(XboxServerInfoReqInitFunc, ini, config,
                 "Xbox.ServerInfo.RequestInitFunction", "");
    Config::Bind(XboxServerInfoReqInitDoc, ini, config,
                 "Xbox.ServerInfo.RequestInitDocument", "");
    Config::Bind(XboxServerInfoAlwaysReset, ini, config,
                 "Xbox.ServerInfo.AlwaysReset", false);
    Config::Bind(XboxServerLogInfo, ini, config, "Xbox.ServerInfo.LogInfo",
                 false);
    Config::Bind(XboxProcessMessageFunc, ini, config, "Xbox.ProcessMessageFunc",
                 "xbox_process_message");
  }
  {
    // Pagelet Server
    Config::Bind(PageletServerThreadCount, ini, config,
                 "PageletServer.ThreadCount", 0);
    Config::Bind(PageletServerHugeThreadCount, ini, config,
                 "PageletServer.HugeThreadCount", 0);
    Config::Bind(PageletServerThreadDropStack, ini, config,
                 "PageletServer.ThreadDropStack");
    Config::Bind(PageletServerThreadDropCacheTimeoutSeconds, ini, config,
                 "PageletServer.ThreadDropCacheTimeoutSeconds", 0);
    Config::Bind(PageletServerQueueLimit, ini, config,
                 "PageletServer.QueueLimit", 0);
  }
  {
    // Static File

    hphp_string_imap<std::string> staticFileDefault;
    staticFileDefault["css"] = "text/css";
    staticFileDefault["gif"] = "image/gif";
    staticFileDefault["html"] = "text/html";
    staticFileDefault["jpeg"] = "image/jpeg";
    staticFileDefault["jpg"] = "image/jpeg";
    staticFileDefault["mp3"] = "audio/mpeg";
    staticFileDefault["png"] = "image/png";
    staticFileDefault["tif"] = "image/tiff";
    staticFileDefault["tiff"] = "image/tiff";
    staticFileDefault["txt"] = "text/plain";
    staticFileDefault["zip"] = "application/zip";

    Config::Bind(StaticFileExtensions, ini, config, "StaticFile.Extensions",
                 staticFileDefault);

    auto matches_callback = [](const IniSettingMap& ini_m, const Hdf& hdf_m,
                               const std::string& /*ini_m_key*/) {
      FilesMatches.push_back(std::make_shared<FilesMatch>(ini_m, hdf_m));
    };
    Config::Iterate(matches_callback, ini, config, "StaticFile.FilesMatch");
  }
  {
    // PhpFile
    Config::Bind(PhpFileExtensions, ini, config, "PhpFile.Extensions");
  }
  {
    // Admin Server
    Config::Bind(AdminServerIP, ini, config, "AdminServer.IP", ServerIP);
    Config::Bind(AdminServerPort, ini, config, "AdminServer.Port", 0);
    Config::Bind(AdminThreadCount, ini, config, "AdminServer.ThreadCount", 1);
    Config::Bind(AdminPassword, ini, config, "AdminServer.Password");
    Config::Bind(AdminPasswords, ini, config, "AdminServer.Passwords");
    Config::Bind(HashedAdminPasswords, ini, config,
                 "AdminServer.HashedPasswords");
  }
  {
    // Proxy
    Config::Bind(ProxyOriginRaw, ini, config, "Proxy.Origin");
    Config::Bind(ProxyPercentageRaw, ini, config, "Proxy.Percentage", 0);
    Config::Bind(ProxyRetry, ini, config, "Proxy.Retry", 3);
    Config::Bind(UseServeURLs, ini, config, "Proxy.ServeURLs");
    Config::Bind(ServeURLs, ini, config, "Proxy.ServeURLs");
    Config::Bind(UseProxyURLs, ini, config, "Proxy.ProxyURLs");
    Config::Bind(ProxyURLs, ini, config, "Proxy.ProxyURLs");
    Config::Bind(ProxyPatterns, ini, config, "Proxy.ProxyPatterns");
  }
  {
    // Http
    Config::Bind(HttpDefaultTimeout, ini, config, "Http.DefaultTimeout", 30);
    Config::Bind(HttpSlowQueryThreshold, ini, config, "Http.SlowQueryThreshold",
                 5000);
  }
  {
    // Debug

    Config::Bind(NativeStackTrace, ini, config, "Debug.NativeStackTrace");
    StackTrace::Enabled = NativeStackTrace;
    Config::Bind(ServerErrorMessage, ini, config, "Debug.ServerErrorMessage");
    Config::Bind(RecordInput, ini, config, "Debug.RecordInput");
    Config::Bind(ClearInputOnSuccess, ini, config, "Debug.ClearInputOnSuccess",
                 true);
    Config::Bind(ProfilerOutputDir, ini, config, "Debug.ProfilerOutputDir",
                 "/tmp");
    Config::Bind(CoreDumpEmail, ini, config, "Debug.CoreDumpEmail");
    Config::Bind(CoreDumpReport, ini, config, "Debug.CoreDumpReport", true);
    if (CoreDumpReport) {
      install_crash_reporter();
    }
    // Binding default dependenant on whether we are using an OSS build or
    // not, and that is set at initialization time of CoreDumpReportDirectory.
    Config::Bind(CoreDumpReportDirectory, ini, config,
                 "Debug.CoreDumpReportDirectory", CoreDumpReportDirectory);
    std::ostringstream stack_trace_stream;
    stack_trace_stream << CoreDumpReportDirectory << "/stacktrace."
                       << (int64_t)getpid() << ".log";
    StackTraceFilename = stack_trace_stream.str();

    Config::Bind(StackTraceTimeout, ini, config, "Debug.StackTraceTimeout", 0);
    Config::Bind(RemoteTraceOutputDir, ini, config,
                 "Debug.RemoteTraceOutputDir", "/tmp");
    Config::Bind(TraceFunctions, ini, config,
                 "Debug.TraceFunctions", TraceFunctions);
    Config::Bind(TraceFuncId, ini, config, "Debug.TraceFuncId", TraceFuncId);

    {
      // Debug SimpleCounter
      Config::Bind(SimpleCounter::SampleStackCount, ini,
                   config, "Debug.SimpleCounter.SampleStackCount", 0);
      Config::Bind(SimpleCounter::SampleStackDepth, ini,
                   config, "Debug.SimpleCounter.SampleStackDepth", 5);
    }
  }
  {
    // Stats
    Config::Bind(EnableStats, ini, config, "Stats.Enable",
                 false); // main switch
    Config::Bind(EnableAPCStats, ini, config, "Stats.APC", false);
    Config::Bind(EnableWebStats, ini, config, "Stats.Web");
    Config::Bind(EnableMemoryStats, ini, config, "Stats.Memory");
    Config::Bind(EnableSQLStats, ini, config, "Stats.SQL");
    Config::Bind(EnableSQLTableStats, ini, config, "Stats.SQLTable");
    Config::Bind(EnableNetworkIOStatus, ini, config, "Stats.NetworkIO");
    Config::Bind(StatsXSL, ini, config, "Stats.XSL");
    Config::Bind(StatsXSLProxy, ini, config, "Stats.XSLProxy");
    Config::Bind(StatsSlotDuration, ini, config, "Stats.SlotDuration", 10 * 60);
    Config::Bind(StatsMaxSlot, ini, config, "Stats.MaxSlot",
                 12 * 6); // 12 hours
    Config::Bind(EnableHotProfiler, ini, config, "Stats.EnableHotProfiler",
                 true);
    Config::Bind(ProfilerTraceBuffer, ini, config, "Stats.ProfilerTraceBuffer",
                 2000000);
    Config::Bind(ProfilerTraceExpansion, ini, config,
                 "Stats.ProfilerTraceExpansion", 1.2);
    Config::Bind(ProfilerMaxTraceBuffer, ini, config,
                 "Stats.ProfilerMaxTraceBuffer", 0);
    Config::Bind(TrackPerUnitMemory, ini, config,
                 "Stats.TrackPerUnitMemory", false);
  }
  {
    Config::Bind(ServerVariables, ini, config, "ServerVariables");
    Config::Bind(EnvVariables, ini, config, "EnvVariables");
  }
  {
    // Sandbox
    Config::Bind(SandboxMode, ini, config, "Sandbox.SandboxMode");
    Config::Bind(SandboxPattern, ini, config, "Sandbox.Pattern");
    SandboxPattern = format_pattern(SandboxPattern, true);
    Config::Bind(SandboxHome, ini, config, "Sandbox.Home");
    Config::Bind(SandboxFallback, ini, config, "Sandbox.Fallback");
    Config::Bind(SandboxConfFile, ini, config, "Sandbox.ConfFile");
    Config::Bind(SandboxFromCommonRoot, ini, config, "Sandbox.FromCommonRoot");
    Config::Bind(SandboxDirectoriesRoot, ini, config,
                 "Sandbox.DirectoriesRoot");
    Config::Bind(SandboxLogsRoot, ini, config, "Sandbox.LogsRoot");
    Config::Bind(SandboxServerVariables, ini, config,
                 "Sandbox.ServerVariables");
  }
  {
    // Mail
    Config::Bind(SendmailPath, ini, config, "Mail.SendmailPath",
                 "/usr/lib/sendmail -t -i");
    Config::Bind(MailForceExtraParameters, ini, config,
                 "Mail.ForceExtraParameters");
  }
  {
    // Preg
    Config::Bind(PregBacktraceLimit, ini, config, "Preg.BacktraceLimit",
                 1000000);
    Config::Bind(PregRecursionLimit, ini, config, "Preg.RecursionLimit",
                 100000);
    Config::Bind(EnablePregErrorLog, ini, config, "Preg.ErrorLog", true);
  }
  {
    // SimpleXML
    Config::Bind(SimpleXMLEmptyNamespaceMatchesAll, ini, config,
                 "SimpleXML.EmptyNamespaceMatchesAll", false);
  }
#ifdef FACEBOOK
  {
    // Fb303Server
    Config::Bind(EnableFb303Server, ini, config, "Fb303Server.Enable",
                 EnableFb303Server);
    Config::Bind(Fb303ServerPort, ini, config, "Fb303Server.Port", 0);
    Config::Bind(Fb303ServerThreadStackSizeMb, ini, config,
                 "Fb303Server.ThreadStackSizeMb", 8);
    Config::Bind(Fb303ServerWorkerThreads, ini, config,
                 "Fb303Server.WorkerThreads", 1);
    Config::Bind(Fb303ServerPoolThreads, ini, config, "Fb303Server.PoolThreads",
                 1);
  }
#endif

  {
    // Xenon
    Config::Bind(XenonPeriodSeconds, ini, config, "Xenon.Period", 0.0);
    Config::Bind(XenonRequestFreq, ini, config, "Xenon.RequestFreq", 1);
    Config::Bind(XenonForceAlwaysOn, ini, config, "Xenon.ForceAlwaysOn", false);
  }
  {
    // We directly read zend.assertions here, so that we can get its INI value
    // in order to know how we should emit bytecode. We don't actually Bind the
    // option here though, since its runtime value can be changed and is per
    // request. (We prevent its value from changing at runtime between values
    // that would affect byecode emission.)
    Variant v;
    bool b = IniSetting::GetSystem("zend.assertions", v);
    if (b) RuntimeOption::AssertEmitted = v.toInt64() >= 0;
  }

  Config::Bind(AliasedNamespaces, ini, config, "AliasedNamespaces");
  for (auto it = AliasedNamespaces.begin(); it != AliasedNamespaces.end(); ) {
    if (!is_valid_class_name(it->second)) {
      Logger::Warning("Skipping invalid AliasedNamespace %s\n",
                      it->second.c_str());
      it = AliasedNamespaces.erase(it);
      continue;
    }

    while (it->second.size() && it->second[0] == '\\') {
      it->second = it->second.substr(1);
    }

    ++it;
  }

  Config::Bind(TzdataSearchPaths, ini, config, "TzdataSearchPaths");

  Config::Bind(CustomSettings, ini, config, "CustomSettings");

  // Run initializers depedent on options, e.g., resizing atomic maps/vectors.
  refineStaticStringTableSize();
  InitFiniNode::ProcessPostRuntimeOptions();
  always_assert(Func::getFuncVec().size() == RuntimeOption::EvalFuncCountHint);

  // **************************************************************************
  //                                  DANGER
  //
  // Do not bind any PHP_INI_ALL or PHP_INI_USER settings here! These settings
  // are process-wide, while those need to be thread-local since they are
  // per-request. They should go into RequestInjectionData. Getting this wrong
  // will cause subtle breakage -- in particular, it probably will not show up
  // in CLI mode, since everything there tends to be single theaded.
  //
  // Per-dir INI settings are bound here, but that seems really questionable
  // since they can change per request too. TODO(#7757602) this should be
  // investigated.
  // **************************************************************************

  // Enables the hotfixing of a bug that occurred with D1797805 where
  // per request user settings (like upload_max_filesize) were not able to be
  // accessed on a server request any longer. The longer term fix is in review
  // D2099778, but we want that to simmer in Master for a while and we need
  // a hotfix for our current 3.6 LTS (Github Issue #4993)
  Config::Bind(EnableZendIniCompat, ini, config, "Eval.EnableZendIniCompat",
               true);
  // Language and Misc Configuration Options
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ONLY, "expose_php",
                   &RuntimeOption::ExposeHPHP);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "auto_prepend_file", &RuntimeOption::AutoPrependFile);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "auto_append_file", &RuntimeOption::AutoAppendFile);

  // Data Handling
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "post_max_size",
                   IniSetting::SetAndGet<int64_t>(
                     nullptr,
                     []() {
                       return VirtualHost::GetMaxPostSize();
                     }
                   ),
                   &RuntimeOption::MaxPostSize);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "always_populate_raw_post_data",
                   &RuntimeOption::AlwaysPopulateRawPostData);

  // Paths and Directories
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "doc_root", &RuntimeOption::SourceRoot);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "sendmail_path", &RuntimeOption::SendmailPath);

  // FastCGI
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ONLY,
                   "pid", &RuntimeOption::PidFile);

  // File Uploads
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "file_uploads", "true",
                   &RuntimeOption::EnableFileUploads);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "upload_tmp_dir", &RuntimeOption::UploadTmpDir);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_PERDIR,
                   "upload_max_filesize",
                   IniSetting::SetAndGet<std::string>(
                     [](const std::string& value) {
                       return ini_on_update(
                         value, RuntimeOption::UploadMaxFileSize);
                     },
                     []() {
                       return convert_long_to_bytes(
                         VirtualHost::GetUploadMaxFileSize());
                     }
                   ));
  // Filesystem and Streams Configuration Options
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "allow_url_fopen",
                   IniSetting::SetAndGet<std::string>(
                     [](const std::string& /*value*/) { return false; },
                     []() { return "1"; }));

  // HPHP specific
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_NONE,
                   "hphp.compiler_id",
                   IniSetting::SetAndGet<std::string>(
                     [](const std::string& /*value*/) { return false; },
                     []() { return compilerId().begin(); }));
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_NONE,
                   "hphp.compiler_version",
                   IniSetting::SetAndGet<std::string>(
                     [](const std::string& /*value*/) { return false; },
                     []() { return getHphpCompilerVersion(); }));
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_NONE,
                   "hphp.cli_server_api_version",
                   IniSetting::SetAndGet<uint64_t>(
                     [](const uint64_t /*value*/) { return false; },
                     []() { return CLI_SERVER_API_VERSION; }));
  IniSetting::Bind(
    IniSetting::CORE, IniSetting::PHP_INI_NONE, "hphp.build_id",
    IniSetting::SetAndGet<std::string>(
      [](const std::string& /*value*/) { return false; }, nullptr),
    &RuntimeOption::BuildId);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "notice_frequency",
                   &RuntimeOption::NoticeFrequency);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM,
                   "warning_frequency",
                   &RuntimeOption::WarningFrequency);
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_ONLY,
                   "hhvm.build_type",
                   IniSetting::SetAndGet<std::string>(
    [](const std::string&) {
      return false;
    },
    []() {
      return s_hhvm_build_type.c_str();
    }
  ));

  // Extensions
  Config::Bind(RuntimeOption::ExtensionDir, ini, config, "extension_dir",
               RuntimeOption::ExtensionDir, false);
  Config::Bind(RuntimeOption::DynamicExtensionPath, ini,
               config, "DynamicExtensionPath",
               RuntimeOption::DynamicExtensionPath);
  Config::Bind(RuntimeOption::Extensions, ini, config, "extensions");
  Config::Bind(RuntimeOption::DynamicExtensions, ini,
               config, "DynamicExtensions");

  ExtensionRegistry::moduleLoad(ini, config);
  initialize_apc();

  if (TraceFunctions.size() || TraceFuncId != InvalidFuncId) {
    Trace::ensureInit(getTraceOutputFile());
  }
}

///////////////////////////////////////////////////////////////////////////////
}
