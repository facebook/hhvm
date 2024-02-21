/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include <iostream>
#include <stack>

#include <folly/json/json.h>
#include <folly/Range.h>
#include <folly/String.h>

#include "hphp/runtime/base/runtime-option.h"

namespace HPHP{
///////////////////////////////////////////////////////////////////////////////
// writers

struct Writer {

  enum class Format {
    XML,
    JSON,
    KVP,
    HTML
  };

  explicit Writer(std::ostream &out) : m_out(out), m_indent(0) {}
  virtual ~Writer() {}

  virtual void writeFileHeader() = 0;
  virtual void writeFileFooter() = 0;

  // Begins writing an object which is different than a list in JSON.
  virtual void beginObject(const char *name) = 0;

  // Begins writing a list (an ordered set of potentially unnamed children)
  // Defaults to beginning an object.
  virtual void beginList(const char *name) {
    beginObject(name);
  }

  // Writes a string value with a given name.
  virtual void writeEntry(const char *name, const std::string &value) = 0;

  // Writes a string value with a given name.
  virtual void writeEntry(const char *name, int64_t value) = 0;

  // Ends the writing of an object.
  virtual void endObject(const char *name) = 0;

  // Ends the writing of a list. Defaults to simply ending an Object.
  virtual void endList(const char *name) {
    endObject(name);
  }

  static std::string escape_for_json(const char* s) {
    auto ret = std::string{};
    auto opts = folly::json::serialization_opts{};
    opts.skip_invalid_utf8 = true;
    opts.encode_non_ascii = true;
    // NB: escapeString prepends and appends double quotes
    folly::json::escapeString(folly::StringPiece{s}, ret, opts);
    return ret;
  }

protected:
  std::ostream &m_out;
  int m_indent;

  virtual void writeIndent() {
    for (int i = 0; i < m_indent; i++) {
      m_out << "  ";
    }
  }
};

struct XMLWriter : Writer {
  explicit XMLWriter(std::ostream &out) : Writer(out) {}

  void writeFileHeader() override {
    m_out << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    if (!RuntimeOption::StatsXSL.empty()) {
      m_out << "<?xml-stylesheet type=\"text/xsl\" href=\""
            << RuntimeOption::StatsXSL << "\"?>\n";
    } else if (!RuntimeOption::StatsXSLProxy.empty()) {
      m_out << "<?xml-stylesheet type=\"text/xsl\" href=\"stats.xsl\"?>\n";
    }
  }

  void writeFileFooter() override {}

  /**
   * In XML/HTML there is no distinction between creating a list, and creating
   * an object with keyed attributes. (unlike the JSON format).
   */
  void beginObject(const char* name) override {
    writeIndent();
    m_out << '<' << name << ">\n";
    ++m_indent;
  }

  void endObject(const char* name) override {
    --m_indent;
    writeIndent();
    m_out << "</" << name << ">\n";
  }

  void writeEntry(const char* name, const std::string& value) override {
    writeIndent();
    m_out << "<entry><key>" << Escape(name) << "</key>";
    m_out << "<value>" << Escape(value.c_str()) << "</value></entry>\n";
  }

  void writeEntry(const char* name, int64_t value) override {
    writeIndent();
    m_out << "<entry><key>" << Escape(name) << "</key>";
    m_out << "<value>" << value << "</value></entry>\n";
  }

private:
  static std::string Escape(const char *s) {
    std::string ret;
    for (const char *p = s; *p; p++) {
      switch (*p) {
      case '<':  ret += "&lt;";  break;
      case '&':  ret += "&amp;"; break;
      default:   ret += *p;      break;
      }
    }
    return ret;
  }
};

struct JSONWriter : Writer {

protected:
  // If true it will generate human-friendly output on multiple lines and with
  // tabs for indentation. If false the output will be in a single line with no
  // extra spacing.
  bool m_prettify;

  // Whether or not we have skiped a comma for this current indent level. Valid
  // json may not have trailing commas such as {"a":4, "b":5,} Since we are
  // writing to a stream, we output *valid* json that has commas preceding all
  // elements except the first, which is equivalent to outputing commas after
  // each element except the last. Skip the preceding comma when m_justIndented.
  bool m_justIndented;

  // Stack that determines whether or not at a given object depth level, we are
  // to be listing child objects with keyed entries. For example, in Json,
  // inside an array, entries are not keyed. Also at the json root node, we
  // begin at a nameless context.
  std::stack<bool> m_namelessContextStack;

  virtual void increaseIndent() {
    ++m_indent;
    m_justIndented = true;
  }

  /**
   * It is *important* to set m_justIndented to false here in the event that we
   * write objects with *no* members, we need to set it to false.
   */
  virtual void decreaseIndent() {
    --m_indent;
    m_justIndented = false;

    // We should never pop off more than we pushed, but just in case someone
    // called too many endObject's etc, we don't want a segfault.
    if (m_namelessContextStack.size() != 0) {
      m_namelessContextStack.pop();
    }
  }

  /**
   * Begins writing a containing entity (such as a list or object).
   * See the 'isList' parameter.
   */
  virtual void beginContainer(const char *name, bool isList) {
    char opener = isList ? '[' : '{';
    beginEntity(name);
    m_out << opener;
    if (m_prettify) {
      m_out << '\n';
    }

    // Push on whether or not we're entering a nameless context
    m_namelessContextStack.push(isList);
    increaseIndent();
  }

  virtual void endContainer(bool isList) {
    char closer = isList ? ']' : '}';
    decreaseIndent();
    if (m_prettify) {
      writeIndent();
    }
    m_out << closer;
    if (m_prettify) {
      m_out << '\n';
    }
  }

  /**
   * Writes any needed leading commas, and keyed name if appropriate.
   * Reduces redundancy. Used whenever an entity is a child of another
   * entity - does all the work of determining if the object should be
   * written with/without a name and if we need a comma.
   */
  virtual void beginEntity(const char *name) {
    if (m_prettify) {
      writeIndent();
    }
    if (!m_justIndented) {
      m_out << ", ";
    }
    if (m_namelessContextStack.size() != 0 && !m_namelessContextStack.top()) {
      m_out << escape_for_json(name) << ": ";
    }
    m_justIndented = false;
  }

public:

  explicit JSONWriter(std::ostream &out, bool prettify=true) : Writer(out),
      m_prettify(prettify), m_justIndented(true) {

    // A valid json object begins in the nameless context. See
    // json.org for JSON state machine.
    m_namelessContextStack.push(true);
  }

  void writeFileHeader() override {
    beginContainer("root", false);
  }

  void writeFileFooter() override {
    endContainer(false);
  }

  void beginObject(const char* name) override {
    beginContainer(name, false);
  }

  void beginList(const char* name) override {
    beginContainer(name, true);
  }

  void endObject(const char* /*name*/) override { endContainer(false); }

  void endList(const char* /*name*/) override { endContainer(true); }

  void writeEntry(const char* name, const std::string& value) override {
    beginEntity(name);

    // Now write the actual value
    m_out << escape_for_json(value.c_str());
    if (m_prettify) {
      m_out << '\n';
    }
  }

  void writeEntry(const char* name, int64_t value) override {
    beginEntity(name);

    // Now write the actual value
    m_out << value;
    if (m_prettify) {
      m_out << '\n';
    }
  }
};

struct HTMLWriter : Writer {
  explicit HTMLWriter(std::ostream &out) : Writer(out) {}

  void writeFileHeader() override {
    m_out << "<!doctype html>\n<html>\n<head>\n"
             "<meta http-equiv=\"content-type\" "
             "content=\"text/html; charset=UTF-8\">\n"
             "<style type=\"text/css\"> table {margin-left:20px} "
             "th {text-align:left}</style>\n"
             "<title>HPHP Stats</title>\n"
             "</head>\n<body>\n<table>\n<tbody>\n";
  }

  void writeFileFooter() override {
    m_out << "</tbody>\n</table>\n</body>\n</html>\n";
  }


  /**
   * In XML/HTML there is no distinction between creating a list, and creating
   * an object with keyed attributes. (unlike the JSON format).
   */
  void beginObject(const char* name) override {
    writeIndent();
    m_out << "<tr id='" << name << "'><td colspan=2>"
          << "<table><tbody><tr><th colspan=2>" << name << "</th></tr>\n";
    ++m_indent;
  }

  void endObject(const char* /*name*/) override {
    --m_indent;
    writeIndent();
    m_out << "</tbody></table></td></tr>\n";
  }

  void writeEntry(const char* name, const std::string& value) override {
    writeIndent();
    m_out << "<tr><td>" << Escape(name) << "</td>";
    m_out << "<td>" << Escape(value.c_str()) << "</td></tr>\n";
  }

  void writeEntry(const char* name, int64_t value) override {
    writeIndent();
    m_out << "<tr><td>" << Escape(name) << "</td>";
    m_out << "<td>" << value << "</td></tr>\n";
  }

private:
  static std::string Escape(const char *s) {
    std::string ret;
    for (const char *p = s; *p; p++) {
      switch (*p) {
      case '<':  ret += "&lt;";  break;
      case '&':  ret += "&amp;"; break;
      default:   ret += *p;      break;
      }
    }
    return ret;
  }
};
}

