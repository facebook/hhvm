/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <signal.h>

#include <boost/program_options.hpp>

#include <folly/Format.h>
#include <folly/Singleton.h>
#include <folly/init/Init.h>
#include <folly/logging/Init.h>

#include "mcrouter/tools/mcpiper/McPiper.h"

using namespace facebook::memcache::mcpiper;

namespace {

std::unique_ptr<McPiper> gMcpiper;

[[noreturn]] void cleanExit(int32_t status) {
  for (auto sig : {SIGINT, SIGABRT, SIGQUIT, SIGPIPE, SIGWINCH}) {
    signal(sig, SIG_IGN);
  }

  if (status >= 0) {
    std::cerr << "exit" << std::endl
              << gMcpiper->stats().totalMessages.load()
              << " messages received, "
              << gMcpiper->stats().printedMessages.load() << " printed."
              << std::endl;
    auto beforeCompress = gMcpiper->stats().numBytesBeforeCompression.load();
    auto afterCompress = gMcpiper->stats().numBytesAfterCompression.load();
    if (beforeCompress > 0 && afterCompress > 0) {
      std::cerr << "Compression ratio = "
                << static_cast<double>(afterCompress) / beforeCompress
                << std::endl;
    }
  }

  exit(status);
}

std::string getUsage(const char* binaryName) {
  return folly::sformat(
      "Usage: {} [OPTION]... [PATTERN]\n"
      "Search for PATTERN in each mcrouter debug fifo in FIFO_ROOT "
      "(see options list) directory.\n"
      "If PATTERN is not provided, match everything.\n"
      "PATTERN is, by default, a basic regular expression (BRE).\n",
      binaryName);
}

Settings parseOptions(int argc, char** argv) {
  Settings settings;

  namespace po = boost::program_options;

  // Named options
  po::options_description namedOpts("Allowed options");
  namedOpts.add_options()("help,h", "Print this help message.")(
      "version", "Print mcpiper version and exit.")(
      "fifo-root,f",
      po::value<std::string>(&settings.fifoRoot),
      "Path of mcrouter fifo's directory.")(
      "filename-pattern,P",
      po::value<std::string>(&settings.filenamePattern),
      "Basic regular expression (BRE) to match the name of the fifos.")(
      "host,H",
      po::value<std::string>(&settings.host),
      "Show only messages sent/received to provided IP address.")(
      "ignore-case,i",
      po::bool_switch(&settings.ignoreCase)->default_value(false),
      "Ignore case on search patterns")(
      "invert-match,v",
      po::bool_switch(&settings.invertMatch)->default_value(false),
      "Invert match")(
      "max-messages,n",
      po::value<uint32_t>(&settings.maxMessages),
      "Display only <arg> messages and exit.")(
      "num-after-match,A",
      po::value<uint32_t>(&settings.numAfterMatch),
      "Shows <arg> messages after a matched message.")(
      "port,p",
      po::value<uint16_t>(&settings.port),
      "Show only messages transmitted in provided port.")(
      "quiet,q",
      po::bool_switch(&settings.quiet)->default_value(false),
      "Doesn't display values.")(
      "time-format,t",
      po::value<std::string>(&settings.timeFormat),
      "Displays timestamp on every match; "
      "ARG is \"absolute\", \"diff\" or \"offset\".")(
      "value-min-size,m",
      po::value<uint32_t>(&settings.valueMinSize),
      "Minimum size of the value of messages to display")(
      "value-max-size,M",
      po::value<uint32_t>(&settings.valueMaxSize),
      "Maximum size of the value of messages to display")(
      "min-latency-us,l",
      po::value<int64_t>(&settings.minLatencyUs),
      "Minimum latency in micros of messages to display")(
      "protocol",
      po::value<std::string>(&settings.protocol),
      "Show only data transmitted in the provided protocol; "
      "ARG is \"ascii\" or \"caret\".")(
      "verbose",
      po::value<size_t>(&settings.verboseLevel),
      "Set verbose level")(
      "raw",
      po::bool_switch(&settings.raw)->default_value(false),
      "Prints raw data. Format: firstly size(8 bytes) then message. "
      "ASCII protocol is not supported")(
      "script",
      po::bool_switch(&settings.script)->default_value(false),
      "Machine-readable JSON output (useful for post-processing).");

  // Positional arguments - hidden from the help message
  po::options_description hiddenOpts("Hidden options");
  hiddenOpts.add_options()(
      "match-expression",
      po::value<std::string>(&settings.matchExpression),
      "Match expression");
  po::positional_options_description posArgs;
  posArgs.add("match-expression", 1);

  // Parse command line
  po::variables_map vm;
  try {
    // Build all options
    po::options_description allOpts;
    allOpts.add(namedOpts).add(hiddenOpts);

    // Parse
    po::store(
        po::command_line_parser(argc, argv)
            .options(allOpts)
            .positional(posArgs)
            .run(),
        vm);
    po::notify(vm);
  } catch (po::error& ex) {
    LOG(ERROR) << ex.what();
    exit(1);
  }

  // Handles help
  if (vm.count("help")) {
    std::cerr << getUsage(argv[0]);
    std::cerr << std::endl;

    // Print only named options
    namedOpts.print(std::cerr);
    exit(0);
  }

  if (vm.count("version")) {
    std::cerr << facebook::memcache::getVersion() << std::endl;
    exit(0);
  }

  // Handles constraints
  CHECK(!settings.fifoRoot.empty())
      << "Fifo's directory (--fifo-root) cannot be empty";

  FLAGS_v = settings.verboseLevel;

  return settings;
}

} // anonymous namespace

// Configure folly to enable INFO+ messages, and everything else to
// enable WARNING+.
// Set the default log handler to log asynchronously by default.
FOLLY_INIT_LOGGING_CONFIG(".=WARNING,folly=INFO; default:async=true");

int main(int argc, char** argv) {
  // Just give the binary name to folly::init() because we use
  // boost::program_options instead of gflags.
  int tempArgc = 1;
  folly::init(&tempArgc, &argv, false);

  gMcpiper = std::make_unique<McPiper>();

  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = cleanExit;
  for (auto sig : {SIGINT, SIGABRT, SIGQUIT, SIGPIPE}) {
    sigaction(sig, &sa, nullptr);
  }

  gMcpiper->run(parseOptions(argc, argv));
  cleanExit(0);
}
