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

#include <cmath>
#include <iostream>

#include <thrift/lib/cpp2/protocol/Serializer.h>
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
  *ooe.some_characters() = "JSON THIS! \"\1";
  *ooe.zomg_unicode() = "\xd7\n\a\t";
  *ooe.base64() = "\1\2\3\255";
  cout << JSONSerializer::serialize<string>(ooe) << endl << endl;

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

  cout << JSONSerializer::serialize<string>(n) << endl << endl;

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

  cout << JSONSerializer::serialize<string>(hm) << endl << endl;

  string serialized;

  cout << "Testing ooe" << endl;

  serialized = JSONSerializer::serialize<string>(ooe);
  auto ooe2 = JSONSerializer::deserialize<OneOfEach>(serialized);

  assert(ooe == ooe2);

  cout << "Testing hm" << endl;

  serialized = JSONSerializer::serialize<string>(hm);
  auto hm2 = JSONSerializer::deserialize<HolyMoley>(serialized);

  assert(hm == hm2);

  *hm2.big()[0].a_bite() = 0xFF;

  assert(hm != hm2);

  Doubles dub;
  *dub.nan() = HUGE_VAL / HUGE_VAL;
  *dub.inf() = HUGE_VAL;
  *dub.neginf() = -HUGE_VAL;
  *dub.repeating() = 10.0 / 3.0;
  *dub.big() = 1E+305;
  *dub.small() = 1E-305;
  *dub.zero() = 0.0;
  *dub.negzero() = -0.0;
  cout << JSONSerializer::serialize<string>(dub) << endl << endl;

  Floats flt;
  *flt.nan() = HUGE_VAL / HUGE_VAL;
  *flt.inf() = HUGE_VAL;
  *flt.neginf() = -HUGE_VAL;
  *flt.repeating() = 10.0 / 3.0;
  *flt.big() = 3E+38;
  *flt.small() = 3E-38;
  *flt.zero() = 0.0;
  *flt.negzero() = -0.0;
  cout << JSONSerializer::serialize<string>(flt) << endl << endl;

  cout << "Testing base" << endl;

  Base64 base;
  *base.a() = 123;
  *base.b1() = "1";
  *base.b2() = "12";
  *base.b3() = "123";
  *base.b4() = "1234";
  *base.b5() = "12345";
  *base.b6() = "123456";

  serialized = JSONSerializer::serialize<string>(base);
  auto base2 = JSONSerializer::deserialize<Base64>(serialized);

  assert(base == base2);

  return 0;
}
