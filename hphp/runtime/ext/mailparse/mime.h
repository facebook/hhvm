/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/mailparse/rfc822.h"
#include "hphp/runtime/base/string-buffer.h"

extern "C" {
#include <mbfl/mbfl_convert.h>
#include <mbfl/mbfilter.h>
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct MimePart : ResourceData {
  enum Decode {
    DecodeNone      = 0,  /* include headers but no section */
    Decode8Bit      = 1,  /* decode body into 8-bit */
    DecodeNoHeaders = 2,  /* don't include the headers */
    DecodeNoBody    = 4,  /* don't include the body */
  };

  static bool ProcessLine(req::ptr<MimePart> workpart, const String& line);

public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(MimePart);

  MimePart();

  CLASSNAME_IS("mailparse_mail_structure")
  const String& o_getClassNameHook() const override {
    return classnameof();
  }

  bool parse(const char *buf, int bufsize);
  Variant extract(const Variant& filename, const Variant& callbackfunc, int decode,
                  bool isfile);
  Array getPartData();
  Array getStructure();
  OptResource findByName(const char *name);

  bool isVersion1();
  int filter(int c);

private:
  struct MimeHeader {
    MimeHeader();
    explicit MimeHeader(const char *value);
    explicit MimeHeader(php_rfc822_tokenized_t *toks);

    bool empty() const { return m_empty;}
    void clear();

    Variant get(const String& attrname);
    void getAll(Array &ret, const String& valuelabel, const String& attrprefix);

    bool m_empty;
    String m_value;
    Array m_attributes;

  private:
    void rfc2231_to_mime(StringBuffer &value_buf, char* value,
                         int charset_p, int prevcharset_p);
  };

private:
  static void UpdatePositions(req::ptr<MimePart> part, int newendpos,
                              int newbodyend, int deltanlines);

  req::ptr<MimePart> m_parent;
  Array  m_children;   /* child parts */

  int m_startpos, m_endpos;   /* offsets of this part in the message */
  int m_bodystart, m_bodyend; /* offsets of the body content of this part */
  int m_nlines, m_nbodylines; /* number of lines in section/body */

  String m_mime_version;
  String m_content_transfer_encoding;
  String m_content_location;
  String m_content_base;
  String m_boundary;
  String m_charset;

  MimeHeader m_content_type;
  MimeHeader m_content_disposition;

  Array m_headers; /* a record of all the headers */

  /* these are used during part extraction */
  typedef void (MimePart::*PFN_CALLBACK)(const String&);
  PFN_CALLBACK m_extract_func;
  mbfl_convert_filter *m_extract_filter;
  Variant m_extract_context;

  /* these are used during parsing */
  struct {
    bool in_header;
    bool is_dummy;
    bool completed;

    String workbuf;
    String headerbuf;
    req::ptr<MimePart> lastpart;
  } m_parsedata;

  int extractImpl(int decode, req::ptr<File> src);
  req::ptr<MimePart> createChild(int startpos, bool inherit);
  bool processHeader();
  const req::ptr<MimePart>& getParent() { return m_parent; }

  void decoderPrepare(bool do_decode);
  void decoderFeed(const String& str);
  void decoderFinish();

  // extract callbacks
  void callUserFunc(const String& s);
  void outputToStdout(const String& s);
  void outputToFile(const String& s);
  void outputToString(const String& s);

  // enumeration
  struct Enumerator {
    Enumerator *next;
    int id;
  };
  typedef bool (MimePart::*PFUNC_ENUMERATOR)
    (Enumerator *enumerator, void *ptr);
  bool enumeratePartsImpl(Enumerator *top, Enumerator **child,
                          PFUNC_ENUMERATOR callback, void *ptr);
  void enumerateParts(PFUNC_ENUMERATOR callback, void *ptr);

  // enumeration callbacks
  bool findPart(Enumerator *id, void *ptr);
  bool getStructure(Enumerator *id, void *ptr);
};

///////////////////////////////////////////////////////////////////////////////
}
