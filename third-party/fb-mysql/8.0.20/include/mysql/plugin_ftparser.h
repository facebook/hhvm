/* Copyright (c) 2005, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef _my_plugin_ftparser_h
#define _my_plugin_ftparser_h

/**
  @file include/mysql/plugin_ftparser.h
*/

/*************************************************************************
  API for Full-text parser plugin. (MYSQL_FTPARSER_PLUGIN)
*/

#ifndef MYSQL_SERVER
#include "plugin.h"
#endif

/* Parsing modes. Set in  MYSQL_FTPARSER_PARAM::mode */
enum enum_ftparser_mode {
  /*
    Fast and simple mode.  This mode is used for indexing, and natural
    language queries.

    The parser is expected to return only those words that go into the
    index. Stopwords or too short/long words should not be returned. The
    'boolean_info' argument of mysql_add_word() does not have to be set.
  */
  MYSQL_FTPARSER_SIMPLE_MODE = 0,

  /*
    Parse with stopwords mode.  This mode is used in boolean searches for
    "phrase matching."

    The parser is not allowed to ignore words in this mode.  Every word
    should be returned, including stopwords and words that are too short
    or long.  The 'boolean_info' argument of mysql_add_word() does not
    have to be set.
  */
  MYSQL_FTPARSER_WITH_STOPWORDS = 1,

  /*
    Parse in boolean mode.  This mode is used to parse a boolean query string.

    The parser should provide a valid MYSQL_FTPARSER_BOOLEAN_INFO
    structure in the 'boolean_info' argument to mysql_add_word().
    Usually that means that the parser should recognize boolean operators
    in the parsing stream and set appropriate fields in
    MYSQL_FTPARSER_BOOLEAN_INFO structure accordingly.  As for
    MYSQL_FTPARSER_WITH_STOPWORDS mode, no word should be ignored.
    Instead, use FT_TOKEN_STOPWORD for the token type of such a word.
  */
  MYSQL_FTPARSER_FULL_BOOLEAN_INFO = 2
};

/*
  Token types for boolean mode searching (used for the type member of
  MYSQL_FTPARSER_BOOLEAN_INFO struct)

  FT_TOKEN_EOF: End of data.
  FT_TOKEN_WORD: Regular word.
  FT_TOKEN_LEFT_PAREN: Left parenthesis (start of group/sub-expression).
  FT_TOKEN_RIGHT_PAREN: Right parenthesis (end of group/sub-expression).
  FT_TOKEN_STOPWORD: Stopword.
*/

enum enum_ft_token_type {
  FT_TOKEN_EOF = 0,
  FT_TOKEN_WORD = 1,
  FT_TOKEN_LEFT_PAREN = 2,
  FT_TOKEN_RIGHT_PAREN = 3,
  FT_TOKEN_STOPWORD = 4
};

/*
  This structure is used in boolean search mode only. It conveys
  boolean-mode metadata to the MySQL search engine for every word in
  the search query. A valid instance of this structure must be filled
  in by the plugin parser and passed as an argument in the call to
  mysql_add_word (the callback function in the MYSQL_FTPARSER_PARAM
  structure) when a query is parsed in boolean mode.

  type: The token type.  Should be one of the enum_ft_token_type values.

  yesno: Whether the word must be present for a match to occur:
    >0 Must be present
    <0 Must not be present
    0  Neither; the word is optional but its presence increases the relevance
  With the default settings of the ft_boolean_syntax system variable,
  >0 corresponds to the '+' operator, <0 corrresponds to the '-' operator,
  and 0 means neither operator was used.

  weight_adjust: A weighting factor that determines how much a match
  for the word counts.  Positive values increase, negative - decrease the
  relative word's importance in the query.

  wasign: The sign of the word's weight in the query. If it's non-negative
  the match for the word will increase document relevance, if it's
  negative - decrease (the word becomes a "noise word", the less of it the
  better).

  trunc: Corresponds to the '*' operator in the default setting of the
  ft_boolean_syntax system variable.

  position: Start position in bytes of the word in the document, used by InnoDB
  FTS.
*/

struct MYSQL_FTPARSER_BOOLEAN_INFO {
  enum enum_ft_token_type type;
  int yesno;
  int weight_adjust;
  char wasign;
  char trunc;
  int position;
  /* These are parser state and must be removed. */
  char prev;
  char *quot;
};

/*
  The following flag means that buffer with a string (document, word)
  may be overwritten by the caller before the end of the parsing (that is
  before st_mysql_ftparser::deinit() call). If one needs the string
  to survive between two successive calls of the parsing function, she
  needs to save a copy of it. The flag may be set by MySQL before calling
  st_mysql_ftparser::parse(), or it may be set by a plugin before calling
  MYSQL_FTPARSER_PARAM::mysql_parse() or
  MYSQL_FTPARSER_PARAM::mysql_add_word().
*/
#define MYSQL_FTFLAGS_NEED_COPY 1

/*
  An argument of the full-text parser plugin. This structure is
  filled in by MySQL server and passed to the parsing function of the
  plugin as an in/out parameter.

  mysql_parse: A pointer to the built-in parser implementation of the
  server. It's set by the server and can be used by the parser plugin
  to invoke the MySQL default parser.  If plugin's role is to extract
  textual data from .doc, .pdf or .xml content, it might extract
  plaintext from the content, and then pass the text to the default
  MySQL parser to be parsed.

  mysql_add_word: A server callback to add a new word.  When parsing
  a document, the server sets this to point at a function that adds
  the word to MySQL full-text index.  When parsing a search query,
  this function will add the new word to the list of words to search
  for.  The boolean_info argument can be NULL for all cases except
  when mode is MYSQL_FTPARSER_FULL_BOOLEAN_INFO.

  ftparser_state: A generic pointer. The plugin can set it to point
  to information to be used internally for its own purposes.

  mysql_ftparam: This is set by the server.  It is used by MySQL functions
  called via mysql_parse() and mysql_add_word() callback.  The plugin
  should not modify it.

  cs: Information about the character set of the document or query string.

  doc: A pointer to the document or query string to be parsed.

  length: Length of the document or query string, in bytes.

  flags: See MYSQL_FTFLAGS_* constants above.

  mode: The parsing mode.  With boolean operators, with stopwords, or
  nothing.  See  enum_ftparser_mode above.
*/

struct MYSQL_FTPARSER_PARAM {
  int (*mysql_parse)(MYSQL_FTPARSER_PARAM *, char *doc, int doc_len);
  int (*mysql_add_word)(MYSQL_FTPARSER_PARAM *, char *word, int word_len,
                        MYSQL_FTPARSER_BOOLEAN_INFO *boolean_info);
  void *ftparser_state;
  void *mysql_ftparam;
  const CHARSET_INFO *cs;
  char *doc;
  int length;
  int flags;
  enum enum_ftparser_mode mode;
};

/*
  Full-text parser descriptor.

  interface_version is, e.g., MYSQL_FTPARSER_INTERFACE_VERSION.
  The parsing, initialization, and deinitialization functions are
  invoked per SQL statement for which the parser is used.
*/

struct st_mysql_ftparser {
  int interface_version;
  int (*parse)(MYSQL_FTPARSER_PARAM *param);
  int (*init)(MYSQL_FTPARSER_PARAM *param);
  int (*deinit)(MYSQL_FTPARSER_PARAM *param);
};

#endif
