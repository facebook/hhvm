/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <cmath>
#include <iostream>

#include <thrift/lib/cpp2/protocol/DebugProtocol.h>
#include <thrift/test/gen-cpp2/DebugProtoTest_constants.h>
#include <thrift/test/gen-cpp2/DebugProtoTest_types_custom_protocol.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::test;

int main() {
  OneOfEach ooe;
  *ooe.im_true() = true;
  *ooe.im_false() = false;
  *ooe.a_bite() = 0xd6;
  *ooe.integer16() = 27000;
  *ooe.integer32() = 1 << 24;
  *ooe.integer64() = (uint64_t)6000 * 1000 * 1000;
  *ooe.double_precision() = M_PI;
  *ooe.some_characters() = "Debug THIS!";
  *ooe.zomg_unicode() = "\xd7\n\a\t";
  ooe.string_string_map()["one"] = "two";
  ooe.string_string_hash_map()["three"] = "four";
  ooe.string_set()->insert("five");
  ooe.string_hash_set()->insert("six");
  *ooe.float_precision() = (float)12.345;
  ooe.rank_map()[567419810] = (float)0.211184;
  ooe.rank_map()[507959914] = (float)0.080382;

  cout << debugString(ooe) << endl << endl;

  cout << "--- const1" << endl;
  cout << debugString(DebugProtoTest_constants::const1()) << endl << endl;
  cout << "--- const2" << endl;
  cout << debugString(DebugProtoTest_constants::const2()) << endl << endl;

  Nesting n;
  *n.my_ooe() = ooe;
  *n.my_ooe()->integer16() = 16;
  *n.my_ooe()->integer32() = 32;
  *n.my_ooe()->integer64() = 64;
  *n.my_ooe()->double_precision() = (std::sqrt(5.0) + 1) / 2;
  *n.my_ooe()->some_characters() = ":R (me going \"rrrr\")";
  *n.my_ooe()->zomg_unicode() =
      "\xd3\x80\xe2\x85\xae\xce\x9d\x20"
      "\xd0\x9d\xce\xbf\xe2\x85\xbf\xd0\xbe\xc9\xa1\xd0\xb3\xd0\xb0\xcf\x81\xe2\x84\x8e"
      "\x20\xce\x91\x74\x74\xce\xb1\xe2\x85\xbd\xce\xba\xc7\x83\xe2\x80\xbc";
  *n.my_bonk()->type() = 31337;
  *n.my_bonk()->message() = "I am a bonk... xor!";

  cout << debugString(n) << endl << endl;

  HolyMoley hm;

  hm.big()->push_back(ooe);
  hm.big()->push_back(*n.my_ooe());
  *hm.big()[0].a_bite() = 0x22;
  *hm.big()[1].a_bite() = 0x33;

  std::vector<std::string> stage1;
  stage1.push_back("and a one");
  stage1.push_back("and a two");
  hm.contain()->insert(stage1);
  stage1.clear();
  stage1.push_back("then a one, two");
  stage1.push_back("three!");
  stage1.push_back("FOUR!!");
  hm.contain()->insert(stage1);
  stage1.clear();
  hm.contain()->insert(stage1);

  std::vector<Bonk> stage2;
  hm.bonks()["nothing"] = stage2;
  stage2.resize(stage2.size() + 1);
  *stage2.back().type() = 1;
  *stage2.back().message() = "Wait.";
  stage2.resize(stage2.size() + 1);
  *stage2.back().type() = 2;
  *stage2.back().message() = "What?";
  hm.bonks()["something"] = stage2;
  stage2.clear();
  stage2.resize(stage2.size() + 1);
  *stage2.back().type() = 3;
  *stage2.back().message() = "quoth";
  stage2.resize(stage2.size() + 1);
  *stage2.back().type() = 4;
  *stage2.back().message() = "the raven";
  stage2.resize(stage2.size() + 1);
  *stage2.back().type() = 5;
  *stage2.back().message() = "nevermore";
  hm.bonks()["poe"] = stage2;

  cout << debugString(hm) << endl << endl;

  TestUnion u;
  u.set_struct_field(std::move(ooe));
  cout << debugString(u) << endl << endl;

  return 0;
}
