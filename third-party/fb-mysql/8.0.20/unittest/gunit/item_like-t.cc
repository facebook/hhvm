/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include "sql/item.h"
#include "sql/item_cmpfunc.h"
#include "sql/sql_class.h"
#include "unittest/gunit/test_utils.h"

namespace item_like_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;
using ::testing::Return;

const char haystack[] =
    "CAAAACCACTATGAGATATCATCTCACACCAGTTAGAATGGCAATCATTA"
    "AAAAGTCAGGAAACAACAGGTGCTGGAGAGGATGCGGAGAAATAGGAACAC";

const char escape[] = "\\";

class ItemLikeTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    initializer.SetUp();
    it_haystack =
        new Item_string(STRING_WITH_LEN(haystack), &my_charset_latin1);
    it_escape = new Item_string(STRING_WITH_LEN(escape), &my_charset_latin1);
  }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
  Item_string *it_haystack;
  Item_string *it_escape;
};

TEST_F(ItemLikeTest, TestOne) {
  const char needle[] = "%ACAGGTGCTGGAGAGGATGCGGAGAAATAGGAACA%";

  Item_string *it_needle =
      new Item_string(STRING_WITH_LEN(needle), &my_charset_latin1);
  Item_func_like *item_BM =
      new Item_func_like(it_haystack, it_needle, it_escape, false);
  EXPECT_FALSE(item_BM->fix_fields(thd(), nullptr));
  EXPECT_EQ(1, item_BM->val_int());
}

// Increase number when doing performance comparisons.
// Run with ./unittest/gunit/item_like-t --disable-tap-output
//   to get timings from googletest.
const int num_executions = 1;  // 100000;

TEST_F(ItemLikeTest, PerfBasic) {
  const char needle[] = "%CAAAACCACTATGAGATATCATCTCACACCAGTTA%";
  Item_string *it_needle =
      new Item_string(STRING_WITH_LEN(needle), &my_charset_latin1);

  Item_func_like *item_BM =
      new Item_func_like(it_haystack, it_needle, it_escape, false);
  EXPECT_FALSE(item_BM->fix_fields(thd(), nullptr));
  for (int ix = 0; ix < num_executions; ++ix) {
    EXPECT_EQ(1, item_BM->val_int());
  }
}

class ItemLikeTestP : public ::testing::TestWithParam<const char *> {
 protected:
  virtual void SetUp() {
    initializer.SetUp();
    m_needle = GetParam();
  }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
  const char *m_needle;
};

class ItemFalseLikeTestP : public ::testing::TestWithParam<const char *> {
 protected:
  virtual void SetUp() {
    initializer.SetUp();
    m_needle = GetParam();
  }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
  const char *m_needle;
};

static const char *needles[] = {"%CAAAACCACTATGAGATATCATCTCACACCAGTTA%",
                                "%AAAACCACTATGAGATATCATCTCACACCAGTTAG%",
                                "%AAAACCACTATGAGATATCATCTCACACCAGTTAG%",
                                "%AAAACCACTATGAGATATCATCTCACACCAGTTAG%",
                                "%AAAACCACTATGAGATATCATCTCACACCAGTTAG%",
                                "%CAAAACCACTATGAGATATCATCTCACACCAGTTA%",
                                "%CAAAACCACTATGAGATATCATCTCACACCAGTTA%",
                                "%AAAACCACTATGAGATATCATCTCACACCAGTTAG%",
                                "%AAACCACTATGAGATATCATCTCACACCAGTTAGA%",
                                "%AACCACTATGAGATATCATCTCACACCAGTTAGAA%",
                                "%ACCACTATGAGATATCATCTCACACCAGTTAGAAT%",
                                "%CCACTATGAGATATCATCTCACACCAGTTAGAATG%",
                                "%CACTATGAGATATCATCTCACACCAGTTAGAATGG%",
                                "%ACTATGAGATATCATCTCACACCAGTTAGAATGGC%",
                                "%CTATGAGATATCATCTCACACCAGTTAGAATGGCA%",
                                "%TATGAGATATCATCTCACACCAGTTAGAATGGCAA%",
                                "%ATGAGATATCATCTCACACCAGTTAGAATGGCAAT%",
                                "%TGAGATATCATCTCACACCAGTTAGAATGGCAATC%",
                                "%GAGATATCATCTCACACCAGTTAGAATGGCAATCA%",
                                "%AGATATCATCTCACACCAGTTAGAATGGCAATCAT%",
                                "%GATATCATCTCACACCAGTTAGAATGGCAATCATT%",
                                "%ATATCATCTCACACCAGTTAGAATGGCAATCATTA%",
                                "%TATCATCTCACACCAGTTAGAATGGCAATCATTAA%",
                                "%ATCATCTCACACCAGTTAGAATGGCAATCATTAAA%",
                                "%TCATCTCACACCAGTTAGAATGGCAATCATTAAAA%",
                                "%CATCTCACACCAGTTAGAATGGCAATCATTAAAAA%",
                                "%ATCTCACACCAGTTAGAATGGCAATCATTAAAAAG%",
                                "%TCTCACACCAGTTAGAATGGCAATCATTAAAAAGT%",
                                "%CTCACACCAGTTAGAATGGCAATCATTAAAAAGTC%",
                                "%TCACACCAGTTAGAATGGCAATCATTAAAAAGTCA%",
                                "%CACACCAGTTAGAATGGCAATCATTAAAAAGTCAG%",
                                "%ACACCAGTTAGAATGGCAATCATTAAAAAGTCAGG%",
                                "%CACCAGTTAGAATGGCAATCATTAAAAAGTCAGGA%",
                                "%ACCAGTTAGAATGGCAATCATTAAAAAGTCAGGAA%",
                                "%CCAGTTAGAATGGCAATCATTAAAAAGTCAGGAAA%",
                                "%CAGTTAGAATGGCAATCATTAAAAAGTCAGGAAAC%",
                                "%AGTTAGAATGGCAATCATTAAAAAGTCAGGAAACA%",
                                "%GTTAGAATGGCAATCATTAAAAAGTCAGGAAACAA%",
                                "%TTAGAATGGCAATCATTAAAAAGTCAGGAAACAAC%",
                                "%TAGAATGGCAATCATTAAAAAGTCAGGAAACAACA%",
                                "%AGAATGGCAATCATTAAAAAGTCAGGAAACAACAG%",
                                "%GAATGGCAATCATTAAAAAGTCAGGAAACAACAGG%",
                                "%AATGGCAATCATTAAAAAGTCAGGAAACAACAGGT%",
                                "%ATGGCAATCATTAAAAAGTCAGGAAACAACAGGTG%",
                                "%TGGCAATCATTAAAAAGTCAGGAAACAACAGGTGC%",
                                "%GCAATCATTAAAAAGTCAGGAAACAACAGGTGCTG%",
                                "%CAATCATTAAAAAGTCAGGAAACAACAGGTGCTGG%",
                                "%AATCATTAAAAAGTCAGGAAACAACAGGTGCTGGA%",
                                "%ATCATTAAAAAGTCAGGAAACAACAGGTGCTGGAG%",
                                "%TCATTAAAAAGTCAGGAAACAACAGGTGCTGGAGA%",
                                "%CATTAAAAAGTCAGGAAACAACAGGTGCTGGAGAG%",
                                "%ATTAAAAAGTCAGGAAACAACAGGTGCTGGAGAGG%",
                                "%TTAAAAAGTCAGGAAACAACAGGTGCTGGAGAGGA%",
                                "%TAAAAAGTCAGGAAACAACAGGTGCTGGAGAGGAT%",
                                "%AAAAAGTCAGGAAACAACAGGTGCTGGAGAGGATG%",
                                "%AAAAGTCAGGAAACAACAGGTGCTGGAGAGGATGC%",
                                "%AAAGTCAGGAAACAACAGGTGCTGGAGAGGATGCG%",
                                "%AAGTCAGGAAACAACAGGTGCTGGAGAGGATGCGG%",
                                "%AGTCAGGAAACAACAGGTGCTGGAGAGGATGCGGA%",
                                "%GTCAGGAAACAACAGGTGCTGGAGAGGATGCGGAG%",
                                "%TCAGGAAACAACAGGTGCTGGAGAGGATGCGGAGA%",
                                "%CAGGAAACAACAGGTGCTGGAGAGGATGCGGAGAA%",
                                "%AGGAAACAACAGGTGCTGGAGAGGATGCGGAGAAA%",
                                "%GGAAACAACAGGTGCTGGAGAGGATGCGGAGAAAT%",
                                "%GAAACAACAGGTGCTGGAGAGGATGCGGAGAAATA%",
                                "%AAACAACAGGTGCTGGAGAGGATGCGGAGAAATAG%",
                                "%AACAACAGGTGCTGGAGAGGATGCGGAGAAATAGG%",
                                "%ACAACAGGTGCTGGAGAGGATGCGGAGAAATAGGA%",
                                "%CAACAGGTGCTGGAGAGGATGCGGAGAAATAGGAA%",
                                "%AACAGGTGCTGGAGAGGATGCGGAGAAATAGGAAC%",
                                "%ACAGGTGCTGGAGAGGATGCGGAGAAATAGGAACA%",
                                "%CAGGTGCTGGAGAGGATGCGGAGAAATAGGAACAC%"};

INSTANTIATE_TEST_CASE_P(MoreNeedles, ItemLikeTestP,
                        ::testing::ValuesIn(needles));

TEST_P(ItemLikeTestP, MoreNeedlesTest) {
  Item_string *it_haystack =
      new Item_string(STRING_WITH_LEN(haystack), &my_charset_latin1);
  Item_string *it_needle =
      new Item_string(m_needle, strlen(m_needle), &my_charset_latin1);
  Item_string *it_escape =
      new Item_string(STRING_WITH_LEN(escape), &my_charset_latin1);

  Item_func_like *item_BM =
      new Item_func_like(it_haystack, it_needle, it_escape, false);
  EXPECT_FALSE(item_BM->fix_fields(thd(), nullptr));
  EXPECT_EQ(1, item_BM->val_int());
}

static const char *falseNeedles[] = {

    "%ATTGACCACACTCTACTATAGAGTATCACCAAAAC%",
    "%GATTGACCACACTCTACTATAGAGTATCACCAAAA%",
    "%AGATTGACCACACTCTACTATAGAGTATCACCAAA%",
    "%AAGATTGACCACACTCTACTATAGAGTATCACCAA%",
    "%TAAGATTGACCACACTCTACTATAGAGTATCACCA%",
    "%GTAAGATTGACCACACTCTACTATAGAGTATCACC%",
    "%GGTAAGATTGACCACACTCTACTATAGAGTATCAC%",
    "%CGGTAAGATTGACCACACTCTACTATAGAGTATCA%",
    "%ACGGTAAGATTGACCACACTCTACTATAGAGTATC%",
    "%AACGGTAAGATTGACCACACTCTACTATAGAGTAT%",
    "%TAACGGTAAGATTGACCACACTCTACTATAGAGTA%",
    "%CTAACGGTAAGATTGACCACACTCTACTATAGAGT%",
    "%ACTAACGGTAAGATTGACCACACTCTACTATAGAG%",
    "%TACTAACGGTAAGATTGACCACACTCTACTATAGA%",
    "%TTACTAACGGTAAGATTGACCACACTCTACTATAG%",
    "%ATTACTAACGGTAAGATTGACCACACTCTACTATA%",
    "%AATTACTAACGGTAAGATTGACCACACTCTACTAT%",
    "%AAATTACTAACGGTAAGATTGACCACACTCTACTA%",
    "%AAAATTACTAACGGTAAGATTGACCACACTCTACT%",
    "%AAAAATTACTAACGGTAAGATTGACCACACTCTAC%",
    "%GAAAAATTACTAACGGTAAGATTGACCACACTCTA%",
    "%TGAAAAATTACTAACGGTAAGATTGACCACACTCT%",
    "%CTGAAAAATTACTAACGGTAAGATTGACCACACTC%",
    "%ACTGAAAAATTACTAACGGTAAGATTGACCACACT%",
    "%GACTGAAAAATTACTAACGGTAAGATTGACCACAC%",
    "%GGACTGAAAAATTACTAACGGTAAGATTGACCACA%",
    "%AGGACTGAAAAATTACTAACGGTAAGATTGACCAC%",
    "%AAGGACTGAAAAATTACTAACGGTAAGATTGACCA%",
    "%AAAGGACTGAAAAATTACTAACGGTAAGATTGACC%",
    "%CAAAGGACTGAAAAATTACTAACGGTAAGATTGAC%",
    "%ACAAAGGACTGAAAAATTACTAACGGTAAGATTGA%",
    "%AACAAAGGACTGAAAAATTACTAACGGTAAGATTG%",
    "%CAACAAAGGACTGAAAAATTACTAACGGTAAGATT%",
    "%ACAACAAAGGACTGAAAAATTACTAACGGTAAGAT%",
    "%GACAACAAAGGACTGAAAAATTACTAACGGTAAGA%",
    "%GGACAACAAAGGACTGAAAAATTACTAACGGTAAG%",
    "%TGGACAACAAAGGACTGAAAAATTACTAACGGTAA%",
    "%GTGGACAACAAAGGACTGAAAAATTACTAACGGTA%",
    "%CGTGGACAACAAAGGACTGAAAAATTACTAACGGT%",
    "%TCGTGGACAACAAAGGACTGAAAAATTACTAACGG%",
    "%GTCGTGGACAACAAAGGACTGAAAAATTACTAACG%",
    "%GGTCGTGGACAACAAAGGACTGAAAAATTACTAAC%",
    "%AGGTCGTGGACAACAAAGGACTGAAAAATTACTAA%",
    "%GAGGTCGTGGACAACAAAGGACTGAAAAATTACTA%",
    "%AGAGGTCGTGGACAACAAAGGACTGAAAAATTACT%",
    "%GAGAGGTCGTGGACAACAAAGGACTGAAAAATTAC%",
    "%GGAGAGGTCGTGGACAACAAAGGACTGAAAAATTA%",
    "%AGGAGAGGTCGTGGACAACAAAGGACTGAAAAATT%",
    "%TAGGAGAGGTCGTGGACAACAAAGGACTGAAAAAT%",
    "%GTAGGAGAGGTCGTGGACAACAAAGGACTGAAAAA%",
    "%CGTAGGAGAGGTCGTGGACAACAAAGGACTGAAAA%",
    "%GCGTAGGAGAGGTCGTGGACAACAAAGGACTGAAA%",
    "%GGCGTAGGAGAGGTCGTGGACAACAAAGGACTGAA%",
    "%AGGCGTAGGAGAGGTCGTGGACAACAAAGGACTGA%",
    "%GAGGCGTAGGAGAGGTCGTGGACAACAAAGGACTG%",
    "%AGAGGCGTAGGAGAGGTCGTGGACAACAAAGGACT%",
    "%AAGAGGCGTAGGAGAGGTCGTGGACAACAAAGGAC%",
    "%AAAGAGGCGTAGGAGAGGTCGTGGACAACAAAGGA%",
    "%TAAAGAGGCGTAGGAGAGGTCGTGGACAACAAAGG%",
    "%ATAAAGAGGCGTAGGAGAGGTCGTGGACAACAAAG%",
    "%GATAAAGAGGCGTAGGAGAGGTCGTGGACAACAAA%",
    "%GGATAAAGAGGCGTAGGAGAGGTCGTGGACAACAA%",
    "%AGGATAAAGAGGCGTAGGAGAGGTCGTGGACAACA%",
    "%AAGGATAAAGAGGCGTAGGAGAGGTCGTGGACAAC%",
    "%CAAGGATAAAGAGGCGTAGGAGAGGTCGTGGACAA%",
    "%ACAAGGATAAAGAGGCGTAGGAGAGGTCGTGGACA%",
    "%CACAAGGATAAAGAGGCGTAGGAGAGGTCGTGGAC%"};

INSTANTIATE_TEST_CASE_P(FalseNeedlesTest, ItemFalseLikeTestP,
                        ::testing::ValuesIn(falseNeedles));

TEST_P(ItemFalseLikeTestP, FalseNeedlesTest) {
  Item_string *it_haystack =
      new Item_string(STRING_WITH_LEN(haystack), &my_charset_latin1);
  Item_string *it_needle =
      new Item_string(m_needle, strlen(m_needle), &my_charset_latin1);
  Item_string *it_escape =
      new Item_string(STRING_WITH_LEN(escape), &my_charset_latin1);

  Item_func_like *item_BM =
      new Item_func_like(it_haystack, it_needle, it_escape, false);
  EXPECT_FALSE(item_BM->fix_fields(thd(), nullptr));
  EXPECT_EQ(0, item_BM->val_int());
}

}  // namespace item_like_unittest
