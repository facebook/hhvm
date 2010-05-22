/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __PHP_MAILPARSE_MIME_H__
#define __PHP_MAILPARSE_MIME_H__

#include <runtime/base/base_includes.h>
#include <runtime/ext/mailparse/rfc822.h>
#include <runtime/base/util/string_buffer.h>

extern "C" {
#include <mbfl/mbfl_convert.h>
#include <mbfl/mbfilter.h>
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
  DECLARE_OBJECT_ALLOCATION(MimePart);

  MimePart();

  // overriding ResourceData
  virtual const char *o_getClassName() const {
    return "mailparse_mail_structure";
  }

  bool parse(const char *buf, int bufsize);
  Variant extract(CVarRef filename, CVarRef callbackfunc, int decode,
                  bool isfile);
  Variant getPartData();
  Array getStructure();
  Object findByName(const char *name);

  bool isVersion1();
  int filter(int c);

private:
  class MimeHeader {
  public:
    MimeHeader();
    MimeHeader(const char *value);
    MimeHeader(php_rfc822_tokenized_t *toks);

    bool empty() const { return m_empty;}
    void clear();

    Variant get(const char *attrname);
    void getAll(Array &ret, litstr valuelabel, litstr attrprefix);

    bool m_empty;
    String m_value;
    Variant m_attributes;

  private:
    void rfc2231_to_mime(StringBuffer &value_buf, char* value,
                         int charset_p, int prevcharset_p);
  };

private:
  static void UpdatePositions(MimePart *part, int newendpos,
                              int newbodyend, int deltanlines);

  Object m_parent;
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
    Object lastpart;
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

#endif // __PHP_MAILPARSE_MIME_H__
