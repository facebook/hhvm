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

#include <runtime/ext/mailparse/mime.h>
#include <runtime/ext/ext_stream.h>
#include <runtime/base/file/mem_file.h>
#include <runtime/base/runtime_error.h>

#define MAXLEVELS  20
#define MAXPARTS  300

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

MimePart::MimeHeader::MimeHeader()
  : m_empty(true) {
}

MimePart::MimeHeader::MimeHeader(const char *value)
  : m_empty(false) {
  ASSERT(value);
  m_attributes = Array::Create();
  m_value = String(value, CopyString);
}

MimePart::MimeHeader::MimeHeader(php_rfc822_tokenized_t *toks)
  : m_empty(false) {
  int i, first_semi, next_semi, comments_before_semi, netscape_bug = 0;
  String name_buf;
  StringBuffer value_buf;
  bool is_rfc2231_name = false;
  char *check_name;
  int charset_p = 0, prevcharset_p = 0;
  bool namechanged = false, currentencoded = false;

  m_attributes = Array::Create();

  /* php_rfc822_print_tokens(toks); */
  /* look for optional ; which separates optional attributes from the main
     value */
  for (first_semi = 2; first_semi < toks->ntokens; first_semi++)
    if (toks->tokens[first_semi].token == ';') break;

  m_value = String(php_rfc822_recombine_tokens
                   (toks, 2, first_semi - 2,
                    PHP_RFC822_RECOMBINE_STRTOLOWER |
                    PHP_RFC822_RECOMBINE_IGNORE_COMMENTS), AttachString);

  if (first_semi < toks->ntokens) first_semi++;

  /* Netscape Bug: Messenger sometimes omits the semi when wrapping the
     * the header.
   * That means we have to be even more clever than the spec says that
   * we need to :-/
   * */
  while (first_semi < toks->ntokens) {
    /* find the next ; */
    comments_before_semi = 0;
    for (next_semi = first_semi; next_semi < toks->ntokens; next_semi++) {
      if (toks->tokens[next_semi].token == ';') break;
      if (toks->tokens[next_semi].token == '(') comments_before_semi++;
    }

    i = first_semi;
    if (i < next_semi) {
      i++;

      /* ignore comments */
      while (i < next_semi && toks->tokens[i].token == '(') {
        i++;
      }

      if (i < next_semi && toks->tokens[i].token == '=') {
        /* Here, next_semi --> "name" and i --> "=", so skip "=" sign */
        i++;

        /* count those tokens; we expect "token = token" (3 tokens); if there
         * are more than that, then something is quite possibly wrong
         * - Netscape Bug! */
        if (next_semi < toks->ntokens && toks->tokens[next_semi].token != ';'
            && next_semi - first_semi - comments_before_semi > 3) {
          next_semi = i + 1;
          netscape_bug = 1;
        }

        String name(php_rfc822_recombine_tokens
                    (toks, first_semi, 1,
                     PHP_RFC822_RECOMBINE_STRTOLOWER|
                     PHP_RFC822_RECOMBINE_IGNORE_COMMENTS), AttachString);
        String value(php_rfc822_recombine_tokens
                     (toks, i, next_semi - i,
                      PHP_RFC822_RECOMBINE_IGNORE_COMMENTS), AttachString);

        /* support rfc2231 mime parameter value
         *
         * Parameter Value Continuations:
         *
         * Content-Type: message/external-body; access-type=URL;
         *  URL*0="ftp://";
         *  URL*1="cs.utk.edu/pub/moore/bulk-mailer/bulk-mailer.tar"
         *
         * is semantically identical to
         *
         * Content-Type: message/external-body; access-type=URL;
         *  URL="ftp://cs.utk.edu/pub/moore/bulk-mailer/bulk-mailer.tar"
         *
         * Original rfc2231 support by IceWarp Ltd. <info@icewarp.com>
         */
        check_name = const_cast<char*>(strchr(name.data(), '*'));
        if (check_name) {
          currentencoded = true;

          /* Is last char * - charset encoding */
          charset_p = (name[name.size() -1] == '*');

          /* Leave only attribute name without * */
          *check_name = 0;

          /* New item or continuous */
          if (name_buf.isNull()) {
            namechanged = false;
            name_buf = name;
          } else {
            namechanged = (name_buf != name);
            if (!namechanged) {
              name.clear();
            }
          }

          /* Check if name changed*/
          if (!namechanged) {

            /* Append string to buffer - check if to be encoded...  */
            rfc2231_to_mime(value_buf, (char*)value.data(), charset_p,
                            prevcharset_p);

            /* Mark previous */
            prevcharset_p = charset_p;
          }

          is_rfc2231_name = true;
        }

        /* Last item was encoded  */
        if (is_rfc2231_name) {
          /* Name not null and name differs with new name*/
          if (!name.empty() && name_buf != name) {
            /* Finalize packet */
            rfc2231_to_mime(value_buf, NULL, 0, prevcharset_p);

            m_attributes.set(name_buf, value_buf.detach());
            value_buf.reset();

            prevcharset_p = 0;
            is_rfc2231_name = false;
            name_buf.clear();

            /* New non encoded name*/
            if (!currentencoded) {
              /* Add string*/
              m_attributes.set(name, value);
            } else {    /* Encoded name changed*/
              if (namechanged) {
                /* Append string to buffer - check if to be encoded...  */
                rfc2231_to_mime(value_buf, (char*)value.data(), charset_p,
                                prevcharset_p);

                /* Mark */
                is_rfc2231_name = true;
                name_buf = name;
                prevcharset_p = charset_p;
              }
            }

            namechanged = false;
          }
        } else {
          m_attributes.set(name, value);
        }
      }
    }

    if (next_semi < toks->ntokens && !netscape_bug) {
      next_semi++;
    }

    first_semi = next_semi;
    netscape_bug = 0;
  }

  if (is_rfc2231_name) {
    /* Finalize packet */
    rfc2231_to_mime(value_buf, NULL, 0, prevcharset_p);
    m_attributes.set(name_buf, value_buf.detach());
  }
}

void MimePart::MimeHeader::clear() {
  m_empty = true;
  m_value.clear();
  m_attributes.reset();
}

Variant MimePart::MimeHeader::get(const char *attrname) {
  return m_attributes[attrname];
}

void MimePart::MimeHeader::getAll(Array &ret, litstr valuelabel,
                                  litstr attrprefix) {
  for (ArrayIter iter(m_attributes); iter; ++iter) {
    ret.set(String(attrprefix) + iter.first().toString(), iter.second());
  }

  /* do this last so that a bogus set of headers like this:
   * Content-Type: multipart/related;
   *    boundary="----=_NextPart_00_0017_01C091F4.1B5EF6B0";
   *    type="text/html"
   *
   * doesn't overwrite content-type with the type="text/html"
   * value.
   * */
  ret.set(valuelabel, m_value);
}

void MimePart::MimeHeader::rfc2231_to_mime(StringBuffer &value_buf,
                                           char* value,
                                           int charset_p, int prevcharset_p) {
  char *strp, *startofvalue = NULL;
  int quotes = 0;

  /* Process string, get positions and replace  */
  /* Set to start of buffer*/
  if (charset_p) {

    /* Previous charset already set so only convert %nn to =nn*/
    if (prevcharset_p) quotes=2;

    strp = value;
    while (*strp) {

      /* Quote handling*/
      if (*strp == '\'') {
        if (quotes <= 1) {

          /* End of charset*/
          if (quotes == 0) {
            *strp=0;
          } else {
            startofvalue = strp+1;
          }

          quotes++;
        }
      } else {
        /* Replace % with = - quoted printable*/
        if (*strp == '%' && quotes==2) {
          *strp = '=';
        }
      }
      strp++;
    }
  }

  /* If first encoded token*/
  if (charset_p && !prevcharset_p && startofvalue) {
    value_buf += "=?";
    value_buf += value;
    value_buf += "?Q?";
    value_buf += startofvalue;
  }

  /* If last encoded token*/
  if (prevcharset_p && !charset_p) {
    value_buf += "?=";
  }

  /* Append value*/
  if ((!charset_p || (prevcharset_p && charset_p)) && value) {
    value_buf += value;
  }
}

///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJECT_ALLOCATION(MimePart);

MimePart::MimePart()
  : m_startpos(0), m_endpos(0), m_bodystart(0), m_bodyend(0),
    m_nlines(0), m_nbodylines(0) {
  m_headers = Array::Create();

  /* begin in header parsing mode */
  m_parsedata.in_header = true;
  m_parsedata.is_dummy = false;
  m_parsedata.completed = false;
}

///////////////////////////////////////////////////////////////////////////////
// enumeration

bool MimePart::enumeratePartsImpl(Enumerator *top, Enumerator **child,
                                  PFUNC_ENUMERATOR callback, void *ptr) {
  *child = NULL;
  if (!(this->*callback)(top, ptr)) return false;

  Enumerator next;
  *child = &next;
  next.id = 1;
  if (!strncasecmp(m_content_type.m_value.data(), "multipart/", 10)) {
    next.id = 0;
  }

  for (ArrayIter iter(m_children); iter; ++iter) {
    if (next.id) {
      MimePart *childpart = iter.second().toObject().getTyped<MimePart>();
      if (!childpart->enumeratePartsImpl(top, &next.next, callback, ptr)) {
        return false;
      }
    }
    next.id++;
  }
  return true;
}

void MimePart::enumerateParts(PFUNC_ENUMERATOR callback, void *ptr) {
  Enumerator top;
  top.id = 1;
  enumeratePartsImpl(&top, &top.next, callback, ptr);
}

struct find_part_struct {
  const char *searchfor;
  MimePart *foundpart;
};

bool MimePart::getStructure(Enumerator *id, void *ptr) {
  char intbuf[16];
  int len, i = 0;
  int buf_size = 1024;
  char *buf = (char*)malloc(buf_size);
  buf[0] = '\0';
  while (id && i < buf_size) {
    sprintf(intbuf, "%d", id->id);
    len = strlen(intbuf);
    if (len > (buf_size-i)) {
      raise_warning("too many nested sections in message");
      free(buf);
      return false;
    }
    if ((i + len + 1) >= buf_size) {
      buf_size = buf_size << 1;
      buf = (char*)realloc(buf, buf_size);
      if (!buf) {
        throw FatalErrorException("The structure buffer has been exceeded "
                                  "(%d).  Please try decreasing the nesting "
                                  "depth of messages and report this to the "
                                  "developers.", buf_size);
      }
    }
    sprintf(&buf[i], "%s%c", intbuf, id->next ? '.' : '\0');
    i += len + (id->next ? 1 : 0);
    id = id->next;
  }
  ((Array*)ptr)->append(String(buf, AttachString));
  return true;
}

Array MimePart::getStructure() {
  Array ret = Array::Create();
  enumerateParts(&MimePart::getStructure, &ret);
  return ret;
}

bool MimePart::findPart(Enumerator *id, void *ptr) {
  struct find_part_struct *find = (find_part_struct *)ptr;
  const unsigned char *num = (const unsigned char*)find->searchfor;
  unsigned int n;

  while (id)  {
    if (!isdigit((int)*num)) return true;
    /* convert from decimal to int */
    n = 0;
    while (isdigit((int)*num)) n = (n * 10) + (*num++ - '0');
    if (*num)  {
      if (*num != '.') return true;
      num++;
    }
    if (n != (unsigned int)id->id) return true;
    id = id->next;
  }
  if (*num == 0) find->foundpart = this;
  return true;
}

Object MimePart::findByName(const char *name) {
  struct find_part_struct find;
  find.searchfor = name;
  find.foundpart = NULL;
  enumerateParts(&MimePart::findPart, &find);
  return find.foundpart;
}

static int filter_into_work_buffer(int c, void *dat) {
  MimePart *part = (MimePart*)dat;
  return part->filter(c);
}

int MimePart::filter(int c) {
  m_parsedata.workbuf += (char)c;
  if (m_parsedata.workbuf.size() >= 4096) {
    (this->*m_extract_func)(m_parsedata.workbuf);
    m_parsedata.workbuf.clear();
  }
  return c;
}

void MimePart::decoderPrepare(bool do_decode) {
  enum mbfl_no_encoding from = mbfl_no_encoding_8bit;

  if (do_decode && !m_content_transfer_encoding.empty()) {
    from = mbfl_name2no_encoding(m_content_transfer_encoding.data());
    if (from == mbfl_no_encoding_invalid) {
      if (strcasecmp("binary", m_content_transfer_encoding.data()) != 0) {
        raise_warning("mbstring doesn't know how to decode %s "
                      "transfer encoding!",
                      m_content_transfer_encoding.data());
      }
      from = mbfl_no_encoding_8bit;
    }
  }

  m_parsedata.workbuf.clear();

  if (do_decode) {
    if (from == mbfl_no_encoding_8bit || from == mbfl_no_encoding_7bit) {
      m_extract_filter = NULL;
    } else {
      m_extract_filter = mbfl_convert_filter_new(from, mbfl_no_encoding_8bit,
                                                 filter_into_work_buffer,
                                                 NULL, this);
    }
  }
}

void MimePart::decoderFinish() {
  if (m_extract_filter) {
    mbfl_convert_filter_flush(m_extract_filter);
    mbfl_convert_filter_delete(m_extract_filter);
  }
  if (m_extract_func && !m_parsedata.workbuf.empty()) {
    (this->*m_extract_func)(m_parsedata.workbuf);
    m_parsedata.workbuf.clear();
  }
}

void MimePart::decoderFeed(CStrRef str) {
  if (!str.empty()) {
    if (m_extract_filter) {
      for (int i = 0; i < str.size(); i++) {
        if (mbfl_convert_filter_feed(str[i], m_extract_filter) < 0) {
          raise_warning("filter conversion failed. Input message is "
                        "probably incorrectly encoded");
          return;
        }
      }
    } else {
      (this->*m_extract_func)(str);
    }
  }
}

bool MimePart::isVersion1() {
  return m_mime_version == "1.0" || !m_parent.isNull();
}

MimePart *MimePart::getParent() {
  return m_parent.getTyped<MimePart>();
}

Variant MimePart::getPartData() {
  Array ret = Array::Create();

  ret.set("headers", m_headers);

  ret.set("starting-pos", m_startpos);
  ret.set("starting-pos-body", m_bodystart);
  if (m_parent.isNull()) {
    ret.set("ending-pos", m_endpos);
    ret.set("ending-pos-body", m_bodyend);
    ret.set("line-count", m_nlines);
    ret.set("body-line-count", m_nbodylines);
  } else {
    ret.set("ending-pos", m_bodyend);
    ret.set("ending-pos-body", m_bodyend);
    ret.set("line-count", m_nlines ? m_nlines - 1 : m_nlines);
    ret.set("body-line-count", m_nbodylines ? m_nbodylines - 1 : m_nbodylines);
  }

  if (!m_charset.empty()) {
    ret.set("charset", m_charset);
  } else {
    ret.set("charset", "us-ascii");
  }
  if (!m_content_transfer_encoding.empty()) {
    ret.set("transfer-encoding", m_content_transfer_encoding);
  } else {
    ret.set("transfer-encoding", "8bit");
  }
  if (!m_content_type.empty()) {
    m_content_type.getAll(ret, "content-type", "content-");
  } else {
    ret.set("content-type", "text/plain; (error)");
  }

  if (!m_content_disposition.empty()) {
    m_content_disposition.getAll(ret, "content-disposition", "disposition-");
  }

  if (!m_content_location.empty()) {
    ret.set("content-location", m_content_location);
  }
  if (!m_content_base.empty()) {
    ret.set("content-base", m_content_base);
  } else {
    ret.set("content-base", "/");
  }

  if (!m_boundary.empty()) {
    ret.set("content-boundary", m_boundary);
  }

  /* extract the address part of the content-id only */
  Variant contentId = m_headers["content-id"];
  if (!contentId.isNull()) {
    php_rfc822_tokenized_t *toks =
      php_mailparse_rfc822_tokenize((const char*)contentId.toString().data(),
                                    true);
    php_rfc822_addresses_t *addrs =
      php_rfc822_parse_address_tokens(toks);
    if (addrs->naddrs > 0) {
      ret.set("content-id", String(addrs->addrs[0].address, CopyString));
    }
    php_rfc822_free_addresses(addrs);
    php_rfc822_tokenize_free(toks);
  }

  const char *key = "content-description";
  if (m_headers.exists(key)) ret.set(key, m_headers[key]);
  key = "content-language";
  if (m_headers.exists(key)) ret.set(key, m_headers[key]);
  key = "content-md5";
  if (m_headers.exists(key)) ret.set(key, m_headers[key]);

  return ret;
}

bool MimePart::parse(const char *buf, int bufsize) {
  while (bufsize > 0) {
    /* look for EOL */
    int len = 0;
    for (; len < bufsize; len++) {
      if (buf[len] == '\n') break;
    }
    if (len < bufsize && buf[len] == '\n') {
      ++len;
      m_parsedata.workbuf += String(buf, len, CopyString);
      ProcessLine(this, m_parsedata.workbuf);
      m_parsedata.workbuf.clear();
    } else {
      m_parsedata.workbuf += String(buf, len, CopyString);
    }

    buf += len;
    bufsize -= len;
  }
  return true;
}

MimePart *MimePart::createChild(int startpos, bool inherit) {
  MimePart *child = NEW(MimePart)();
  m_parsedata.lastpart = child;
  child->m_parent = this;

  m_children.append(Object(child));
  child->m_startpos = child->m_endpos = child->m_bodystart =
    child->m_bodyend = startpos;

  if (inherit) {
    child->m_content_transfer_encoding = m_content_transfer_encoding;
    child->m_charset = m_charset;
  }
  return child;
}

bool MimePart::processHeader() {
  if (m_parsedata.headerbuf.empty()) {
    return true;
  }

  /* parse the header line */
  php_rfc822_tokenized_t *toks =
    php_mailparse_rfc822_tokenize(m_parsedata.headerbuf.data(), 0);

  /* valid headers consist of at least three tokens,
     with the first being a string and the second token being a ':' */
  if (toks->ntokens < 2 || toks->tokens[0].token != 0 ||
      toks->tokens[1].token != ':') {
    m_parsedata.headerbuf.clear();
    php_rfc822_tokenize_free(toks);
    return false;
  }

  /* get a lower-case version of the first token */
  String header_key(php_rfc822_recombine_tokens
                    (toks, 0, 1,
                     PHP_RFC822_RECOMBINE_IGNORE_COMMENTS|
                     PHP_RFC822_RECOMBINE_STRTOLOWER), AttachString);

  const char *header_val = strchr(m_parsedata.headerbuf.data(), ':');
  String header_val_stripped(php_rfc822_recombine_tokens
                             (toks, 2, toks->ntokens-2,
                              PHP_RFC822_RECOMBINE_IGNORE_COMMENTS|
                              PHP_RFC822_RECOMBINE_STRTOLOWER), AttachString);

  if (header_val) {
    header_val++;
    while (isspace(*header_val)) {
      header_val++;
    }

    /* add the header to the hash.
     * join multiple To: or Cc: lines together */
    if ((header_key == "to" || header_key == "cc") &&
        m_headers.exists(header_key)) {
      String newstr = m_headers[header_key].toString();
      newstr += ", ";
      newstr += header_val;
      m_headers.set(header_key, newstr);
    } else {
      if (m_headers.exists(header_key)) {
        Variant &zheaderval = m_headers.lvalAt(header_key);
        if (zheaderval.isArray()) {
          zheaderval.append(String(header_val, CopyString));
        } else {
          // Create a nested array if there is more than one of the same header
          Array zarr = Array::Create();
          zarr.append(zheaderval);
          zarr.append(String(header_val, CopyString));
          m_headers.set(header_key, zarr);
        }
      } else {
        m_headers.set(header_key, String(header_val, CopyString));
      }
    }

    /* if it is useful, keep a pointer to it in the mime part */
    if (header_key == "mime-version") {
      m_mime_version = header_val_stripped;
    } else if (header_key == "content-location") {
      m_content_location =
        String(php_rfc822_recombine_tokens
               (toks, 2, toks->ntokens-2,
                PHP_RFC822_RECOMBINE_IGNORE_COMMENTS), AttachString);
    } else if (header_key == "content-base") {
      m_content_base =
        String(php_rfc822_recombine_tokens
               (toks, 2, toks->ntokens-2,
                PHP_RFC822_RECOMBINE_IGNORE_COMMENTS), AttachString);
    } else if (header_key == "content-transfer-encoding") {
      m_content_transfer_encoding = header_val_stripped;
    } else if (header_key == "content-type") {
      m_content_type = MimeHeader(toks);
      Variant boundary = m_content_type.get("boundary");
      if (!boundary.isNull()) {
        m_boundary = boundary.toString();
      }
      Variant charset = m_content_type.get("charset");
      if (!charset.isNull()) {
        m_charset = charset.toString();
      }
    } else if (header_key == "content-disposition") {
      m_content_disposition = MimeHeader(toks);
    }
  }

  php_rfc822_tokenize_free(toks);
  m_parsedata.headerbuf.clear();
  return true;
}

bool MimePart::ProcessLine(MimePart *workpart, CStrRef line) {
  /* sanity check */
  if (workpart->m_children.size() > MAXPARTS) {
    raise_warning("MIME message too complex");
    return false;
  }

  const char *c = line.data();

  /* strip trailing \r\n -- we always have a trailing \n */
  int origcount = line.size();
  int linelen = origcount - 1;
  if (linelen && c[linelen-1] == '\r') {
    --linelen;
  }

  /* Discover which part we were last working on */
  while (!workpart->m_parsedata.lastpart.isNull()) {
    MimePart *lastpart = workpart->m_parsedata.lastpart.getTyped<MimePart>();

    if (lastpart->m_parsedata.completed) {
      UpdatePositions(workpart, workpart->m_endpos + origcount,
                      workpart->m_endpos + origcount, 1);
      return true;
    }
    if (workpart->m_boundary.empty() || workpart->m_parsedata.in_header) {
      workpart = lastpart;
      continue;
    }
    int bound_len = workpart->m_boundary.size();

    /* Look for a boundary */
    if (c[0] == '-' && c[1] == '-' && linelen >= 2+bound_len &&
        strncasecmp(workpart->m_boundary.data(), c+2, bound_len) == 0) {

      /* is it the final boundary ? */
      if (linelen >= 4 + bound_len && strncmp(c+2+bound_len, "--", 2) == 0) {
        lastpart->m_parsedata.completed = true;
        UpdatePositions(workpart, workpart->m_endpos + origcount,
                        workpart->m_endpos + origcount, 1);
        return true;
      }

      MimePart *newpart =
        workpart->createChild(workpart->m_endpos + origcount, true);
      UpdatePositions(workpart, workpart->m_endpos + origcount,
                      workpart->m_endpos + linelen, 1);
      newpart->m_mime_version = workpart->m_mime_version;
      newpart->m_parsedata.in_header = true;
      return true;
    }
    workpart = lastpart;
  }

  if (!workpart->m_parsedata.in_header) {
    if (!workpart->m_parsedata.completed &&
        workpart->m_parsedata.lastpart.isNull()) {
      /* update the body/part end positions.
       * For multipart messages, the final newline belongs to the boundary.
       * Otherwise it belongs to the body
       * */
      if (!workpart->m_parent.isNull() &&
          strncasecmp(workpart->getParent()->m_content_type.m_value.data(),
                      "multipart/", 10) == 0) {
        UpdatePositions(workpart, workpart->m_endpos + origcount,
                        workpart->m_endpos + linelen, true);
      } else {
        UpdatePositions(workpart, workpart->m_endpos + origcount,
                        workpart->m_endpos + origcount, true);
      }
    }
  } else {

    if (linelen > 0) {
      UpdatePositions(workpart, workpart->m_endpos + origcount,
                      workpart->m_endpos + linelen, true);

      if (*c == ' ' || *c == '\t') {
        /* This doesn't technically confirm to rfc2822, as we're replacing
           \t with \s, but this seems to fix cases where clients incorrectly
           fold by inserting a \t character.
         */
        workpart->m_parsedata.headerbuf += " ";
        c++; linelen--;
      } else {
        workpart->processHeader();
      }
      /* save header for possible continuation */
      workpart->m_parsedata.headerbuf += String(c, linelen, CopyString);

    } else {
      /* end of headers */
      workpart->processHeader();

      /* start of body */
      workpart->m_parsedata.in_header = false;
      workpart->m_bodystart = workpart->m_endpos + origcount;
      UpdatePositions(workpart, workpart->m_bodystart, workpart->m_bodystart,
                      true);
      --workpart->m_nbodylines;

      /* some broken mailers include the content-type header but not a
       * mime-version header.
       * Let's relax and pretend they said they were mime 1.0 compatible */
      if (workpart->m_mime_version.empty() &&
          !workpart->m_content_type.empty()) {
        workpart->m_mime_version = "1.0";
      }

      if (!workpart->isVersion1()) {
        /* if we don't understand the MIME version, discard the content-type
           and boundary */
        workpart->m_content_disposition.clear();
        workpart->m_boundary.clear();
        workpart->m_content_type.clear();
        workpart->m_content_type = MimeHeader("text/plain");
      }
      /* if there is no content type, default to text/plain, but use
         multipart/digest when in a multipart/rfc822 message */
      if (workpart->isVersion1() && workpart->m_content_type.empty()) {
        char *def_type = "text/plain";
        if (!workpart->m_parent.isNull() &&
            strcasecmp(workpart->getParent()->m_content_type.m_value.data(),
                       "multipart/digest") == 0) {
          def_type = "message/rfc822";
        }
        workpart->m_content_type = MimeHeader(def_type);
      }

      /* if no charset had previously been set, either through inheritance
       * or by an explicit content-type header, default to us-ascii */
      if (workpart->m_charset.isNull()) {
        workpart->m_charset = "us-ascii";
      }

      if (strcasecmp(workpart->m_content_type.m_value.data(),
                     "message/rfc822") == 0) {
        workpart = workpart->createChild(workpart->m_bodystart, false);
        workpart->m_parsedata.in_header = true;
        return true;

      }

      /* create a section for the preamble that precedes the first boundary */
      if (!workpart->m_boundary.empty()) {
        workpart = workpart->createChild(workpart->m_bodystart, true);
        workpart->m_parsedata.in_header = false;
        workpart->m_parsedata.is_dummy = true;
        return true;
      }

      return true;
    }

  }

  return true;
}

void MimePart::UpdatePositions(MimePart *part, int newendpos,
                               int newbodyend, int deltanlines) {
  while (part) {
    part->m_endpos = newendpos;
    part->m_bodyend = newbodyend;
    part->m_nlines += deltanlines;
    if (!part->m_parsedata.in_header) {
      part->m_nbodylines += deltanlines;
    }
    part = part->m_parent.getTyped<MimePart>(true);
  }
}

Variant MimePart::extract(CVarRef filename, CVarRef callbackfunc, int decode,
                          bool isfile) {
  /* filename can be a filename or a stream */
  Object file;
  File *f = NULL;
  if (filename.isResource()) {
    f = filename.toObject().getTyped<File>();
  } else if (isfile) {
    Variant stream = File::Open(filename, "rb", Array());
    if (!same(stream, false)) {
      file = stream.toObject();
      f = file.getTyped<File>();
    }
  } else {
    /* filename is the actual data */
    String data = filename.toString();
    f = NEW(MemFile)(data.data(), data.size());
    file = Object(f);
  }

  if (f == NULL) {
    return false;
  }

  m_extract_context = callbackfunc;
  if (callbackfunc.isString() && callbackfunc.toString().empty()) {
    m_extract_func = &MimePart::outputToStdout;
  } else {
    if (callbackfunc.isNull()) {
      m_extract_func = &MimePart::outputToString;
    } else if (callbackfunc.isResource()) {
      m_extract_func = &MimePart::outputToFile;
    } else {
      m_extract_func = &MimePart::callUserFunc;
    }
  }

  if (extractImpl(decode, f)) {
    if (callbackfunc.isNull()) {
      return m_extract_context;
    }
    if (callbackfunc.isResource()) {
      return f_stream_get_contents(callbackfunc);
    }
    return true;
  }
  return null;
}

int MimePart::extractImpl(int decode, File *src) {
  /* figure out where the message part starts/ends */
  int start_pos = (decode & DecodeNoHeaders) ? m_bodystart : m_startpos;
  int end = (decode & DecodeNoBody) ?
    m_bodystart : (!m_parent.isNull() ? m_bodyend : m_endpos);

  decoderPrepare(decode & Decode8Bit);

  if (!src->seek(start_pos)) {
    raise_warning("unable to seek to section start");
    decoderFinish();
    return false;
  }

  while (start_pos < end) {
    int n = 4095;
    if (n > end - start_pos) n = end - start_pos;
    String str = src->read(n);
    if (str.empty()) {
      raise_warning("error reading from file at offset %lld", start_pos);
      decoderFinish();
      return false;
    }
    decoderFeed(str);
    start_pos += str.size();
  }
  decoderFinish();
  return true;
}

void MimePart::callUserFunc(CStrRef s) {
  f_call_user_func_array(m_extract_context, CREATE_VECTOR1(s));
}

void MimePart::outputToStdout(CStrRef s) {
  echo(s);
}

void MimePart::outputToFile(CStrRef s) {
  m_extract_context.toObject().getTyped<File>()->write(s);
}

void MimePart::outputToString(CStrRef s) {
  m_extract_context = m_extract_context.toString() + s;
}

///////////////////////////////////////////////////////////////////////////////
}
