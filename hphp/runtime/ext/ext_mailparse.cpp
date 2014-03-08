/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_mailparse.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/temp-file.h"
#include "hphp/runtime/ext/ext_process.h"
#include "hphp/runtime/ext/mailparse/mime.h"
#include "hphp/runtime/ext/mailparse/rfc822.h"
#include "hphp/runtime/base/zend-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// utility functions

/**
 * Removes whitespaces from the end, and replaces control characters with ' '
 * from the beginning.
 */
static String php_trim(const String& str) {
  std::string s(str.c_str());
  unsigned int l = s.length();
  while (l > 0 && isspace((unsigned char)s[l - 1])) {
    l--;
  }
  for (unsigned int i = 0; i < l; i++) {
    if (iscntrl((unsigned char)s[i])) {
      if (i + 2 < l && s[i] == '\r' && s[i + 1] == '\n' &&
          (s[i + 2] == ' ' || s[i + 2] == '\t')) {
        i += 2;
        while (i + 1 < l && (s[i + 1] == ' ' || s[i + 1] == '\t')) {
          i++;
        }
        continue;
      }
      s[i] = ' ';
    }
  }
  return s.substr(0, l);
}

///////////////////////////////////////////////////////////////////////////////

bool php_mail(const String& to, const String& subject, const String& message,
              const String& headers, const String& extra_cmd) {
  // assumes we always have sendmail installed
  always_assert(!RuntimeOption::SendmailPath.empty());

  std::ostringstream os;
  os << RuntimeOption::SendmailPath;
  if (!extra_cmd.empty()) {
    os << ' ' << extra_cmd.c_str();
  }

  errno = 0;
  FILE *sendmail = popen(os.str().c_str(), "w");
  if (sendmail == NULL || EACCES == errno) {
    raise_warning("Unable to execute %s",
                  RuntimeOption::SendmailPath.c_str());
    return false;
  }

  fprintf(sendmail, "To: %s\n", to.c_str());
  fprintf(sendmail, "Subject: %s\n", subject.c_str());
  if (!headers.empty()) {
    fprintf(sendmail, "%s\n", headers.c_str());
  }
  fprintf(sendmail, "\n%s\n", message.c_str());

  int ret = pclose(sendmail);
  return (!ret);
}

const StaticString
  s_zero(LITSTR_INIT("\0")),
  s_space(" ");

bool f_mail(const String& to, const String& subject, const String& message,
            const String& additional_headers /* = null_string */,
            const String& additional_parameters /* = null_string */) {
  // replace \0 with spaces
  String to2 = string_replace(to, s_zero, s_space);
  String subject2 = string_replace(subject, s_zero, s_space);
  String message2 = string_replace(message, s_zero, s_space);
  String headers2;
  if (!additional_headers.empty()) {
    headers2 = string_replace(additional_headers, s_zero, s_space);
  }
  String params2;
  if (!additional_parameters.empty()) {
    params2 = string_replace(additional_parameters, s_zero, s_space);
  }

  to2 = php_trim(to2);
  subject2 = php_trim(subject2);

  if (!RuntimeOption::MailForceExtraParameters.empty()) {
    params2 = f_escapeshellcmd(RuntimeOption::MailForceExtraParameters);
  } else {
    params2 = f_escapeshellcmd(params2);
  }

  return php_mail(to2, subject2, message2, headers2, params2);
}

int64_t f_ezmlm_hash(const String& addr) {
  unsigned long h = 5381L;
  int str_len = addr.length();
  for (int i = 0; i < str_len; i++) {
    h = (h + (h << 5)) ^
        ((unsigned long)(unsigned char)tolower(addr.charAt(i)));
  }
  h = (h % 53);
  return (int)h;
}

///////////////////////////////////////////////////////////////////////////////
// mailparse

Resource f_mailparse_msg_create() {
  return NEWOBJ(MimePart)();
}

bool f_mailparse_msg_free(const Resource& mimemail) {
  return true;
}

Variant f_mailparse_msg_parse_file(const String& filename) {
  Variant stream = File::Open(filename, "rb");
  if (same(stream, false)) return false;
  File *f = stream.toResource().getTyped<File>();

  MimePart *p = NEWOBJ(MimePart)();
  Resource ret(p);
  while (!f->eof()) {
    String line = f->readLine();
    if (!line.isNull()) {
      if (!MimePart::ProcessLine(p, line)) {
        return false;
      }
    }
  }
  return ret;
}

bool f_mailparse_msg_parse(const Resource& mimemail, const String& data) {
  return mimemail.getTyped<MimePart>()->parse(data.data(), data.size());
}

Variant f_mailparse_msg_extract_part_file(const Resource& mimemail, const Variant& filename,
                                          const Variant& callbackfunc /* = "" */) {
  return mimemail.getTyped<MimePart>()->
    extract(filename, callbackfunc,
            MimePart::Decode8Bit | MimePart::DecodeNoHeaders, true);
}

Variant f_mailparse_msg_extract_whole_part_file(const Resource& mimemail,
                                                const Variant& filename,
                                                const Variant& callbackfunc /* = "" */) {
  return mimemail.getTyped<MimePart>()->
    extract(filename, callbackfunc, MimePart::DecodeNone, true);
}

Variant f_mailparse_msg_extract_part(const Resource& mimemail, const Variant& msgbody,
                                     const Variant& callbackfunc /* = "" */) {
  return mimemail.getTyped<MimePart>()->
    extract(msgbody, callbackfunc,
            MimePart::Decode8Bit | MimePart::DecodeNoHeaders, false);
}

Array f_mailparse_msg_get_part_data(const Resource& mimemail) {
  return mimemail.getTyped<MimePart>()->getPartData().toArray();
}

Variant f_mailparse_msg_get_part(const Resource& mimemail, const String& mimesection) {
  Resource part =
    mimemail.getTyped<MimePart>()->findByName(mimesection.c_str());
  if (part.isNull()) {
    raise_warning("cannot find section %s in message", mimesection.data());
    return false;
  }
  return part;
}

Array f_mailparse_msg_get_structure(const Resource& mimemail) {
  return mimemail.getTyped<MimePart>()->getStructure();
}

const StaticString
  s_display("display"),
  s_address("address"),
  s_is_group("is_group");

Array f_mailparse_rfc822_parse_addresses(const String& addresses) {
  php_rfc822_tokenized_t *toks =
    php_mailparse_rfc822_tokenize(addresses.data(), 1);
  php_rfc822_addresses_t *addrs = php_rfc822_parse_address_tokens(toks);

  Array ret = Array::Create();
  for (int i = 0; i < addrs->naddrs; i++) {
    Array item = Array::Create();
    if (addrs->addrs[i].name) {
      item.set(s_display, String(addrs->addrs[i].name, CopyString));
    }
    if (addrs->addrs[i].address) {
      item.set(s_address, String(addrs->addrs[i].address, CopyString));
    }
    item.set(s_is_group, (bool)addrs->addrs[i].is_group);
    ret.append(item);
  }

  php_rfc822_free_addresses(addrs);
  php_rfc822_tokenize_free(toks);
  return ret;
}

static int mailparse_stream_output(int c, void *stream) {
  char buf[2];
  buf[0] = c;
  buf[1] = '\0';
  return ((File*)stream)->write(buf, 1);
}
static int mailparse_stream_flush(void *stream) {
  return ((File*)stream)->flush() ? 1 : 0;
}

bool f_mailparse_stream_encode(const Resource& sourcefp, const Resource& destfp,
                               const String& encoding) {
  File *srcstream = sourcefp.getTyped<File>(true, true);
  File *deststream = destfp.getTyped<File>(true, true);
  if (srcstream == NULL || deststream == NULL) {
    return false;
  }

  enum mbfl_no_encoding enc = mbfl_name2no_encoding(encoding.data());
  if (enc == mbfl_no_encoding_invalid)  {
    raise_warning("Unknown encoding \"%s\"", encoding.data());
    return false;
  }

  mbfl_convert_filter *conv =
    mbfl_convert_filter_new(mbfl_no_encoding_8bit, enc,
                            mailparse_stream_output, mailparse_stream_flush,
                            deststream);

  if (enc == mbfl_no_encoding_qprint) {
    /* If the qp encoded section is going to be digitally signed,
     * it is a good idea to make sure that lines that begin "From "
     * have the letter F encoded, so that MTAs do not stick a > character
     * in front of it and invalidate the content/signature */
    while (!srcstream->eof())  {
      String line = srcstream->readLine();
      if (!line.isNull()) {
        int i;
        if (strncmp(line.data(), "From ", 5) == 0) {
          mbfl_convert_filter_flush(conv);
          deststream->write("=46rom ", 7);
          i = 5;
        } else {
          i = 0;
        }
        const char *p = line.data();
        for (; i < line.size(); i++) {
          mbfl_convert_filter_feed(p[i], conv);
        }
      }
    }

  } else {
    while (!srcstream->eof())  {
      String data = srcstream->read();
      if (!data.empty()) {
        const char *p = data.data();
        for (int i = 0; i < data.size(); i++) {
          mbfl_convert_filter_feed(p[i], conv);
        }
      }
    }
  }

  mbfl_convert_filter_flush(conv);
  mbfl_convert_filter_delete(conv);
  return true;
}

#define UUDEC(c) (char)(((c)-' ')&077)
#define UU_NEXT(v) \
  if (line[x] == '\0' || line[x] == '\r' || line[x] == '\n') break; \
  v = line[x++]; v = UUDEC(v)

static size_t mailparse_do_uudecode(File *instream, File *outstream) {
  int A, B, C, D, n;
  size_t file_size = 0;
  if (outstream) {
    /* write to outstream */
    while (!instream->eof())  {
      String line = instream->readLine(128);
      if (line.isNull()) break;

      int x = 0;
      UU_NEXT(n);
      while (n) {
        UU_NEXT(A); UU_NEXT(B); UU_NEXT(C); UU_NEXT(D);
        if (n-- > 0) {
          file_size++;
          outstream->putc((A << 2) | (B >> 4));
        }
        if (n-- > 0) {
          file_size++;
          outstream->putc((B << 4) | (C >> 2));
        }
        if (n-- > 0) {
          file_size++;
          outstream->putc((C << 6) | D);
        }
      }
    }
  } else {
    /* skip (and measure) the data, but discard it.
     * This is separated from the version above to speed it up by a few cycles
     */
    while (!instream->eof())  {
      String line = instream->readLine(128);
      if (line.isNull()) break;

      int x = 0;
      UU_NEXT(n);
      while (line[x] && n != 0) {
        UU_NEXT(A); UU_NEXT(B); UU_NEXT(C); UU_NEXT(D);
        if (n-- > 0) file_size++;
        if (n-- > 0) file_size++;
        if (n-- > 0) file_size++;
      }
    }
  }
  return file_size;
}

const StaticString
  s_filename("filename"),
  s_origfilename("origfilename");

Variant f_mailparse_uudecode_all(const Resource& fp) {
  File *instream = fp.getTyped<File>();
  instream->rewind();

  File *outstream = NEWOBJ(TempFile)(false);
  Resource deleter(outstream);

  Array return_value;
  int nparts = 0;
  while (!instream->eof()) {
    String line = instream->readLine();
    if (line.isNull()) break;

    /* Look for the "begin " sequence that identifies a uuencoded file */
    if (strncmp(line.data(), "begin ", 6) == 0) {
      /* parse out the file name.
       * The next 4 bytes are an octal number for perms; ignore it */
       // TODO: Update gcc and get rid of this dumb workaround.
      char *origfilename = (char *)((size_t)line.data() + (10 * sizeof(char)));
      /* NUL terminate the filename */
      int len = strlen(origfilename);
      while (isspace(origfilename[len-1])) {
        origfilename[--len] = '\0';
      }

      /* make the return an array */
      if (nparts == 0) {
        return_value = Array::Create();
        /* create an initial item representing the file with all uuencoded
           parts removed */
        Array item = Array::Create();
        item.set(s_filename, String(((TempFile*)outstream)->getName()));
        return_value.append(item);
      }

      /* add an item */
      Array item = Array::Create();
      item.set(s_origfilename, String(origfilename, CopyString));

      /* create a temp file for the data */
      File *partstream = NEWOBJ(TempFile)(false);
      Resource deleter(partstream);
      if (partstream)  {
        nparts++;
        item.set(s_filename, String(((TempFile*)partstream)->getName()));
        return_value.append(item);

        /* decode it */
        mailparse_do_uudecode(instream, partstream);
      }
    } else {
      /* write to the output file */
      outstream->write(line);
    }
  }

  instream->rewind();
  if (nparts == 0) {
    return false;
  }
  return return_value;
}

Variant f_mailparse_determine_best_xfer_encoding(const Resource& fp) {
  File *stream = fp.getTyped<File>();
  stream->rewind();

  int linelen = 0;
  enum mbfl_no_encoding bestenc = mbfl_no_encoding_7bit;
  bool longline = false;
  while (!stream->eof()) {
    int c = stream->getc();
    if (c > 0x80) {
      bestenc = mbfl_no_encoding_8bit;
    } else if (c == 0)  {
      bestenc = mbfl_no_encoding_base64;
      longline = false;
      break;
    }
    if (c == '\n') {
      linelen = 0;
    } else if (++linelen > 200) {
      longline = true;
    }
  }
  if (longline) bestenc = mbfl_no_encoding_qprint;
  stream->rewind();

  char * name = (char *)mbfl_no2preferred_mime_name(bestenc);
  if (name) {
    return String(name, CopyString);
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
