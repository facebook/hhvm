/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "StandaloneUtils.h"

#include <getopt.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <folly/Conv.h>

#include "mcrouter/CarbonRouterInstance.h"
#include "mcrouter/McrouterLogFailure.h"
#include "mcrouter/RouterRegistry.h"
#include "mcrouter/Server.h"
#include "mcrouter/StandaloneConfig.h"
#include "mcrouter/config.h"
#include "mcrouter/options.h"
#include "mcrouter/standalone_options.h"
#include "mcrouter/stats.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

thread_local size_t tlsWorkerThreadId;

namespace {

void printUsage(const char* option, const char* description) {
  fprintf(stderr, "\t%*s%s\n", -49, option, description);
}

[[noreturn]] void printUsageAndDie(
    const char* progName,
    int errorCode,
    const std::string& pkgName) {
  fprintf(
      stderr,
      "%s\n"
      "usage: %s [options] -p port(s) --config file:<config-file>\n\n",
      pkgName.c_str(),
      progName);

  fprintf(stderr, "libmcrouter options:\n");

  auto optData = McrouterOptions::getOptionData();
  auto standaloneOptData = McrouterStandaloneOptions::getOptionData();
  optData.insert(
      optData.end(), standaloneOptData.begin(), standaloneOptData.end());
  std::string current_group;
  for (auto& opt : optData) {
    if (!opt.long_option.empty()) {
      if (current_group != opt.group) {
        current_group = opt.group;
        fprintf(stderr, "\n  %s\n", current_group.c_str());
      }
      if (opt.short_option)
        fprintf(stderr, "\t-%c,", opt.short_option);
      else
        fprintf(stderr, "\t   ");

      fprintf(
          stderr,
          " --%*s %s",
          -42,
          opt.long_option.c_str(),
          opt.docstring.c_str());

      if (opt.type != McrouterOptionData::Type::toggle)
        fprintf(stderr, " [default: %s]", opt.default_value.c_str());

      fprintf(stderr, "\n");
    }
  }

  fprintf(stderr, "\nMisc options:\n");

  printUsage(
      "    --proxy-threads",
      "Like --num-proxies, but also accepts"
      " 'auto' to start one thread per core");
  printUsage("-h, --help", "help");
  printUsage("-V, --version", "version");
  printUsage("-v, --verbosity", "Set verbosity of VLOG");
  printUsage(
      "    --validate-config=exit|run",
      "Enable strict config checking. If 'exit' or no argument "
      "is provided, exit immediately with good or error status. "
      "Otherwise keep running if config is valid.");
  printUsage(
      "    --service-name",
      "Set the service name for standalone mcrouter. Default is \"mcrouter\".");

  fprintf(stderr, "\nRETURN VALUE\n");
  printUsage("2", "On a problem that might be resolved by restarting later.");
  static_assert(
      2 == kExitStatusTransientError, "Transient error status must be 2");
  printUsage(
      "3",
      "On a problem that will definitely not be resolved by "
      "restarting.");
  static_assert(
      3 == kExitStatusUnrecoverableError,
      "Unrecoverable error status must be 3");
  exit(errorCode);
}

bool areOptionsValid(
    const McrouterOptions& opts,
    const McrouterStandaloneOptions& standaloneOpts) {
  if (opts.num_proxies == 0) {
    LOG(ERROR) << "invalid number of proxy threads";
    return false;
  }
  if (standaloneOpts.ssl_ports.empty() && standaloneOpts.ports.empty() &&
      standaloneOpts.listen_sock_fd < 0 &&
      standaloneOpts.unix_domain_sock.empty()) {
    LOG(ERROR) << "invalid ports";
    return false;
  }

  if (opts.keepalive_idle_s <= 0 || opts.keepalive_interval_s <= 0 ||
      opts.keepalive_cnt < 0) {
    LOG(ERROR) << "invalid keepalive options";
    return false;
  }
  return true;
}

std::string constructArgString(int argc, char** argv) {
  std::string res;
  for (int i = 1; i < argc; ++i) {
    res += argv[i];
    res += (i == argc - 1) ? "" : " ";
  }
  return res;
}

void reportOptionsErrors(
    const McrouterOptions& opts,
    const std::vector<McrouterOptionError>& errors) {
  if (!errors.empty()) {
    for (auto& e : errors) {
      MC_LOG_FAILURE(
          opts,
          failure::Category::kInvalidOption,
          "Option parse error: {}={}, {}",
          e.requestedName,
          e.requestedValue,
          e.errorMsg);
    }
  }
}

template <class RouterInfo, template <class> class RequestHandler>
void validateConfigAndExit(const McrouterOptions& libmcrouterOptions) {
  try {
    auto router = CarbonRouterInstance<RouterInfo>::init(
        "standalone-validate", libmcrouterOptions);
    if (router == nullptr) {
      throw std::runtime_error("Couldn't create mcrouter.");
    }
  } catch (const std::exception& e) {
    LOG(ERROR) << "CRITICAL: Failed to initialize mcrouter: " << e.what();
    exit(kExitStatusUnrecoverableError);
  } catch (...) {
    LOG(ERROR) << "CRITICAL: Failed to initialize mcrouter";
    exit(kExitStatusUnrecoverableError);
  }

  // Exit immediately with good code
  _exit(0);
}

template <
    class RouterInfo,
    template <class>
    class RequestHandler,
    template <class>
    class ThriftRequestHandler>
void validateConfigAndExitThrift(const McrouterOptions& libmcrouterOptions) {
  validateConfigAndExit<RouterInfo, RequestHandler>(libmcrouterOptions);
}

template <class RouterInfo, template <class> class RequestHandler>
void run(
    const McrouterOptions& libmcrouterOptions,
    const McrouterStandaloneOptions& standaloneOptions,
    StandalonePreRunCb preRunCb) {
  LOG(INFO) << "Starting " << RouterInfo::name << " router";

  if (!runServer<RouterInfo, RequestHandler>(
          libmcrouterOptions, standaloneOptions, std::move(preRunCb))) {
    exit(kExitStatusTransientError);
  }
}

template <
    class RouterInfo,
    template <class>
    class RequestHandler,
    template <class>
    class ThriftRequestHandler>
void runDual(
    const McrouterOptions& libmcrouterOptions,
    const McrouterStandaloneOptions& standaloneOptions,
    StandalonePreRunCb preRunCb) {
  LOG(INFO) << "Starting dual mode" << RouterInfo::name << " router";

  if (!runServerDual<RouterInfo, RequestHandler, ThriftRequestHandler>(
          libmcrouterOptions, standaloneOptions, std::move(preRunCb))) {
    exit(kExitStatusTransientError);
  }
}

} // namespace

CmdLineOptions parseCmdLineOptions(int argc, char** argv, std::string pkgName) {
  CmdLineOptions res;

  CHECK(argc >= 1) << "invalid argc value";

  if (pkgName.empty()) {
    res.packageName = argv[0];
  } else {
    res.packageName = pkgName;
  }

  res.commandArgs = constructArgString(argc, argv);

  std::vector<option> longOptions = {
      {"verbosity", 0, nullptr, 'v'},
      {"help", 0, nullptr, 'h'},
      {"version", 0, nullptr, 'V'},
      {"validate-config", 2, nullptr, 0},
      {"proxy-threads", 1, nullptr, 0},
      {"service-name", 1, nullptr, 0},

      // Deprecated or not supported
      {"gets", 0, nullptr, 0},
      {"skip-sanity-checks", 0, nullptr, 0},
      {"retry-timeout", 1, nullptr, 0},
  };

  std::string optstring = "dD:v:hV";

  // Append auto-generated options to longOptions and optstring
  auto optionData = McrouterOptions::getOptionData();
  auto standaloneData = McrouterStandaloneOptions::getOptionData();
  auto combinedOptionData = optionData;
  combinedOptionData.insert(
      combinedOptionData.end(), standaloneData.begin(), standaloneData.end());
  for (auto& opt : combinedOptionData) {
    if (!opt.long_option.empty()) {
      // Toggle options can have optional argument
      int extraArgs = (opt.type == McrouterOptionData::Type::toggle ? 2 : 1);
      longOptions.push_back(
          {opt.long_option.c_str(), extraArgs, nullptr, opt.short_option});

      if (opt.short_option) {
        optstring.push_back(opt.short_option);
        if (extraArgs == 1) {
          optstring += ":";
        } else if (extraArgs == 2) {
          optstring += "::";
        }
      }
    }
  }

  longOptions.push_back({0, 0, 0, 0});

  int longIndex = -1;
  int c;
  while ((c = getopt_long(
              argc, argv, optstring.c_str(), longOptions.data(), &longIndex)) !=
         -1) {
    switch (c) {
      case 'v':
        FLAGS_v = folly::to<int>(optarg);
        break;

      case 'h':
        printUsageAndDie(argv[0], /* errorCode */ 0, pkgName);
      case 'V':
        printf("%s\n", MCROUTER_PACKAGE_STRING);
        exit(0);

      case 0:
      default:
        if (longIndex != -1 &&
            strcmp("constantly-reload-configs", longOptions[longIndex].name) ==
                0) {
          LOG(ERROR)
              << "CRITICAL: You have enabled constantly-reload-configs. "
                 "This undocumented feature is incredibly dangerous. "
                 "It will result in massively increased CPU consumption. "
                 "It will trigger lots of edge cases, surely causing hard failures. "
                 "If you're using this for *anything* other than testing, "
                 "please resign.";
        }

        // If the current short/long option is found in optData,
        // set it in the optDict and return true.  False otherwise.
        auto find_and_set =
            [&](const std::vector<McrouterOptionData>& optData,
                std::unordered_map<std::string, std::string>& optDict) {
              for (auto& opt : optData) {
                if (!opt.long_option.empty()) {
                  if ((opt.short_option && opt.short_option == c) ||
                      (!opt.long_option.empty() && longIndex != -1 &&
                       opt.long_option == longOptions[longIndex].name)) {
                    if (opt.type == McrouterOptionData::Type::toggle) {
                      optDict[opt.name] =
                          (optarg != nullptr
                               ? optarg
                               : (opt.default_value == "false" ? "1" : "0"));
                      ;
                    } else {
                      optDict[opt.name] = optarg;
                    }

                    return true;
                  }
                }
              }
              return false;
            };

        if (find_and_set(optionData, res.libmcrouterOptionsOverrides)) {
          break;
        }

        if (find_and_set(standaloneData, res.standaloneOptionsOverrides)) {
          break;
        }

        if (longIndex == -1) {
          res.unrecognizedOptions.insert(argv[optind - 1]);
        } else if (strcmp("proxy-threads", longOptions[longIndex].name) == 0) {
          if (strcmp(optarg, "auto") == 0) {
            int nprocs = sysconf(_SC_NPROCESSORS_ONLN);
            if (nprocs > 0) {
              res.libmcrouterOptionsOverrides["num_proxies"] =
                  std::to_string(nprocs);
            } else {
              LOG(INFO) << "Couldn't determine how many cores are available. "
                           "Defaulting to "
                        << DEFAULT_NUM_PROXIES << " proxy thread(s)";
            }
          } else {
            res.libmcrouterOptionsOverrides["num_proxies"] = optarg;
          }
        } else if (
            strcmp("validate-config", longOptions[longIndex].name) == 0) {
          if (!optarg || strcmp(optarg, "exit") == 0) {
            res.validateConfigMode = ValidateConfigMode::Exit;
          } else if (strcmp(optarg, "run") == 0) {
            res.validateConfigMode = ValidateConfigMode::Run;
          } else {
            LOG(ERROR) << "Invalid argument for --validate-config: '" << optarg
                       << "'. Ignoring the option.";
          }
        } else if (strcmp("retry-timeout", longOptions[longIndex].name) == 0) {
          LOG(WARNING) << "--retry-timeout is deprecated, use"
                          " --probe-timeout-initial";
          res.libmcrouterOptionsOverrides["probe_delay_initial_ms"] = optarg;
        } else if (strcmp("service-name", longOptions[longIndex].name) == 0) {
          // setup service name for standalone mcrouter
          res.serviceName = optarg;
        } else {
          res.unrecognizedOptions.insert(argv[optind - 1]);
        }
    }
    longIndex = -1;
  }

  // getopt permutes argv so that all non-options are at the end.
  // For now we only expect one non-option argument, so look at the last one.
  if (optind < argc && argv[optind]) {
    res.flavor = std::string(argv[optind]);
  }
  if (optind + 1 < argc) {
    LOG(ERROR) << "Expected only one non-option argument";
  }

  return res;
}

void getFlavorOptionsAndApplyOverrides(
    const CmdLineOptions& cmdLineOpts,
    std::unordered_map<std::string, std::string>& libmcrouterOptionsDict,
    std::unordered_map<std::string, std::string>& standaloneOptionsDict) {
  // This has to be called before reading from the flavor files, because it
  // may affect the way we read from flavors (fb-only).
  standalonePreInitFromCommandLineOpts(cmdLineOpts.standaloneOptionsOverrides);

  if (cmdLineOpts.flavor.empty()) {
    libmcrouterOptionsDict = cmdLineOpts.libmcrouterOptionsOverrides;
    standaloneOptionsDict = cmdLineOpts.standaloneOptionsOverrides;
  } else {
    read_standalone_flavor(
        cmdLineOpts.flavor, libmcrouterOptionsDict, standaloneOptionsDict);
    for (auto& it : cmdLineOpts.libmcrouterOptionsOverrides) {
      libmcrouterOptionsDict[it.first] = it.second;
    }
    for (auto& it : cmdLineOpts.standaloneOptionsOverrides) {
      standaloneOptionsDict[it.first] = it.second;
    }
  }
}

void setupStandaloneMcrouter(
    const std::string& serviceName,
    const CmdLineOptions& cmdLineOpts,
    const std::unordered_map<std::string, std::string>& libmcrouterOptionsDict,
    const std::unordered_map<std::string, std::string>& standaloneOptionsDict,
    McrouterOptions& libmcrouterOptions,
    McrouterStandaloneOptions& standaloneOptions) {
  // From now on, we might be reporting errors, so first thing is to
  // setup the logging file, if one was provided.
  auto logFile = standaloneOptionsDict.find("log_file");
  if (logFile != standaloneOptionsDict.end() && !logFile->second.empty()) {
    auto fd = open(
        logFile->second.c_str(),
        O_CREAT | O_WRONLY | O_APPEND,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd == -1) {
      LOG(ERROR) << "Couldn't open log file " << logFile->second
                 << " for writing: " << strerror(errno);
    } else {
      LOG(INFO) << "Logging to " << logFile->second;
      PCHECK(dup2(fd, STDERR_FILENO));
    }
  }

  // Before parsing options from dictionary to objects, we need to setup
  // the log failure, because we can call MC_LOG_FAILURE when parsing the
  // dictionaries.
  failure::setServiceContext(
      "mcrouter",
      folly::to<std::string>(
          cmdLineOpts.programName, " ", cmdLineOpts.commandArgs));
  failure::setHandler(failure::handlers::logToStdError());

  auto libmcrouterErrors =
      libmcrouterOptions.updateFromDict(libmcrouterOptionsDict);
  auto standaloneErrors =
      standaloneOptions.updateFromDict(standaloneOptionsDict);

  if (standaloneOptions.core_multiplier > 0) {
    auto c = std::thread::hardware_concurrency();
    if (!standaloneOptions.core_multiplier_threshold ||
        standaloneOptions.core_multiplier_threshold >= c) {
      libmcrouterOptions.num_proxies = c * standaloneOptions.core_multiplier;
    }
  }

  if (libmcrouterOptions.enable_failure_logging) {
    initFailureLogger();
  }

  // now that we (maybe) called initFailureLogger(), we can report the
  // errors with the options.
  reportOptionsErrors(libmcrouterOptions, libmcrouterErrors);
  reportOptionsErrors(libmcrouterOptions, standaloneErrors);
  for (const auto& option : cmdLineOpts.unrecognizedOptions) {
    MC_LOG_FAILURE(
        libmcrouterOptions,
        failure::Category::kInvalidOption,
        "Unrecognized option: {}",
        option);
  }

  // finialize standalone options
  finalizeStandaloneOptions(standaloneOptions);

  // init a few things.
  initStandaloneSSL();
  srand(time(nullptr) + getpid());

  // check if the values to provided to the options are sane.
  if (!areOptionsValid(libmcrouterOptions, standaloneOptions)) {
    printUsageAndDie(
        cmdLineOpts.programName.c_str(),
        kExitStatusUnrecoverableError,
        cmdLineOpts.packageName);
  }

  LOG(INFO) << cmdLineOpts.packageName << " startup (" << getpid() << ")";

  // update service_name and router_name
  if (cmdLineOpts.serviceName.empty()) {
    libmcrouterOptions.service_name = serviceName;
  } else {
    libmcrouterOptions.service_name = cmdLineOpts.serviceName;
  }
  if (cmdLineOpts.flavor.empty()) {
    std::string port = "0";
    if (!standaloneOptions.ports.empty()) {
      port = folly::to<std::string>(standaloneOptions.ports.at(0));
    }
    libmcrouterOptions.router_name = port;
  }

  // setup additional failure handler if necessary.
  if (cmdLineOpts.validateConfigMode != ValidateConfigMode::None) {
    failure::addHandler(failure::handlers::throwLogicError());
  }

  // initialize the follwing only if we are not going to exit right away.
  if (cmdLineOpts.validateConfigMode != ValidateConfigMode::Exit) {
    standaloneInit(libmcrouterOptions, standaloneOptions);
    set_standalone_args(cmdLineOpts.commandArgs);
  }
}

void runStandaloneMcrouter(
    const CmdLineOptions& cmdLineOpts,
    const McrouterOptions& libmcrouterOptions,
    const McrouterStandaloneOptions& standaloneOptions,
    StandalonePreRunCb preRunCb) {
  try {
    if (cmdLineOpts.validateConfigMode == ValidateConfigMode::Exit) {
      if (standaloneOptions.use_thrift) {
        CALL_BY_ROUTER_NAME_THRIFT(
            standaloneOptions.carbon_router_name,
            validateConfigAndExitThrift,
            libmcrouterOptions);
      } else {
        CALL_BY_ROUTER_NAME(
            standaloneOptions.carbon_router_name,
            validateConfigAndExit,
            libmcrouterOptions);
      }
    }

    if (standaloneOptions.use_thrift) {
      CALL_BY_ROUTER_NAME_THRIFT(
          standaloneOptions.carbon_router_name,
          runDual,
          libmcrouterOptions,
          standaloneOptions,
          std::move(preRunCb));
    } else {
      CALL_BY_ROUTER_NAME(
          standaloneOptions.carbon_router_name,
          run,
          libmcrouterOptions,
          standaloneOptions,
          std::move(preRunCb));
    }
  } catch (const std::invalid_argument& ia) {
    LOG(ERROR) << "Error starting mcrouter: " << ia.what();
    exit(EXIT_FAILURE);
  }
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
