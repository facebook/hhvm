/*
   Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <unicode/errorcode.h>
#include <unicode/regex.h>
#include <unicode/unistr.h>
#include <cassert>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

#include "lex.h"  // symbols[]
#include "template_utils.h"
#include "welcome_copyright_notice.h"  // ORACLE_WELCOME_COPYRIGHT_NOTICE

using icu::RegexMatcher;
using icu::UnicodeString;

bool icu_error(const UErrorCode &status) {
  if (!U_FAILURE(status)) {
    return false;
  }
  icu::ErrorCode error_code;
  error_code.set(status);
  std::cerr << "Error: " << error_code.errorName() << std::endl;
  return true;
}

int main(int argc, const char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <YACC file>\n", argv[0]);
    return EXIT_FAILURE;
  }

  const char *yacc_filename = argv[1];

  std::ifstream yacc(yacc_filename);
  if (!yacc.is_open()) {
    fprintf(stderr, "Failed to open \"%s\"", yacc_filename);
    return EXIT_FAILURE;
  }

  UErrorCode status = U_ZERO_ERROR;

  UnicodeString rx(
      "^%(token|left|right|nonassoc)[[:space:]]*"  // g.1: %token, %left etc.
      "(<[._[:alnum:]]+>)?[[:space:]]*"            // g.2: opt. '<' type '>'
      "([_[:alnum:]]+)"                            // g.3: token name
      "([[:space:]]+[0-9]+)?"                      // g.4: opt token number
      ".*$");                                      // opt. rest of line
  RegexMatcher match(rx, 0, status);
  if (icu_error(status)) {
    return EXIT_FAILURE;
  }

  std::set<size_t> keyword_tokens;
  std::string s;
  size_t input_line = 0;
  while (getline(yacc, s)) {
    //
    // Collect non-reserved keyword token numbers in keyword_tokens, where
    // non-reserved keyword tokens are declared like this:
    //
    //   %token <lexer.keyword> token_name token_number
    //
    // At the same time, validate if all %token declarations have explicit
    // token numbers.
    //

    input_line++;
    UnicodeString sample(s.data(), s.length());
    match.reset(sample);
    if (match.matches(status)) {
      assert(match.groupCount() == 4);

      //
      // Check if %token definition contains an explicit token number
      //

      UnicodeString uc_declaration = match.group(1, status);
      if (icu_error(status)) {
        return EXIT_FAILURE;
      }

      static const UnicodeString token_declaration("token");
      if (uc_declaration == token_declaration) {
        //
        // This is %token ...
        //

        UnicodeString uc_token_number = match.group(4, status);
        if (icu_error(status)) {
          return EXIT_FAILURE;
        }

        std::string utf8_token_number;
        uc_token_number.toUTF8String(utf8_token_number);
        if (uc_token_number.length() == 0) {
          fprintf(stderr, "%s:%zu: error: missing token number\n",
                  yacc_filename, input_line);
          exit(EXIT_FAILURE);
        }

        UnicodeString uc_semantic_type = match.group(2, status);
        if (icu_error(status)) {
          return EXIT_FAILURE;
        }

        static const UnicodeString keyword_semantic_type("<lexer.keyword>");
        if (uc_semantic_type == keyword_semantic_type) {
          //
          // This is %token <lexer.keyword> ...
          //

          int token_num = std::stoi(utf8_token_number);
          keyword_tokens.insert(token_num);
        }
      }
    }
  }

  std::map<std::string, bool> words;

  for (size_t i = 0; i < array_elements(symbols); i++) {
    const SYMBOL *sym = &symbols[i];

    if (sym->group != SG_KEYWORDS && sym->group != SG_HINTABLE_KEYWORDS)
      continue;  // Function or optimizer hint name.

    if (!isalpha(sym->name[0])) continue;  // Operator.

    bool is_reserved = keyword_tokens.count(sym->tok) == 0;
    if (!words.insert(std::make_pair(sym->name, is_reserved)).second) {
      fprintf(stderr,
              "This should not happen: \"%s\" has duplicates."
              " See symbols[] in lex.h",
              sym->name);
      assert(false);
      return EXIT_FAILURE;
    }
  }

  auto &out = std::cout;

  out << ORACLE_GPL_COPYRIGHT_NOTICE("2017") << std::endl;

  out << "#ifndef GEN_KEYWORD_LIST_H__INCLUDED\n";
  out << "#define GEN_KEYWORD_LIST_H__INCLUDED\n\n";
  out << "/*\n";
  out << "  This file is generated, do not edit.\n";
  out << "  See file sql/gen_keyword_list.cc.\n";
  out << "*/\n\n";

  out << "typedef struct { const char *word; int reserved; } keyword_t;\n\n";

  out << "static const keyword_t keyword_list[]= {\n";
  for (auto p : words)
    out << "  { \"" << p.first << "\", " << (p.second ? 1 : 0) << " },\n";
  out << "};/*keyword_list*/\n\n";

  out << "#endif/*GEN_KEYWORD_LIST_H__INCLUDED*/\n";

  return 0;
}
