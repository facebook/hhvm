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

#include "hphp/runtime/ext/libxml/ext_libxml.h"
#include "hphp/runtime/ext/ext_file.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/file.h"

#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/tree.h>
#include <libxml/uri.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlsave.h>
#ifdef LIBXML_SCHEMAS_ENABLED
#include <libxml/relaxng.h>
#include <libxml/xmlschemas.h>
#endif

#include <memory>
#include <cstring>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static xmlParserInputBufferPtr
libxml_create_input_buffer(const char *URI, xmlCharEncoding enc);

static xmlOutputBufferPtr
libxml_create_output_buffer(const char *URI,
  xmlCharEncodingHandlerPtr encoder, int compression ATTRIBUTE_UNUSED);

static Resource libxml_streams_IO_open_wrapper(
    const char *filename, const char* mode, bool read_only);
static int libxml_streams_IO_read(void *context, char *buffer, int len);
static int libxml_streams_IO_write(void *context, const char *buffer, int len);
static int libxml_streams_IO_close(void *context);

class xmlErrorVec : public std::vector<xmlError> {
public:
  ~xmlErrorVec() {
    clearErrors();
  }

  void reset() {
    clearErrors();
    xmlErrorVec().swap(*this);
  }

private:
  void clearErrors() {
    for (int64_t i = 0; i < size(); i++) {
      xmlResetError(&at(i));
    }
  }
};

struct LibXmlRequestData final : RequestEventHandler {
  void requestInit() override {
    m_use_error = false;
    m_errors.reset();
    m_entity_loader_disabled = false;
    m_streams_context = uninit_null();
  }

  void requestShutdown() override {
    m_use_error = false;
    m_errors.reset();
    m_streams_context = uninit_null();
  }

  bool m_entity_loader_disabled;
  bool m_use_error;
  xmlErrorVec m_errors;
  Variant m_streams_context;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(LibXmlRequestData, tl_libxml_request_data);

static Class * s_LibXMLError_class;

const StaticString
  s_LibXMLError("LibXMLError"),
  s_level("level"),
  s_code("code"),
  s_column("column"),
  s_message("message"),
  s_file("file"),
  s_line("line"),
  s_LIBXML_VERSION("LIBXML_VERSION"),
  s_LIBXML_DOTTED_VERSION("LIBXML_DOTTED_VERSION"),
  s_LIBXML_LOADED_VERSION("LIBXML_LOADED_VERSION"),
  s_LIBXML_NOENT("LIBXML_NOENT"),
  s_LIBXML_DTDLOAD("LIBXML_DTDLOAD"),
  s_LIBXML_DTDATTR("LIBXML_DTDATTR"),
  s_LIBXML_DTDVALID("LIBXML_DTDVALID"),
  s_LIBXML_NOERROR("LIBXML_NOERROR"),
  s_LIBXML_NOWARNING("LIBXML_NOWARNING"),
  s_LIBXML_NOBLANKS("LIBXML_NOBLANKS"),
  s_LIBXML_XINCLUDE("LIBXML_XINCLUDE"),
  s_LIBXML_NSCLEAN("LIBXML_NSCLEAN"),
  s_LIBXML_NOCDATA("LIBXML_NOCDATA"),
  s_LIBXML_NONET("LIBXML_NONET"),
  s_LIBXML_PEDANTIC("LIBXML_PEDANTIC"),
  s_LIBXML_COMPACT("LIBXML_COMPACT"),
  s_LIBXML_NOXMLDECL("LIBXML_NOXMLDECL"),
  s_LIBXML_PARSEHUGE("LIBXML_PARSEHUGE"),
  s_LIBXML_NOEMPTYTAG("LIBXML_NOEMPTYTAG"),
  s_LIBXML_SCHEMA_CREATE("LIBXML_SCHEMA_CREATE"),
  s_LIBXML_HTML_NOIMPLIED("LIBXML_HTML_NOIMPLIED"),
  s_LIBXML_HTML_NODEFDTD("LIBXML_HTML_NODEFDTD"),
  s_LIBXML_ERR_NONE("LIBXML_ERR_NONE"),
  s_LIBXML_ERR_WARNING("LIBXML_ERR_WARNING"),
  s_LIBXML_ERR_ERROR("LIBXML_ERR_ERROR"),
  s_LIBXML_ERR_FATAL("LIBXML_ERR_FATAL");

bool libxml_use_internal_error() {
  return tl_libxml_request_data->m_use_error;
}

void libxml_add_error(const std::string &msg) {
  xmlErrorVec* error_list = &tl_libxml_request_data->m_errors;

  error_list->resize(error_list->size() + 1);
  xmlError &error_copy = error_list->back();
  memset(&error_copy, 0, sizeof(xmlError));

  error_copy.domain = 0;
  error_copy.code = XML_ERR_INTERNAL_ERROR;
  error_copy.level = XML_ERR_ERROR;
  error_copy.line = 0;
  error_copy.node = nullptr;
  error_copy.int1 = 0;
  error_copy.int2 = 0;
  error_copy.ctxt = nullptr;
  error_copy.message = (char*)xmlStrdup((const xmlChar*)msg.c_str());
  error_copy.file = nullptr;
  error_copy.str1 = nullptr;
  error_copy.str2 = nullptr;
  error_copy.str3 = nullptr;
}

static void libxml_error_handler(void* userData, xmlErrorPtr error) {
  xmlErrorVec* error_list = &tl_libxml_request_data->m_errors;

  error_list->resize(error_list->size() + 1);
  xmlError &error_copy = error_list->back();
  memset(&error_copy, 0, sizeof(xmlError));

  if (error) {
    xmlCopyError(error, &error_copy);
  } else {
    error_copy.code = XML_ERR_INTERNAL_ERROR;
    error_copy.level = XML_ERR_ERROR;
  }
}

static Object create_libxmlerror(xmlError &error) {
  Object ret = ObjectData::newInstance(s_LibXMLError_class);
  ret->o_set(s_level,   error.level);
  ret->o_set(s_code,    error.code);
  ret->o_set(s_column,  error.int2);
  ret->o_set(s_message, String(error.message, CopyString));
  ret->o_set(s_file,    String(error.file, CopyString));
  ret->o_set(s_line,    error.line);
  return ret;
}

Variant HHVM_FUNCTION(libxml_get_errors) {
  xmlErrorVec* error_list = &tl_libxml_request_data->m_errors;
  Array ret = Array::Create();
  for (int64_t i = 0; i < error_list->size(); i++) {
    ret.append(create_libxmlerror(error_list->at(i)));
  }
  return ret;
}

Variant HHVM_FUNCTION(libxml_get_last_error) {
  xmlErrorPtr error = xmlGetLastError();
  if (error) {
    return create_libxmlerror(*error);
  }
  return false;
}

void HHVM_FUNCTION(libxml_clear_errors) {
  xmlResetLastError();
  tl_libxml_request_data->m_errors.reset();
}

bool HHVM_FUNCTION(libxml_use_internal_errors, bool use_errors) {
  bool ret = (xmlStructuredError == libxml_error_handler);
  if (!use_errors) {
    xmlSetStructuredErrorFunc(nullptr, nullptr);
    tl_libxml_request_data->m_use_error = false;
    tl_libxml_request_data->m_errors.reset();
  } else {
    xmlSetStructuredErrorFunc(nullptr, libxml_error_handler);
    tl_libxml_request_data->m_use_error = true;
  }
  return ret;
}

bool HHVM_FUNCTION(libxml_disable_entity_loader, bool disable /* = true */) {
  bool old = tl_libxml_request_data->m_entity_loader_disabled;

  tl_libxml_request_data->m_entity_loader_disabled = disable;

  return old;
}

void HHVM_FUNCTION(libxml_set_streams_context, const Resource & context) {
  tl_libxml_request_data->m_streams_context = context;
}

///////////////////////////////////////////////////////////////////////////////
// Callbacks
//
// Note that these stream callbacks may re-enter the VM via a user-defined
// stream wrapper. The VM state should be synced using VMRegAnchor by the
// caller, before entering libxml2.

static Resource libxml_streams_IO_open_wrapper(
    const char *filename, const char* mode, bool read_only)
{
  String strFilename = StringData::Make(filename, CopyString);
  /* FIXME: PHP calls stat() here if the wrapper has a non-null stat handler,
   * in order to skip the open of a missing file, thus suppressing warnings.
   * Our stat handlers are virtual, so there's no easy way to tell if stat
   * is supported, so instead we will just call stat() for plain files, since
   * of the default transports, only plain files have support for stat().
   */
  if (read_only) {
    int pathIndex = 0;
    Stream::Wrapper * wrapper = Stream::getWrapperFromURI(strFilename, &pathIndex);
    if (dynamic_cast<FileStreamWrapper*>(wrapper)) {
      if (!f_file_exists(strFilename)) {
        return Resource();
      }
    }
  }

  // PHP unescapes the URI here, but that should properly be done by the wrapper.
  // The wrapper should expect a valid URI, e.g. file:///foo%20bar
  return File::Open(strFilename, mode, 0,
      tl_libxml_request_data->m_streams_context);
}

static int libxml_streams_IO_read(void *context, char *buffer, int len) {
  Resource stream = (ResourceData*)context;
  assert(len >= 0);
  Variant ret = f_fread(stream, len);
  if (ret.is(KindOfString)) {
    const String & str = ret.asCStrRef();
    if (str.size() <= len) {
      std::memcpy(buffer, str.data(), str.size());
      return str.size();
    } else {
      return -1;
    }
  } else {
    return -1;
  }
}

static int libxml_streams_IO_write(void *context, const char *buffer, int len) {
  Resource stream = (ResourceData*)context;
  String strBuffer = StringData::Make(buffer, len, CopyString);
  Variant ret = f_fwrite(stream, strBuffer);
  if (ret.is(KindOfInt64) && ret.asInt64Val() < INT_MAX) {
    return (int)ret.asInt64Val();
  } else {
    return -1;
  }
}

static int libxml_streams_IO_close(void *context) {
  if (MemoryManager::sweeping()) {
    // Stream wrappers close themselves on sweep, so there's nothing to do here
    return 0;
  }
  Resource stream = (ResourceData*)context;
  int ret = f_fclose(stream) ? 0 : -1;
  stream.get()->decRefAndRelease();
  return ret;
}

static xmlParserInputBufferPtr
libxml_create_input_buffer(const char *URI, xmlCharEncoding enc) {
  if (tl_libxml_request_data->m_entity_loader_disabled) {
    return nullptr;
  }

  if (URI == nullptr) {
    return nullptr;
  }

  Resource stream = libxml_streams_IO_open_wrapper(URI, "rb", true);

  if (stream.isInvalid()) {
    return nullptr;
  }

  // Allocate the Input buffer front-end.
  xmlParserInputBufferPtr ret = xmlAllocParserInputBuffer(enc);
  if (ret != nullptr) {
    stream.get()->incRefCount();
    ret->context = (void*)stream.get();
    ret->readcallback = libxml_streams_IO_read;
    ret->closecallback = libxml_streams_IO_close;
  }

  return ret;
}

static xmlOutputBufferPtr
libxml_create_output_buffer(const char *URI,
                              xmlCharEncodingHandlerPtr encoder,
                              int compression ATTRIBUTE_UNUSED)
{
  if (URI == nullptr) {
    return nullptr;
  }
  // PHP unescapes the URI here, but that should properly be done by the wrapper.
  // The wrapper should expect a valid URI, e.g. file:///foo%20bar
  Resource stream = libxml_streams_IO_open_wrapper(URI, "wb", false);
  if (stream.isInvalid()) {
    return nullptr;
  }
  // Allocate the Output buffer front-end.
  xmlOutputBufferPtr ret = xmlAllocOutputBuffer(encoder);
  if (ret != nullptr) {
    stream.get()->incRefCount();
    ret->context = (void*)stream.get();
    ret->writecallback = libxml_streams_IO_write;
    ret->closecallback = libxml_streams_IO_close;
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// Extension

class LibXMLExtension : public Extension {
  public:
    LibXMLExtension() : Extension("libxml") {}

  private:
    // Aliases for brevity
    inline static void cnsInt(const StaticString & name, int64_t value) {
      Native::registerConstant<KindOfInt64>(name.get(), value);
    }

    inline static void cnsStr(const StaticString & name, const char * value) {
      Native::registerConstant<KindOfStaticString>(
          name.get(), StaticString(value).get());
    }

    void moduleInit() override {
      cnsInt(s_LIBXML_VERSION, LIBXML_VERSION);
      cnsStr(s_LIBXML_DOTTED_VERSION, LIBXML_DOTTED_VERSION);
      cnsStr(s_LIBXML_LOADED_VERSION, xmlParserVersion);

      // For use with loading xml
      cnsInt(s_LIBXML_NOENT, XML_PARSE_NOENT);
      cnsInt(s_LIBXML_DTDLOAD, XML_PARSE_DTDLOAD);
      cnsInt(s_LIBXML_DTDATTR, XML_PARSE_DTDATTR);
      cnsInt(s_LIBXML_DTDVALID, XML_PARSE_DTDVALID);
      cnsInt(s_LIBXML_NOERROR, XML_PARSE_NOERROR);
      cnsInt(s_LIBXML_NOWARNING, XML_PARSE_NOWARNING);
      cnsInt(s_LIBXML_NOBLANKS, XML_PARSE_NOBLANKS);
      cnsInt(s_LIBXML_XINCLUDE, XML_PARSE_XINCLUDE);
      cnsInt(s_LIBXML_NSCLEAN, XML_PARSE_NSCLEAN);
      cnsInt(s_LIBXML_NOCDATA, XML_PARSE_NOCDATA);
      cnsInt(s_LIBXML_NONET, XML_PARSE_NONET);
      cnsInt(s_LIBXML_PEDANTIC, XML_PARSE_PEDANTIC);
      cnsInt(s_LIBXML_COMPACT, XML_PARSE_COMPACT);
      cnsInt(s_LIBXML_NOXMLDECL, XML_SAVE_NO_DECL);
      cnsInt(s_LIBXML_PARSEHUGE, XML_PARSE_HUGE);
      cnsInt(s_LIBXML_NOEMPTYTAG, LIBXML_SAVE_NOEMPTYTAG);

      // Schema validation options
#if defined(LIBXML_SCHEMAS_ENABLED)
      cnsInt(s_LIBXML_SCHEMA_CREATE, XML_SCHEMA_VAL_VC_I_CREATE);
#endif

      // Additional constants for use with loading html
#if LIBXML_VERSION >= 20707
      cnsInt(s_LIBXML_HTML_NOIMPLIED, HTML_PARSE_NOIMPLIED);
#endif

#if LIBXML_VERSION >= 20708
      cnsInt(s_LIBXML_HTML_NODEFDTD, HTML_PARSE_NODEFDTD);
#endif

      // Error levels
      cnsInt(s_LIBXML_ERR_NONE, XML_ERR_NONE);
      cnsInt(s_LIBXML_ERR_WARNING, XML_ERR_WARNING);
      cnsInt(s_LIBXML_ERR_ERROR, XML_ERR_ERROR);
      cnsInt(s_LIBXML_ERR_FATAL, XML_ERR_FATAL);

      HHVM_FE(libxml_get_errors);
      HHVM_FE(libxml_get_last_error);
      HHVM_FE(libxml_clear_errors);
      HHVM_FE(libxml_use_internal_errors);
      HHVM_FE(libxml_disable_entity_loader);
      HHVM_FE(libxml_set_streams_context);

      loadSystemlib();

      s_LibXMLError_class = Unit::lookupClass(s_LibXMLError.get());
      xmlParserInputBufferCreateFilenameDefault(libxml_create_input_buffer);
      xmlOutputBufferCreateFilenameDefault(libxml_create_output_buffer);
    }

    void requestInit() override {
      xmlResetLastError();
    }

} s_libxml_extension;

///////////////////////////////////////////////////////////////////////////////
}
