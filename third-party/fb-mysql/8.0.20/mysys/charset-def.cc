/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "m_ctype.h"
#include "my_compiler.h"
#include "my_inttypes.h"
#include "my_sys.h"

/**
  @file mysys/charset-def.cc
*/

/*
  Include all compiled character sets into the client
  If a client don't want to use all of them, they can define their own
  init_compiled_charsets() that only adds those that they want
*/

extern CHARSET_INFO my_charset_latin1_bin;
extern CHARSET_INFO my_charset_latin1_german2_ci;
extern CHARSET_INFO my_charset_big5_chinese_ci;
extern CHARSET_INFO my_charset_big5_bin;
extern CHARSET_INFO my_charset_cp1250_czech_ci;
extern CHARSET_INFO my_charset_cp932_japanese_ci;
extern CHARSET_INFO my_charset_cp932_bin;
extern CHARSET_INFO my_charset_latin2_czech_ci;
extern CHARSET_INFO my_charset_eucjpms_japanese_ci;
extern CHARSET_INFO my_charset_eucjpms_bin;
extern CHARSET_INFO my_charset_euckr_korean_ci;
extern CHARSET_INFO my_charset_euckr_bin;
extern CHARSET_INFO my_charset_gb2312_chinese_ci;
extern CHARSET_INFO my_charset_gb2312_bin;
extern CHARSET_INFO my_charset_gbk_chinese_ci;
extern CHARSET_INFO my_charset_gbk_bin;
extern CHARSET_INFO my_charset_gb18030_chinese_ci;
extern CHARSET_INFO my_charset_gb18030_bin;
extern CHARSET_INFO my_charset_sjis_japanese_ci;
extern CHARSET_INFO my_charset_sjis_bin;
extern CHARSET_INFO my_charset_tis620_thai_ci;
extern CHARSET_INFO my_charset_tis620_bin;
extern CHARSET_INFO my_charset_ujis_japanese_ci;
extern CHARSET_INFO my_charset_ujis_bin;

extern CHARSET_INFO my_charset_ucs2_general_ci;
extern CHARSET_INFO my_charset_ucs2_unicode_ci;
extern CHARSET_INFO my_charset_ucs2_bin;
extern CHARSET_INFO my_charset_ucs2_general_mysql500_ci;
extern CHARSET_INFO my_charset_ucs2_german2_uca_ci;
extern CHARSET_INFO my_charset_ucs2_icelandic_uca_ci;
extern CHARSET_INFO my_charset_ucs2_latvian_uca_ci;
extern CHARSET_INFO my_charset_ucs2_romanian_uca_ci;
extern CHARSET_INFO my_charset_ucs2_slovenian_uca_ci;
extern CHARSET_INFO my_charset_ucs2_polish_uca_ci;
extern CHARSET_INFO my_charset_ucs2_estonian_uca_ci;
extern CHARSET_INFO my_charset_ucs2_spanish_uca_ci;
extern CHARSET_INFO my_charset_ucs2_swedish_uca_ci;
extern CHARSET_INFO my_charset_ucs2_turkish_uca_ci;
extern CHARSET_INFO my_charset_ucs2_czech_uca_ci;
extern CHARSET_INFO my_charset_ucs2_danish_uca_ci;
extern CHARSET_INFO my_charset_ucs2_lithuanian_uca_ci;
extern CHARSET_INFO my_charset_ucs2_slovak_uca_ci;
extern CHARSET_INFO my_charset_ucs2_spanish2_uca_ci;
extern CHARSET_INFO my_charset_ucs2_roman_uca_ci;
extern CHARSET_INFO my_charset_ucs2_persian_uca_ci;
extern CHARSET_INFO my_charset_ucs2_esperanto_uca_ci;
extern CHARSET_INFO my_charset_ucs2_hungarian_uca_ci;
extern CHARSET_INFO my_charset_ucs2_croatian_uca_ci;
extern CHARSET_INFO my_charset_ucs2_sinhala_uca_ci;
extern CHARSET_INFO my_charset_ucs2_unicode_520_ci;
extern CHARSET_INFO my_charset_ucs2_vietnamese_ci;

extern CHARSET_INFO my_charset_utf32_general_ci;
extern CHARSET_INFO my_charset_utf32_bin;
extern CHARSET_INFO my_charset_utf32_german2_uca_ci;
extern CHARSET_INFO my_charset_utf32_icelandic_uca_ci;
extern CHARSET_INFO my_charset_utf32_latvian_uca_ci;
extern CHARSET_INFO my_charset_utf32_romanian_uca_ci;
extern CHARSET_INFO my_charset_utf32_slovenian_uca_ci;
extern CHARSET_INFO my_charset_utf32_polish_uca_ci;
extern CHARSET_INFO my_charset_utf32_estonian_uca_ci;
extern CHARSET_INFO my_charset_utf32_spanish_uca_ci;
extern CHARSET_INFO my_charset_utf32_swedish_uca_ci;
extern CHARSET_INFO my_charset_utf32_turkish_uca_ci;
extern CHARSET_INFO my_charset_utf32_czech_uca_ci;
extern CHARSET_INFO my_charset_utf32_danish_uca_ci;
extern CHARSET_INFO my_charset_utf32_lithuanian_uca_ci;
extern CHARSET_INFO my_charset_utf32_slovak_uca_ci;
extern CHARSET_INFO my_charset_utf32_spanish2_uca_ci;
extern CHARSET_INFO my_charset_utf32_roman_uca_ci;
extern CHARSET_INFO my_charset_utf32_persian_uca_ci;
extern CHARSET_INFO my_charset_utf32_esperanto_uca_ci;
extern CHARSET_INFO my_charset_utf32_hungarian_uca_ci;
extern CHARSET_INFO my_charset_utf32_croatian_uca_ci;
extern CHARSET_INFO my_charset_utf32_sinhala_uca_ci;
extern CHARSET_INFO my_charset_utf32_unicode_520_ci;
extern CHARSET_INFO my_charset_utf32_vietnamese_ci;

extern CHARSET_INFO my_charset_utf16_general_ci;
extern CHARSET_INFO my_charset_utf16_unicode_ci;
extern CHARSET_INFO my_charset_utf16_bin;
extern CHARSET_INFO my_charset_utf16le_general_ci;
extern CHARSET_INFO my_charset_utf16le_bin;
extern CHARSET_INFO my_charset_utf16_german2_uca_ci;
extern CHARSET_INFO my_charset_utf16_icelandic_uca_ci;
extern CHARSET_INFO my_charset_utf16_latvian_uca_ci;
extern CHARSET_INFO my_charset_utf16_romanian_uca_ci;
extern CHARSET_INFO my_charset_utf16_slovenian_uca_ci;
extern CHARSET_INFO my_charset_utf16_polish_uca_ci;
extern CHARSET_INFO my_charset_utf16_estonian_uca_ci;
extern CHARSET_INFO my_charset_utf16_spanish_uca_ci;
extern CHARSET_INFO my_charset_utf16_swedish_uca_ci;
extern CHARSET_INFO my_charset_utf16_turkish_uca_ci;
extern CHARSET_INFO my_charset_utf16_czech_uca_ci;
extern CHARSET_INFO my_charset_utf16_danish_uca_ci;
extern CHARSET_INFO my_charset_utf16_lithuanian_uca_ci;
extern CHARSET_INFO my_charset_utf16_slovak_uca_ci;
extern CHARSET_INFO my_charset_utf16_spanish2_uca_ci;
extern CHARSET_INFO my_charset_utf16_roman_uca_ci;
extern CHARSET_INFO my_charset_utf16_persian_uca_ci;
extern CHARSET_INFO my_charset_utf16_esperanto_uca_ci;
extern CHARSET_INFO my_charset_utf16_hungarian_uca_ci;
extern CHARSET_INFO my_charset_utf16_croatian_uca_ci;
extern CHARSET_INFO my_charset_utf16_sinhala_uca_ci;
extern CHARSET_INFO my_charset_utf16_unicode_520_ci;
extern CHARSET_INFO my_charset_utf16_vietnamese_ci;

extern CHARSET_INFO my_charset_utf8_tolower_ci;
extern CHARSET_INFO my_charset_utf8_bin;
extern CHARSET_INFO my_charset_utf8_general_mysql500_ci;
extern CHARSET_INFO my_charset_utf8_german2_uca_ci;
extern CHARSET_INFO my_charset_utf8_icelandic_uca_ci;
extern CHARSET_INFO my_charset_utf8_latvian_uca_ci;
extern CHARSET_INFO my_charset_utf8_romanian_uca_ci;
extern CHARSET_INFO my_charset_utf8_slovenian_uca_ci;
extern CHARSET_INFO my_charset_utf8_polish_uca_ci;
extern CHARSET_INFO my_charset_utf8_estonian_uca_ci;
extern CHARSET_INFO my_charset_utf8_spanish_uca_ci;
extern CHARSET_INFO my_charset_utf8_swedish_uca_ci;
extern CHARSET_INFO my_charset_utf8_turkish_uca_ci;
extern CHARSET_INFO my_charset_utf8_czech_uca_ci;
extern CHARSET_INFO my_charset_utf8_danish_uca_ci;
extern CHARSET_INFO my_charset_utf8_lithuanian_uca_ci;
extern CHARSET_INFO my_charset_utf8_slovak_uca_ci;
extern CHARSET_INFO my_charset_utf8_spanish2_uca_ci;
extern CHARSET_INFO my_charset_utf8_roman_uca_ci;
extern CHARSET_INFO my_charset_utf8_persian_uca_ci;
extern CHARSET_INFO my_charset_utf8_esperanto_uca_ci;
extern CHARSET_INFO my_charset_utf8_hungarian_uca_ci;
extern CHARSET_INFO my_charset_utf8_croatian_uca_ci;
extern CHARSET_INFO my_charset_utf8_sinhala_uca_ci;
extern CHARSET_INFO my_charset_utf8_unicode_520_ci;
extern CHARSET_INFO my_charset_utf8_vietnamese_ci;

extern CHARSET_INFO my_charset_utf8mb4_general_ci;
extern CHARSET_INFO my_charset_utf8mb4_unicode_ci;
extern CHARSET_INFO my_charset_utf8mb4_bin;
extern CHARSET_INFO my_charset_utf8mb4_german2_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_icelandic_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_latvian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_romanian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_slovenian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_polish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_estonian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_spanish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_swedish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_turkish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_czech_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_danish_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_lithuanian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_slovak_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_spanish2_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_roman_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_persian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_esperanto_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_hungarian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_croatian_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_sinhala_uca_ci;
extern CHARSET_INFO my_charset_utf8mb4_unicode_520_ci;
extern CHARSET_INFO my_charset_utf8mb4_vietnamese_ci;
extern CHARSET_INFO my_charset_utf8mb4_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_de_pb_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_is_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_lv_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_ro_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_sl_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_pl_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_et_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_es_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_sv_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_tr_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_cs_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_da_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_lt_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_sk_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_es_trad_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_la_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_eo_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_hu_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_hr_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_vi_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_ru_0900_ai_ci;
extern CHARSET_INFO my_charset_utf8mb4_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_de_pb_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_is_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_lv_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_ro_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_sl_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_pl_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_et_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_es_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_sv_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_tr_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_cs_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_da_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_lt_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_sk_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_es_trad_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_la_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_eo_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_hu_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_hr_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_vi_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_ja_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_ja_0900_as_cs_ks;
extern CHARSET_INFO my_charset_utf8mb4_0900_as_ci;
extern CHARSET_INFO my_charset_utf8mb4_ru_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_zh_0900_as_cs;
extern CHARSET_INFO my_charset_utf8mb4_0900_bin;

extern CHARSET_INFO my_charset_gb18030_unicode_520_ci;

bool init_compiled_charsets(myf flags MY_ATTRIBUTE((unused))) {
  CHARSET_INFO *cs;

  add_compiled_collation(&my_charset_bin);

  add_compiled_collation(&my_charset_latin1);
  add_compiled_collation(&my_charset_latin1_bin);
  add_compiled_collation(&my_charset_latin1_german2_ci);

  add_compiled_collation(&my_charset_big5_chinese_ci);
  add_compiled_collation(&my_charset_big5_bin);

  add_compiled_collation(&my_charset_cp1250_czech_ci);

  add_compiled_collation(&my_charset_cp932_japanese_ci);
  add_compiled_collation(&my_charset_cp932_bin);

  add_compiled_collation(&my_charset_latin2_czech_ci);

  add_compiled_collation(&my_charset_eucjpms_japanese_ci);
  add_compiled_collation(&my_charset_eucjpms_bin);

  add_compiled_collation(&my_charset_euckr_korean_ci);
  add_compiled_collation(&my_charset_euckr_bin);

  add_compiled_collation(&my_charset_gb2312_chinese_ci);
  add_compiled_collation(&my_charset_gb2312_bin);

  add_compiled_collation(&my_charset_gbk_chinese_ci);
  add_compiled_collation(&my_charset_gbk_bin);

  add_compiled_collation(&my_charset_gb18030_unicode_520_ci);
  add_compiled_collation(&my_charset_gb18030_chinese_ci);
  add_compiled_collation(&my_charset_gb18030_bin);

  add_compiled_collation(&my_charset_sjis_japanese_ci);
  add_compiled_collation(&my_charset_sjis_bin);

  add_compiled_collation(&my_charset_tis620_thai_ci);
  add_compiled_collation(&my_charset_tis620_bin);

  add_compiled_collation(&my_charset_ucs2_general_ci);
  add_compiled_collation(&my_charset_ucs2_bin);
  add_compiled_collation(&my_charset_ucs2_general_mysql500_ci);
  add_compiled_collation(&my_charset_ucs2_unicode_ci);
  add_compiled_collation(&my_charset_ucs2_german2_uca_ci);
  add_compiled_collation(&my_charset_ucs2_icelandic_uca_ci);
  add_compiled_collation(&my_charset_ucs2_latvian_uca_ci);
  add_compiled_collation(&my_charset_ucs2_romanian_uca_ci);
  add_compiled_collation(&my_charset_ucs2_slovenian_uca_ci);
  add_compiled_collation(&my_charset_ucs2_polish_uca_ci);
  add_compiled_collation(&my_charset_ucs2_estonian_uca_ci);
  add_compiled_collation(&my_charset_ucs2_spanish_uca_ci);
  add_compiled_collation(&my_charset_ucs2_swedish_uca_ci);
  add_compiled_collation(&my_charset_ucs2_turkish_uca_ci);
  add_compiled_collation(&my_charset_ucs2_czech_uca_ci);
  add_compiled_collation(&my_charset_ucs2_danish_uca_ci);
  add_compiled_collation(&my_charset_ucs2_lithuanian_uca_ci);
  add_compiled_collation(&my_charset_ucs2_slovak_uca_ci);
  add_compiled_collation(&my_charset_ucs2_spanish2_uca_ci);
  add_compiled_collation(&my_charset_ucs2_roman_uca_ci);
  add_compiled_collation(&my_charset_ucs2_persian_uca_ci);
  add_compiled_collation(&my_charset_ucs2_esperanto_uca_ci);
  add_compiled_collation(&my_charset_ucs2_hungarian_uca_ci);
  add_compiled_collation(&my_charset_ucs2_croatian_uca_ci);
  add_compiled_collation(&my_charset_ucs2_sinhala_uca_ci);
  add_compiled_collation(&my_charset_ucs2_unicode_520_ci);
  add_compiled_collation(&my_charset_ucs2_vietnamese_ci);

  add_compiled_collation(&my_charset_ujis_japanese_ci);
  add_compiled_collation(&my_charset_ujis_bin);

  add_compiled_collation(&my_charset_utf8_general_ci);
  add_compiled_collation(&my_charset_utf8_tolower_ci);
  add_compiled_collation(&my_charset_utf8_bin);
  add_compiled_collation(&my_charset_utf8_general_mysql500_ci);
  add_compiled_collation(&my_charset_utf8_unicode_ci);
  add_compiled_collation(&my_charset_utf8_german2_uca_ci);
  add_compiled_collation(&my_charset_utf8_icelandic_uca_ci);
  add_compiled_collation(&my_charset_utf8_latvian_uca_ci);
  add_compiled_collation(&my_charset_utf8_romanian_uca_ci);
  add_compiled_collation(&my_charset_utf8_slovenian_uca_ci);
  add_compiled_collation(&my_charset_utf8_polish_uca_ci);
  add_compiled_collation(&my_charset_utf8_estonian_uca_ci);
  add_compiled_collation(&my_charset_utf8_spanish_uca_ci);
  add_compiled_collation(&my_charset_utf8_swedish_uca_ci);
  add_compiled_collation(&my_charset_utf8_turkish_uca_ci);
  add_compiled_collation(&my_charset_utf8_czech_uca_ci);
  add_compiled_collation(&my_charset_utf8_danish_uca_ci);
  add_compiled_collation(&my_charset_utf8_lithuanian_uca_ci);
  add_compiled_collation(&my_charset_utf8_slovak_uca_ci);
  add_compiled_collation(&my_charset_utf8_spanish2_uca_ci);
  add_compiled_collation(&my_charset_utf8_roman_uca_ci);
  add_compiled_collation(&my_charset_utf8_persian_uca_ci);
  add_compiled_collation(&my_charset_utf8_esperanto_uca_ci);
  add_compiled_collation(&my_charset_utf8_hungarian_uca_ci);
  add_compiled_collation(&my_charset_utf8_croatian_uca_ci);
  add_compiled_collation(&my_charset_utf8_sinhala_uca_ci);
  add_compiled_collation(&my_charset_utf8_unicode_520_ci);
  add_compiled_collation(&my_charset_utf8_vietnamese_ci);

  // utf8mb4 is the only character set with more than two binary collations. For
  // backward compatibility, we want the deprecated BINARY type attribute to use
  // utf8mb4_bin, and not the newer utf8mb4_0900_bin collation, for the utf8mb4
  // character set. That is, the following column definition should result in a
  // column with utf8mb4_bin collation:
  //
  //    col_name VARCHAR(10) CHARSET utf8mb4 BINARY
  //
  // Since add_compiled_collation() records the last binary collation added for
  // a given character set as the binary collation of that character set (stored
  // in cs_name_bin_num_map), we add utf8mb4_bin after utf8mb4_0900_bin to make
  // it the preferred binary collation of utf8mb4.
  add_compiled_collation(&my_charset_utf8mb4_0900_bin);
  add_compiled_collation(&my_charset_utf8mb4_bin);

  add_compiled_collation(&my_charset_utf8mb4_general_ci);
  add_compiled_collation(&my_charset_utf8mb4_unicode_ci);
  add_compiled_collation(&my_charset_utf8mb4_german2_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_icelandic_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_latvian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_romanian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_slovenian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_polish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_estonian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_spanish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_swedish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_turkish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_czech_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_danish_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_lithuanian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_slovak_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_spanish2_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_roman_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_persian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_esperanto_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_hungarian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_croatian_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_sinhala_uca_ci);
  add_compiled_collation(&my_charset_utf8mb4_unicode_520_ci);
  add_compiled_collation(&my_charset_utf8mb4_vietnamese_ci);
  add_compiled_collation(&my_charset_utf8mb4_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_de_pb_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_is_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_lv_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_ro_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_sl_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_pl_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_et_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_es_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_sv_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_tr_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_cs_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_da_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_lt_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_sk_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_es_trad_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_la_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_eo_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_hu_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_hr_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_vi_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_ru_0900_ai_ci);
  add_compiled_collation(&my_charset_utf8mb4_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_de_pb_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_is_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_lv_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_ro_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_sl_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_pl_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_et_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_es_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_sv_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_tr_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_cs_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_da_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_lt_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_sk_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_es_trad_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_la_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_eo_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_hu_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_hr_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_vi_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_ja_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_ja_0900_as_cs_ks);
  add_compiled_collation(&my_charset_utf8mb4_0900_as_ci);
  add_compiled_collation(&my_charset_utf8mb4_ru_0900_as_cs);
  add_compiled_collation(&my_charset_utf8mb4_zh_0900_as_cs);

  add_compiled_collation(&my_charset_utf16_general_ci);
  add_compiled_collation(&my_charset_utf16_bin);
  add_compiled_collation(&my_charset_utf16le_general_ci);
  add_compiled_collation(&my_charset_utf16le_bin);
  add_compiled_collation(&my_charset_utf16_unicode_ci);
  add_compiled_collation(&my_charset_utf16_german2_uca_ci);
  add_compiled_collation(&my_charset_utf16_icelandic_uca_ci);
  add_compiled_collation(&my_charset_utf16_latvian_uca_ci);
  add_compiled_collation(&my_charset_utf16_romanian_uca_ci);
  add_compiled_collation(&my_charset_utf16_slovenian_uca_ci);
  add_compiled_collation(&my_charset_utf16_polish_uca_ci);
  add_compiled_collation(&my_charset_utf16_estonian_uca_ci);
  add_compiled_collation(&my_charset_utf16_spanish_uca_ci);
  add_compiled_collation(&my_charset_utf16_swedish_uca_ci);
  add_compiled_collation(&my_charset_utf16_turkish_uca_ci);
  add_compiled_collation(&my_charset_utf16_czech_uca_ci);
  add_compiled_collation(&my_charset_utf16_danish_uca_ci);
  add_compiled_collation(&my_charset_utf16_lithuanian_uca_ci);
  add_compiled_collation(&my_charset_utf16_slovak_uca_ci);
  add_compiled_collation(&my_charset_utf16_spanish2_uca_ci);
  add_compiled_collation(&my_charset_utf16_roman_uca_ci);
  add_compiled_collation(&my_charset_utf16_persian_uca_ci);
  add_compiled_collation(&my_charset_utf16_esperanto_uca_ci);
  add_compiled_collation(&my_charset_utf16_hungarian_uca_ci);
  add_compiled_collation(&my_charset_utf16_croatian_uca_ci);
  add_compiled_collation(&my_charset_utf16_sinhala_uca_ci);
  add_compiled_collation(&my_charset_utf16_unicode_520_ci);
  add_compiled_collation(&my_charset_utf16_vietnamese_ci);

  add_compiled_collation(&my_charset_utf32_general_ci);
  add_compiled_collation(&my_charset_utf32_bin);
  add_compiled_collation(&my_charset_utf32_unicode_ci);
  add_compiled_collation(&my_charset_utf32_german2_uca_ci);
  add_compiled_collation(&my_charset_utf32_icelandic_uca_ci);
  add_compiled_collation(&my_charset_utf32_latvian_uca_ci);
  add_compiled_collation(&my_charset_utf32_romanian_uca_ci);
  add_compiled_collation(&my_charset_utf32_slovenian_uca_ci);
  add_compiled_collation(&my_charset_utf32_polish_uca_ci);
  add_compiled_collation(&my_charset_utf32_estonian_uca_ci);
  add_compiled_collation(&my_charset_utf32_spanish_uca_ci);
  add_compiled_collation(&my_charset_utf32_swedish_uca_ci);
  add_compiled_collation(&my_charset_utf32_turkish_uca_ci);
  add_compiled_collation(&my_charset_utf32_czech_uca_ci);
  add_compiled_collation(&my_charset_utf32_danish_uca_ci);
  add_compiled_collation(&my_charset_utf32_lithuanian_uca_ci);
  add_compiled_collation(&my_charset_utf32_slovak_uca_ci);
  add_compiled_collation(&my_charset_utf32_spanish2_uca_ci);
  add_compiled_collation(&my_charset_utf32_roman_uca_ci);
  add_compiled_collation(&my_charset_utf32_persian_uca_ci);
  add_compiled_collation(&my_charset_utf32_esperanto_uca_ci);
  add_compiled_collation(&my_charset_utf32_hungarian_uca_ci);
  add_compiled_collation(&my_charset_utf32_croatian_uca_ci);
  add_compiled_collation(&my_charset_utf32_sinhala_uca_ci);
  add_compiled_collation(&my_charset_utf32_unicode_520_ci);
  add_compiled_collation(&my_charset_utf32_vietnamese_ci);

  /* Copy compiled charsets */
  for (cs = compiled_charsets; cs->name; cs++) add_compiled_collation(cs);

  return false;
}
