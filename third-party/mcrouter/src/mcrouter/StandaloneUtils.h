/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <unordered_map>
#include <unordered_set>

#include "mcrouter/Server.h"
#include "mcrouter/options.h"
#include "mcrouter/standalone_options.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

// Exit status constants
constexpr int kExitStatusTransientError = 2;
constexpr int kExitStatusUnrecoverableError = 3;

/**
 * Whether or not we are running on validate-config mode. Validate-config mode
 * is a mode where this binary will run just to validate configs.
 * It's useful just for testing config changes.
 */
enum class ValidateConfigMode {
  /**
   * We are *not* running on validate config mode. Just run mcrouter normally.
   */
  None,

  /**
   * In this mode, we will *not* run mcrouter. We will just validate configs
   * and exit the program (even if the config is valid).
   */
  Exit,

  /**
   * In this mode, we will run config validation and, it the configs are 100%
   * valid, we will run mcrouter.
   * The program will terminate if there's anything wrong with the configs (
   * e.g. we will NOT try to run configs from disk if configs are invalid).
   */
  Run
};

/**
 * This struct contains the options parsed out of command line arguments.
 */
struct CmdLineOptions {
  // the flavor to use (or empty string if none was provided).
  std::string flavor;

  // the libmcrouter options overrides (they will override flavor options).
  std::unordered_map<std::string, std::string> libmcrouterOptionsOverrides;

  // the stnadalone mcrouter options overrides (they will override flavor
  // options).
  std::unordered_map<std::string, std::string> standaloneOptionsOverrides;

  // the list of invalid/unrecognized option overrides.
  std::unordered_set<std::string> unrecognizedOptions;

  // in which mode we should run.
  ValidateConfigMode validateConfigMode{ValidateConfigMode::None};

  // the name of the binary (i.e. argv[0]).
  std::string programName;

  // the name of the package (if none is provided, will use argv[0]).
  std::string packageName;

  // the raw command line arguments (it doesn't include the name of the binary).
  std::string commandArgs;

  // the service name we want to override
  std::string serviceName;
};

/**
 * Parses the command line arguments.
 *
 * @param argc      argc argument to your main.
 * @param argv      argv argument to your main.
 * @param pkgName   The name of the package. If none is provided, argv[0] will
 *                  be used.
 */
CmdLineOptions
parseCmdLineOptions(int argc, char** argv, std::string pkgName = "");

/**
 * Fills in libmcrouterOptions and standaloneOptions by first reading data from
 * the flavor (in cmdLineOpts.flavor), and then applying the command-line
 * overrides (in cmdLineOpts.libmcrouterOptionsOverrides and
 * cmdLineOpts.standaloneOptionsOverrides).
 *
 * @param cmdLineOpts             The result of parseCmdLineOptions() funciton.
 * @param libmcrouterOptionsDict  Output argument with the final libmcrouter
 *                                options.
 * @param standaloneOptionsDict   Output argument with the final standalone
 *                                mcrouter options.
 */
void getFlavorOptionsAndApplyOverrides(
    const CmdLineOptions& cmdLineOpts,
    std::unordered_map<std::string, std::string>& libmcrouterOptionsDict,
    std::unordered_map<std::string, std::string>& standaloneOptionsDict);

/**
 * Setup standalone mcrouter.
 * It doesn't run the standalone mcrouter server. It just perform the necessary
 * initializetion, such as: setup logging file, report any invalid command-line
 * argumnets, initialize ssl, etc.
 *
 * @param serviceName             Mcrouter's service_name.
 * @param cmdLineOpts             The result of parseCmdLineOptions() funciton.
 * @param libmcrouterOptionsDict  The final libmcrouter options, as returned by
 *                                getFlavorOptionsAndApplyOverrides().
 * @param standaloneOptionsDict   The final standalone mcrouter options, as
 *                                returned by
 *                                getFlavorOptionsAndApplyOverrides().
 * @param libmcrouterOptions      Output argument with the final libmcrouter
 *                                options object.
 * @param standaloneOptions       Output argument with the final standalone
 *                                mcrouter options object.
 */
void setupStandaloneMcrouter(
    const std::string& serviceName,
    const CmdLineOptions& cmdLineOpts,
    const std::unordered_map<std::string, std::string>& libmcrouterOptionsDict,
    const std::unordered_map<std::string, std::string>& standaloneOptionsDict,
    McrouterOptions& libmcrouterOptions,
    McrouterStandaloneOptions& standaloneOptions);

/**
 * Starts standalone mcrouter.
 * Note: this function will just return when standalone mcrouter is shutdown.
 *
 * @param cmdLineOpts         The result of parseCmdLineOptions() funciton.
 * @param libmcrouterOptions  The final libmcrouter options object.
 * @param standaloneOptions   The final standalone mcrouter options object.
 * @param preRunCb            Callback called before starting the server.
 */
void runStandaloneMcrouter(
    const CmdLineOptions& cmdLineOpts,
    const McrouterOptions& libmcrouterOptions,
    const McrouterStandaloneOptions& standaloneOptions,
    StandalonePreRunCb preRunCb = nullptr);

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
