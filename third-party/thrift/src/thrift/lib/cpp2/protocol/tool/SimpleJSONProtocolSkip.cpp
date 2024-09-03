/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdio>
#include <string>

#include <boost/program_options.hpp>

#include <folly/FileUtil.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>

namespace {

namespace po = boost::program_options;

struct cli_options {
  std::string input;
  size_t reps;
};

struct cli_parser {
  po::options_description desc; // all
  po::options_description ndesc{"Options"}; // named
  po::options_description pdesc; // positional
  po::positional_options_description extra;

  cli_parser() {
    ndesc.add_options()( //
        "input,i", // input file; if empty, stdin
        po::value<std::string>());
    ndesc.add_options()(
        "reps", // # times to parse in a loop
        po::value<size_t>() //
            ->default_value(1));
    desc.add(pdesc).add(ndesc);
  }

  po::variables_map parse(int argc, char** argv) const {
    po::command_line_parser parser{argc, argv};
    parser.options(desc);
    parser.positional(extra);
    po::variables_map vm;
    po::store(parser.run(), vm);
    vm.notify();
    return vm;
  }

  cli_options translate(po::variables_map const& vm) const {
    cli_options opts{};
    opts.input = vm["input"].as<std::string>();
    opts.reps = vm["reps"].as<size_t>();
    return opts;
  }

  cli_options operator()(int argc, char** argv) const {
    return translate(parse(argc, argv));
  }
};

static std::string read_input(cli_options const& opts) {
  std::string str;
  opts.input.empty() //
      ? folly::readFile(STDIN_FILENO, str)
      : folly::readFile(opts.input.c_str(), str);
  auto const pos = str.find_last_not_of(" \n\r\t");
  str.erase(pos + 1);
  return str;
}

static void do_skip(folly::io::Cursor cur) {
  apache::thrift::SimpleJSONProtocolReader reader;
  reader.setInput(cur);
  while (!reader.getCursor().isAtEnd()) {
    reader.skip(apache::thrift::protocol::TType::T_STRUCT);
  }
}

} // namespace

int main(int argc, char** argv) {
  static int one = 1;
  folly::Init init(&one, &argv, false);

  auto const opts = cli_parser{}(argc, argv);
  auto const in = read_input(opts);

  folly::IOBuf const buf(folly::IOBuf::WRAP_BUFFER, folly::StringPiece(in));
  folly::io::Cursor const cur{&buf};

  for (size_t i = 0; i < opts.reps; ++i) {
    do_skip(cur);
  }

  return 0;
}
