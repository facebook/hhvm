/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_PHP_MAILPARSE_MIME_H_
#define incl_HPHP_PHP_MAILPARSE_MIME_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/mailparse/rfc822.h"
#include "hphp/runtime/base/string-buffer.h"

extern "C" {
#include "mbfl/mbfl_convert.h"
#include "mbfl/mbfilter.h"
}

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class MimePart : public ResourceData {
public:
  enum Decode {
    DecodeNone      = 0,  /* include headers but no section */
    Decode8Bit      = 1,  /* decode body into 8-bit */
    DecodeNoHeaders = 2,  /* don't include the headers */
    DecodeNoBody    = 4,  /* don't include the body */
  };

  static bool ProcessLine(MimePart *workpart, CStrRef line);

public:
  DECLARE_RESOURCE_ALLOCATION(MimePart);

  MimePart();

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassNameHook() const { return s_class_name; }

  bool parse(const char *buf, int bufsize);
  Variant extract(CVarRef filename, CVarRef callbackfunc, int decode,
                  bool isfile);
  Variant getPartData();
  Array getStructure();
  Resource findByName(const char *name);

  bool isVersion1();
  int filter(int c);

private:
  class MimeHeader {
  public:
    MimeHeader();
    explicit MimeHeader(const char *value);
    explicit MimeHeader(php_rfc822_tokenized_t *toks);

    bool empty() const { return m_empty;}
    void clear();

    Variant get(CStrRef attrname);
    void getAll(Array &ret, CStrRef valuelabel, CStrRef attrprefix);

    bool m_empty;
    String m_value;
    Array m_attributes;

  private:
    void rfc2231_to_mime(StringBuffer &value_buf, char* value,
                         int charset_p, int prevcharset_p);
  };

private:
  static void UpdatePositions(MimePart *part, int newendpos,
                              int newbodyend, int deltanlines);

  Resource m_parent;
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
  typedef void (MimePart::*PFN_CALLBACK)(CStrRef);
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
    Resource lastpart;
  } m_parsedata;

  int extractImpl(int decode, File *src);
  MimePart *createChild(int startpos, bool inherit);
  bool processHeader();
  MimePart *getParent();

  void decoderPrepare(bool do_decode);
  void decoderFeed(CStrRef str);
  void decoderFinish();

  // extract callbacks
  void callUserFunc(CStrRef s);
  void outputToStdout(CStrRef s);
  void outputToFile(CStrRef s);
  void outputToString(CStrRef s);

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

#endif // incl_HPHP_PHP_MAILPARSE_MIME_H_
