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

package com.facebook.thrift;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import thrift.test.proto.Bonk;
import thrift.test.proto.CompactProtoTestStruct;
import thrift.test.proto.HolyMoley;
import thrift.test.proto.Nesting;
import thrift.test.proto.OneOfEach;

public class Fixtures {

  private static final byte[] kUnicodeBytes = {
    (byte) 0xd3, (byte) 0x80, (byte) 0xe2, (byte) 0x85, (byte) 0xae, (byte) 0xce,
    (byte) 0x9d, (byte) 0x20, (byte) 0xd0, (byte) 0x9d, (byte) 0xce, (byte) 0xbf,
    (byte) 0xe2, (byte) 0x85, (byte) 0xbf, (byte) 0xd0, (byte) 0xbe, (byte) 0xc9,
    (byte) 0xa1, (byte) 0xd0, (byte) 0xb3, (byte) 0xd0, (byte) 0xb0, (byte) 0xcf,
    (byte) 0x81, (byte) 0xe2, (byte) 0x84, (byte) 0x8e, (byte) 0x20, (byte) 0xce,
    (byte) 0x91, (byte) 0x74, (byte) 0x74, (byte) 0xce, (byte) 0xb1, (byte) 0xe2,
    (byte) 0x85, (byte) 0xbd, (byte) 0xce, (byte) 0xba, (byte) 0x83, (byte) 0xe2,
    (byte) 0x80, (byte) 0xbc
  };

  public static final OneOfEach oneOfEach;
  public static final Nesting nesting;
  public static final HolyMoley holyMoley;
  public static final CompactProtoTestStruct compactProtoTestStruct;

  static {
    try {
      oneOfEach = new OneOfEach();
      oneOfEach.im_true = true;
      oneOfEach.im_false = false;
      oneOfEach.a_bite = (byte) 0x03;
      oneOfEach.integer16 = 27000;
      oneOfEach.integer32 = 1 << 24;
      oneOfEach.integer64 = (long) 6000 * 1000 * 1000;
      oneOfEach.double_precision = Math.PI;
      oneOfEach.some_characters = "JSON THIS! \"\1";
      oneOfEach.zomg_unicode = new String(kUnicodeBytes, "UTF-8");

      nesting = new Nesting(new Bonk(), new OneOfEach());
      nesting.my_ooe.integer16 = 16;
      nesting.my_ooe.integer32 = 32;
      nesting.my_ooe.integer64 = 64;
      nesting.my_ooe.double_precision = (Math.sqrt(5) + 1) / 2;
      nesting.my_ooe.some_characters = ":R (me going \"rrrr\")";
      nesting.my_ooe.zomg_unicode = new String(kUnicodeBytes, "UTF-8");
      nesting.my_bonk.type = 31337;
      nesting.my_bonk.message = "I am a bonk... xor!";

      holyMoley = new HolyMoley();

      holyMoley.big = new ArrayList<OneOfEach>();
      holyMoley.big.add(new OneOfEach(oneOfEach));
      holyMoley.big.add(nesting.my_ooe);
      holyMoley.big.get(0).a_bite = (byte) 0x22;
      holyMoley.big.get(1).a_bite = (byte) 0x23;

      holyMoley.contain = new HashSet<List<String>>();
      ArrayList<String> stage1 = new ArrayList<String>(2);
      stage1.add("and a one");
      stage1.add("and a two");
      holyMoley.contain.add(stage1);
      stage1 = new ArrayList<String>(3);
      stage1.add("then a one, two");
      stage1.add("three!");
      stage1.add("FOUR!!");
      holyMoley.contain.add(stage1);
      stage1 = new ArrayList<String>(0);
      holyMoley.contain.add(stage1);

      ArrayList<Bonk> stage2 = new ArrayList<Bonk>();
      holyMoley.bonks = new HashMap<String, List<Bonk>>();
      // one empty
      holyMoley.bonks.put("nothing", stage2);

      // one with two
      stage2 = new ArrayList<Bonk>();
      Bonk b = new Bonk();
      b.type = 1;
      b.message = "Wait.";
      stage2.add(b);
      b = new Bonk();
      b.type = 2;
      b.message = "What?";
      stage2.add(b);
      holyMoley.bonks.put("something", stage2);

      // one with three
      stage2 = new ArrayList<Bonk>();
      b = new Bonk();
      b.type = 3;
      b.message = "quoth";
      b = new Bonk();
      b.type = 4;
      b.message = "the raven";
      b = new Bonk();
      b.type = 5;
      b.message = "nevermore";
      holyMoley.bonks.put("poe", stage2);

      // superhuge compact proto test struct
      compactProtoTestStruct = new CompactProtoTestStruct(thrift.test.proto.Constants.COMPACT_TEST);
      compactProtoTestStruct.a_binary = new byte[] {0, 1, 2, 3, 4, 5, 6, 7, 8};
    } catch (Exception e) {
      throw new RuntimeException(e);
    }
  }
}
