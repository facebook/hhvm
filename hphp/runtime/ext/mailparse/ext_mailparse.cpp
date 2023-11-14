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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/temp-file.h"
#include "hphp/runtime/ext/mailparse/mime.h"
#include "hphp/runtime/ext/mailparse/rfc822.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

static req::ptr<File> get_valid_file_resource(const OptResource& fp) {
  auto f = dyn_cast_or_null<File>(fp);
  if (f == nullptr || f->isClosed()) {
    raise_warning("Not a valid stream resource");
    return nullptr;
  }
  return f;
}

///////////////////////////////////////////////////////////////////////////////

OptResource HHVM_FUNCTION(mailparse_msg_create) {
  return OptResource(req::make<MimePart>());
}

bool HHVM_FUNCTION(mailparse_msg_free, const OptResource& /*mimemail*/) {
  return true;
}

Variant HHVM_FUNCTION(mailparse_msg_parse_file, const String& filename) {
  auto f = File::Open(filename, "rb");
  if (!f) return false;

  auto p = req::make<MimePart>();
  while (!f->eof()) {
    String line = f->readLine();
    if (!line.isNull()) {
      if (!MimePart::ProcessLine(p, line)) {
        return false;
      }
    }
  }
  return Variant(std::move(p));
}

bool HHVM_FUNCTION(mailparse_msg_parse,
                   const OptResource& mimemail,
                   const String& data) {
  return cast<MimePart>(mimemail)->parse(data.data(), data.size());
}

Variant HHVM_FUNCTION(mailparse_msg_extract_part_file,
                      const OptResource& mimemail,
                      const Variant& filename,
                      const Variant& callbackfunc /* = "" */) {
  return cast<MimePart>(mimemail)->
    extract(filename, callbackfunc,
            MimePart::Decode8Bit | MimePart::DecodeNoHeaders, true);
}

Variant HHVM_FUNCTION(mailparse_msg_extract_whole_part_file,
                      const OptResource& mimemail,
                      const Variant& filename,
                      const Variant& callbackfunc /* = "" */) {
  return cast<MimePart>(mimemail)->
    extract(filename, callbackfunc, MimePart::DecodeNone, true);
}

Variant HHVM_FUNCTION(mailparse_msg_extract_part,
                      const OptResource& mimemail,
                      const Variant& msgbody,
                      const Variant& callbackfunc /* = "" */) {
  return cast<MimePart>(mimemail)->
    extract(msgbody, callbackfunc,
            MimePart::Decode8Bit | MimePart::DecodeNoHeaders, false);
}

Array HHVM_FUNCTION(mailparse_msg_get_part_data, const OptResource& mimemail) {
  return cast<MimePart>(mimemail)->getPartData();
}

Variant HHVM_FUNCTION(mailparse_msg_get_part,
                      const OptResource& mimemail,
                      const String& mimesection) {
  OptResource part =
    cast<MimePart>(mimemail)->findByName(mimesection.c_str());
  if (part.isNull()) {
    raise_warning("cannot find section %s in message", mimesection.data());
    return false;
  }
  return part;
}

Array HHVM_FUNCTION(mailparse_msg_get_structure, const OptResource& mimemail) {
  return cast<MimePart>(mimemail)->getStructure();
}

const StaticString
  s_display("display"),
  s_address("address"),
  s_is_group("is_group");

Array HHVM_FUNCTION(mailparse_rfc822_parse_addresses,
                    const String& addresses) {
  php_rfc822_tokenized_t *toks =
    php_mailparse_rfc822_tokenize(addresses.data(), 1);
  php_rfc822_addresses_t *addrs = php_rfc822_parse_address_tokens(toks);

  Array ret = Array::CreateVec();
  for (int i = 0; i < addrs->naddrs; i++) {
    Array item = Array::CreateDict();
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

bool HHVM_FUNCTION(mailparse_stream_encode,
                   const OptResource& sourcefp,
                   const OptResource& destfp,
                   const String& encoding) {
  auto srcstream = get_valid_file_resource(sourcefp);
  if (!srcstream) {
    return false;
  }
  auto deststream = get_valid_file_resource(destfp);
  if (!deststream) {
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
                            deststream.get());

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

static size_t mailparse_do_uudecode(const req::ptr<File>& instream,
                                    const req::ptr<File>& outstream) {
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

Variant HHVM_FUNCTION(mailparse_uudecode_all, const OptResource& fp) {
  auto instream = get_valid_file_resource(fp);
  if (!instream) {
    return false;
  }
  instream->rewind();

  auto outstream = req::make<TempFile>(false);

  Array return_value;
  int nparts = 0;
  while (!instream->eof()) {
    String line = instream->readLine();
    if (line.isNull()) break;

    /* Look for the "begin " sequence that identifies a uuencoded file */
    if (strncmp(line.data(), "begin ", 6) == 0) {

      /*
       * The next 4 bytes are an octal number for perms and a space; ignore them
       * If for some reason they aren't found treat it as an empty filename.
       */
      const char *origfilename = "";
      size_t namelen = 0;
      if (line.size() >= 10) {
        origfilename = line.data() + 10;
        namelen = line.size() - 10;
        while (namelen > 0 && isspace(origfilename[namelen-1])) {
          --namelen;
        }
      }

      /* make the return an array */
      if (nparts == 0) {
        return_value = Array::CreateVec();
        /* create an initial item representing the file with all uuencoded
           parts removed */
        Array item = Array::CreateDict();
        item.set(s_filename, String(outstream->getName()));
        return_value.append(item);
      }

      /* add an item */
      Array item = Array::CreateDict();
      item.set(s_origfilename, String(origfilename, namelen, CopyString));

      /* create a temp file for the data */
      auto partstream = req::make<TempFile>(false);
      if (partstream)  {
        nparts++;
        item.set(s_filename, String(partstream->getName()));
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

Variant HHVM_FUNCTION(mailparse_determine_best_xfer_encoding,
                      const OptResource& fp) {
  auto stream = get_valid_file_resource(fp);
  if (!stream) {
    return false;
  }
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

struct MailparseExtension final : Extension {
  MailparseExtension() : Extension("mailparse", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) { }
  void moduleInit() override {
    HHVM_FE(mailparse_msg_create);
    HHVM_FE(mailparse_msg_free);
    HHVM_FE(mailparse_msg_parse_file);
    HHVM_FE(mailparse_msg_parse);
    HHVM_FE(mailparse_msg_extract_part_file);
    HHVM_FE(mailparse_msg_extract_whole_part_file);
    HHVM_FE(mailparse_msg_extract_part);
    HHVM_FE(mailparse_msg_get_part_data);
    HHVM_FE(mailparse_msg_get_part);
    HHVM_FE(mailparse_msg_get_structure);
    HHVM_FE(mailparse_rfc822_parse_addresses);
    HHVM_FE(mailparse_stream_encode);
    HHVM_FE(mailparse_uudecode_all);
    HHVM_FE(mailparse_determine_best_xfer_encoding);
  }
} s_mailparse_extension;

///////////////////////////////////////////////////////////////////////////////
}
