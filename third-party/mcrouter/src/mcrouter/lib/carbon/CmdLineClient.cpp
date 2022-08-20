/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CmdLineClient.h"

#include <boost/program_options.hpp>

#include <folly/Format.h>

namespace carbon {
namespace tools {

namespace {

std::string getUsage(const char* binaryName) {
  return folly::sformat(
      "Usage: {} [OPTIONS]... [REQUEST_NAME] [REQUEST]\n"
      "Parses REQUEST json into the carbon request with name REQUEST_NAME "
      "and sends it to the server.\n",
      binaryName);
}

} // anonymous namespace

CmdLineClient::CmdLineClient(std::ostream& targetOut, std::ostream& targetErr)
    : targetOut_(targetOut), targetErr_(targetErr) {}

CmdLineClient::Settings CmdLineClient::parseSettings(
    int argc,
    const char** argv) {
  Settings settings;

  namespace po = boost::program_options;

  // Named options
  po::options_description namedOpts("Allowed options");
  namedOpts.add_options()("help", "Print this help message")(
      "host,h",
      po::value<std::string>(&settings.clientOptions.host),
      "The hostname/IPAddress of the carbon server")(
      "port,p",
      po::value<uint16_t>(&settings.clientOptions.port),
      "The port of the carbon server")(
      "timeout,t",
      po::value<size_t>(&settings.clientOptions.serverTimeoutMs),
      "The timeout in milliseconds")(
      "use-ssl",
      po::bool_switch(&settings.clientOptions.useSsl)->default_value(false),
      "Whether or not to use SSL")(
      "sslCertPath",
      po::value<std::string>(&settings.clientOptions.pemCertPath),
      "The SSL cert pem path")(
      "sslKeyPath",
      po::value<std::string>(&settings.clientOptions.pemKeyPath),
      "The SSL key pem path")(
      "sslCaPath",
      po::value<std::string>(&settings.clientOptions.pemCaPath),
      "The SSL CA pem path")(
      "sslServiceIdentity",
      po::value<std::string>(&settings.clientOptions.sslServiceIdentity),
      "The SSL service identity")(
      "stop-on-error",
      po::bool_switch(&settings.clientOptions.ignoreParsingErrors)
          ->default_value(true),
      "Do not send the requests if any error is found");

  // Positional arguments - hidden from the help message
  po::options_description hiddenOpts("Hidden options");
  hiddenOpts.add_options()(
      "requestName",
      po::value<std::string>(&settings.requestName),
      "The name of the request to send")(
      "data",
      po::value<std::string>(&settings.data),
      "The actual request(s) to send");
  po::positional_options_description posArgs;
  posArgs.add("requestName", 1);
  posArgs.add("data", 1);

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
    targetErr_ << ex.what() << std::endl;
    exit(1);
  }

  // Validate args
  bool error = false;
  if (settings.requestName.empty()) {
    error = true;
    targetErr_ << "ERROR: No request name provided." << std::endl;
  } else if (settings.data.empty()) {
    error = true;
    targetErr_ << "ERROR: No request data provided." << std::endl;
  }

  // Handles help
  if (vm.count("help") || error) {
    targetErr_ << getUsage(argv[0]) << std::endl;

    // Print only named options
    namedOpts.print(targetErr_);
    exit(0);
  }

  return settings;
}

void CmdLineClient::sendRequests(
    JsonClient& client,
    const std::string& requestName,
    const std::string& data) {
  folly::dynamic json;

  try {
    folly::json::serialization_opts opts;
    opts.allow_non_string_keys = true;
    opts.allow_trailing_comma = true;
    json = folly::parseJson(data, opts);
  } catch (const std::exception& e) {
    targetErr_ << "Error parsing json: " << e.what() << std::endl;
    return;
  }

  try {
    folly::dynamic reply;
    if (client.sendRequests(requestName, json, reply)) {
      targetOut_ << folly::toPrettyJson(reply) << std::endl;
    } else {
      targetErr_ << "Error sending requests to server." << std::endl;
    }
  } catch (const std::exception& e) {
    targetErr_ << "Error sending request: " << e.what() << std::endl;
    return;
  }
}

} // namespace tools
} // namespace carbon
