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

#include "hphp/runtime/ext/libxml/ext_libxml.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/root-map.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/ext/std/ext_std_file.h"

#include <folly/FBVector.h>

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

TRACE_SET_MOD(libxml);

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct xmlErrorVec : folly::fbvector<xmlError> {
  ~xmlErrorVec() {
    clearErrors();
  }

  void reset() {
    clearErrors();
    clear();
  }

private:
  void clearErrors() {
    for (auto& error : *this) {
      xmlResetError(&error);
    }
  }
};

struct LibXmlRequestData final : RequestEventHandler {
  void requestInit() override {
    m_use_error = false;
    m_suppress_error = false;
    m_errors = xmlErrorVec();
    m_entity_loader_disabled = false;
    m_streams_context = nullptr;
    m_streams.reset();
  }

  void requestShutdown() override {
    m_use_error = false;
    m_errors = xmlErrorVec();
    m_streams_context = nullptr;
    m_streams.reset();
  }

  bool m_entity_loader_disabled;
  bool m_suppress_error;
  bool m_use_error;
  xmlErrorVec m_errors;
  req::ptr<StreamContext> m_streams_context;
  RootMap<File> m_streams;
};

IMPLEMENT_STATIC_REQUEST_LOCAL(LibXmlRequestData, tl_libxml_request_data);

namespace {

// This function takes ownership of a req::ptr<File> and returns
// a void* token that can be used to lookup the File later.  This
// is so a reference to the file can be stored in an XML context
// object as a void*.  The set of remembered files is cleared out
// at request shutdown. The ext_libxml extension is the only
// XML extension that should be storing streams as roots,
// since it has no other place to safely store a req::ptr.
// The other XML extensions either own the req::ptr<File> locally
// or are able to store it in a object.
inline void* rememberStream(req::ptr<File>&& stream) {
  return reinterpret_cast<void*>(
    tl_libxml_request_data->m_streams.addRoot(std::move(stream))
  );
}

// This function returns the File associated with the given token.
// If the token is not in the m_streams map, it means a pointer to
// the File has been stored directly in the XML context.
inline req::ptr<File> getStream(void* userData) {
  auto file = tl_libxml_request_data->m_streams.lookupRoot(userData);
  return file ? file : *reinterpret_cast<req::ptr<File>*>(userData);
}

// This closes and deletes the File associated with the given token.
// It is used by the XML callback that destroys a context.
inline bool forgetStream(void* userData) {
  auto ptr = tl_libxml_request_data->m_streams.removeRoot(userData);
  return ptr->close();
}

}

static Class* s_LibXMLError_class;

const StaticString
  s_LibXMLError("LibXMLError"),
  s_level("level"),
  s_code("code"),
  s_column("column"),
  s_message("message"),
  s_file("file"),
  s_line("line");

///////////////////////////////////////////////////////////////////////////////

static void php_libxml_node_free_resource(xmlNodePtr node, bool force);

void XMLNodeData::sweep() {
  if (m_node) {
    assertx(this == m_node->_private);

    m_node->_private = nullptr;
    php_libxml_node_free_resource(m_node, true);
  }

  if (m_doc) m_doc->detachNode();
}

void XMLDocumentData::cleanup() {
  assertx(!m_liveNodes);
  auto docp = (xmlDocPtr)m_node;
  if (docp->URL) {
    xmlFree((void*)docp->URL);
    docp->URL = nullptr;
  }
  xmlFreeDoc(docp);

  m_node = nullptr; // don't let XMLNode try to cleanup
}

void XMLDocumentData::sweep() {
  if (!m_liveNodes) {
    cleanup();
  }
  m_destruct = true;
  if (m_doc) m_doc->detachNode();
}

///////////////////////////////////////////////////////////////////////////////
// Callbacks and helpers
//
// Note that these stream callbacks may re-enter the VM via a user-defined
// stream wrapper. The VM state should be synced using VMRegGuard by the
// caller, before entering libxml2.

static req::ptr<File> libxml_streams_IO_open_wrapper(
    const char *filename, const char* mode, bool read_only)
{
  ITRACE(1, "libxml_open_wrapper({}, {}, {})\n", filename, mode, read_only);
  Trace::Indent _i;

  auto strFilename = String::attach(StringData::Make(filename, CopyString));
  /* FIXME: PHP calls stat() here if the wrapper has a non-null stat handler,
   * in order to skip the open of a missing file, thus suppressing warnings.
   * Our stat handlers are virtual, so there's no easy way to tell if stat
   * is supported, so instead we will just call stat() for plain files, since
   * of the default transports, only plain files have support for stat().
   */
  if (read_only) {
    int pathIndex = 0;
    Stream::Wrapper* wrapper = Stream::getWrapperFromURI(strFilename,
                                                         &pathIndex);
    if (wrapper->isNormalFileStream()) {
      if (!HHVM_FN(file_exists)(strFilename)) {
        return nullptr;
      }
    }
  }

  // PHP unescapes the URI here, but that should properly be done by the
  // wrapper.  The wrapper should expect a valid URI, e.g. file:///foo%20bar
  return File::Open(strFilename, mode, 0,
                    tl_libxml_request_data->m_streams_context);
}

int libxml_streams_IO_read(void* context, char* buffer, int len) {
  ITRACE(1, "libxml_IO_read({}, {}, {})\n", context, (void*)buffer, len);
  Trace::Indent _i;

  auto stream = getStream(context);
  assertx(len >= 0);
  if (len > 0) {
    String str = stream->read(len);
    if (str.size() <= len) {
      std::memcpy(buffer, str.data(), str.size());
      return str.size();
    }
  }

  return -1;
}

int libxml_streams_IO_write(void* context, const char* buffer, int len) {
  ITRACE(1, "libxml_IO_write({}, {}, {})\n", context, (void*)buffer, len);
  Trace::Indent _i;

  auto stream = getStream(context);
  int64_t ret = stream->write(String(buffer, len, CopyString));
  return (ret < INT_MAX) ? ret : -1;
}

int libxml_streams_IO_close(void* context) {
  ITRACE(1, "libxml_IO_close({}), sweeping={}\n",
         context, MemoryManager::sweeping());
  Trace::Indent _i;

  if (MemoryManager::sweeping()) {
    // Stream wrappers close themselves on sweep, so there's nothing to do here
    return 0;
  }

  return forgetStream(context) ? 0 : -1;
}

int libxml_streams_IO_nop_close(void* /*context*/) {
  return 0;
}

static xmlExternalEntityLoader s_default_entity_loader = nullptr;

/*
 * A whitelist of protocols allowed to be use in xml external entities. Note
 * that accesses to this set are not synchronized, so it must not be modified
 * after module initialization.
 */
static std::unordered_set<
  const StringData*,
  string_data_hash,
  string_data_isame
> s_ext_entity_whitelist;

static bool allow_ext_entity_protocol(const String& protocol) {
  return s_ext_entity_whitelist.count(protocol.get());
}

static xmlParserInputPtr libxml_ext_entity_loader(const char *url,
                                                  const char *id,
                                                  xmlParserCtxtPtr context) {
  ITRACE(1, "libxml_ext_entity_loader({}, {}, {})\n",
         url, id, (void*)context);
  Trace::Indent _i;

  auto protocol = Stream::getWrapperProtocol(url);
  if (!allow_ext_entity_protocol(protocol)) {
    raise_warning("Protocol '%s' for external XML entity '%s' is disabled for"
                  " security reasons. This may be changed using the"
                  " hhvm.libxml.ext_entity_whitelist ini setting.",
                  protocol.c_str(), url);
    return nullptr;
  }

  return s_default_entity_loader(url, id, context);
}

static xmlParserInputBufferPtr
libxml_create_input_buffer(const char* URI, xmlCharEncoding enc) {
  ITRACE(1, "libxml_create_input_buffer({}, {})\n", URI, static_cast<int>(enc));
  Trace::Indent _i;

 if (tl_libxml_request_data->m_entity_loader_disabled || !URI) return nullptr;

  auto stream = libxml_streams_IO_open_wrapper(URI, "rb", true);
  if (!stream || stream->isInvalid()) return nullptr;

  // Allocate the Input buffer front-end.
  xmlParserInputBufferPtr ret = xmlAllocParserInputBuffer(enc);
  if (ret != nullptr) {
    ret->context = rememberStream(std::move(stream));
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
  ITRACE(1, "libxml_create_output_buffer({}, {}, {})\n",
         URI, (void*)encoder, compression);
  Trace::Indent _i;

  if (URI == nullptr) {
    return nullptr;
  }
  // PHP unescapes the URI here, but that should properly be done by the
  // wrapper.  The wrapper should expect a valid URI, e.g. file:///foo%20bar
  auto stream = libxml_streams_IO_open_wrapper(URI, "wb", false);
  if (!stream || stream->isInvalid()) {
    return nullptr;
  }
  // Allocate the Output buffer front-end.
  xmlOutputBufferPtr ret = xmlAllocOutputBuffer(encoder);
  if (ret != nullptr) {
    ret->context = rememberStream(std::move(stream));
    ret->writecallback = libxml_streams_IO_write;
    ret->closecallback = libxml_streams_IO_close;
  }

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool libxml_use_internal_error() {
  return tl_libxml_request_data->m_use_error;
}

void libxml_add_error(const std::string &msg) {
  if (tl_libxml_request_data->m_suppress_error) {
    return;
  }
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

void php_libxml_node_free(xmlNodePtr node, bool force) {
  if (node) {
    if (node->_private) {
      // When running in Eval.LibXMLUseSafeSubtrees, linked nodes should only be
      // freed out from under their resources when in requestShutdown() or
      // sweeping. In all other cases release should be deferred.
      assertx(!RuntimeOption::EvalLibXMLUseSafeSubtrees || force);

      // XXX: we may be sweeping- so don't create a smart pointer
      reinterpret_cast<XMLNodeData*>(node->_private)->reset();
    }
    switch (node->type) {
    case XML_ATTRIBUTE_NODE:
      xmlFreeProp((xmlAttrPtr) node);
      break;
    case XML_ENTITY_DECL:
    case XML_ELEMENT_DECL:
    case XML_ATTRIBUTE_DECL:
      break;
    case XML_NOTATION_NODE:
      /* These require special handling */
      if (node->name != NULL) {
        xmlFree((char *) node->name);
      }
      if (((xmlEntityPtr) node)->ExternalID != NULL) {
        xmlFree((char *) ((xmlEntityPtr) node)->ExternalID);
      }
      if (((xmlEntityPtr) node)->SystemID != NULL) {
        xmlFree((char *) ((xmlEntityPtr) node)->SystemID);
      }
      xmlFree(node);
      break;
    case XML_NAMESPACE_DECL:
      if (node->ns) {
        xmlFreeNs(node->ns);
        node->ns = NULL;
      }
      node->type = XML_ELEMENT_NODE;
    default:
      xmlFreeNode(node);
    }
  }
}

namespace {

template<class F1, class F2>
void walk_tree(xmlNodePtr node, F1 preaction, F2 postaction) {
  xmlNodePtr curnode;

  if (node != nullptr) {
    curnode = node;
    while (curnode != nullptr) {
      node = curnode;
      preaction(node);
      switch (node->type) {
      /* Skip property freeing for the following types */
      case XML_NOTATION_NODE:
      case XML_ENTITY_DECL:
        break;
      case XML_ENTITY_REF_NODE:
        walk_tree((xmlNodePtr) node->properties, preaction, postaction);
        break;
      case XML_ATTRIBUTE_NODE:
      case XML_ATTRIBUTE_DECL:
      case XML_DTD_NODE:
      case XML_DOCUMENT_TYPE_NODE:
      case XML_NAMESPACE_DECL:
      case XML_TEXT_NODE:
        walk_tree(node->children, preaction, postaction);
        break;
      default:
        walk_tree(node->children, preaction, postaction);
        walk_tree((xmlNodePtr) node->properties, preaction, postaction);
      }

      curnode = node->next;
      postaction(node);
    }
  }
}

void php_libxml_node_free_list(xmlNodePtr node, bool force) {
  walk_tree(
    node,
    [&] (xmlNodePtr node) {
      if (node->type == XML_ATTRIBUTE_NODE &&
          node->doc &&
          ((xmlAttrPtr)node)->atype == XML_ATTRIBUTE_ID) {
        xmlRemoveID(node->doc, (xmlAttrPtr) node);
      }
    },
    [&] (xmlNodePtr node) {
      xmlUnlinkNode(node);
      php_libxml_node_free(node, force);
    }
  );
}

bool isOrphanedRoot(xmlNodePtr node) {
  return !node->_private && (!node->parent || node->type == XML_NAMESPACE_DECL);
}

}

struct LibXmlDeferredTrees;
THREAD_LOCAL(LibXmlDeferredTrees, tl_libxml_trees);

// Unfortunately this struct can't be declared with internal linkage as it
// contains pointers to request allocated resources
struct LibXmlDeferredTrees final {
  ~LibXmlDeferredTrees() {
    // We won't have access to the list of orphaned tree-roots while sweeping,
    // they need to be dealt with now. We can't just walk the list because some
    // of these nodes may actually be in the same tree so first find the ones
    // that are definitely orphaned (and therefore the roots of their trees)
    std::vector<xmlNodePtr> toFree;
    for (auto par : m_refCounts) {
      if (isOrphanedRoot(par.first)) toFree.push_back(par.first);
    }

    for (auto node : toFree) php_libxml_node_free_resource(node, true);
  }

  static void decref(xmlNodePtr root) {
    if (!root) return;

    auto it = tl_libxml_trees->m_refCounts.find(root);
    assertx(it != tl_libxml_trees->m_refCounts.end() && it->second > 0);
    if (!--it->second) {
      tl_libxml_trees->m_refCounts.erase(it);

      // There may be new undiscovered roots, this free will find them and
      // re-add the root to the deferred list.
      if (isOrphanedRoot(root)) php_libxml_node_free_resource(root, false);
    }
  }

  static bool hasRefs(xmlNodePtr root) {
    if (!RuntimeOption::EvalLibXMLUseSafeSubtrees) return false;

    // If we are cleaning up the request then all roots must be freed, don't
    // bother with additional work. Callers of php_libxml_node_free_resource
    // should force unconditional cleanup.
    assertx(!MemoryManager::sweeping());

    {
      auto it = tl_libxml_trees->m_refCounts.find(root);
      if (it != tl_libxml_trees->m_refCounts.end()) {
        assertx(it->second != 0);
        return true;
      }
    }
    uint32_t count = 0;
    walk_tree(
      root,
      [&] (xmlNodePtr node) {
        if (node->_private) {
          // If rootOf(node) == root then we would have found a refcount above
          assertx(rootOf(node) != root);
          decref(rootOf(node));
          rootOf(node) = root;
          ++count;
        }
      },
      [&] (xmlNodePtr node) {}
    );
    if (!count) return false;
    tl_libxml_trees->m_refCounts.emplace(root, count);
    return true;
  }

private:
  static xmlNodePtr& rootOf(xmlNodePtr node) {
    assertx(node->_private);
    return reinterpret_cast<XMLNodeData*>(node->_private)->m_lastSeenRoot;
  }

  req::fast_map<xmlNodePtr,uint32_t> m_refCounts;
};

static void php_libxml_node_free_resource(xmlNodePtr node, bool force) {
  // If we are sweeping or otherwise iterating the list of roots or ref counts
  // it is unsafe to perform hasRefs as that may allocate a new tl_libxml_trees
  // or invalidate an active iterator. When called with force we are always
  // shutting down and therefore about to lose track of any stored root data
  // so the loss of consistency is fine.
  assertx(!MemoryManager::sweeping() || force);
  if (!isOrphanedRoot(node)) return;

  // If we are running in Eval.LibXMLUseSafeSubtrees mode and a subtree still
  // holds a reference to the root then don't do anything here.
  if (!force && LibXmlDeferredTrees::hasRefs(node)) {
    // We shouldn't have even attempted to free the document until all of its
    // children have been released (it's shared/reference counted).
    assertx(node->type != XML_DOCUMENT_NODE);
    assertx(node->type != XML_HTML_DOCUMENT_NODE);
    return;
  }

  if (node) {
    switch (node->type) {
    case XML_DOCUMENT_NODE:
    case XML_HTML_DOCUMENT_NODE:
      break;
    default:
      assertx(isOrphanedRoot(node));
      php_libxml_node_free_list((xmlNodePtr) node->children, force);
      switch (node->type) {
      /* Skip property freeing for the following types */
      case XML_ATTRIBUTE_DECL:
      case XML_DTD_NODE:
      case XML_DOCUMENT_TYPE_NODE:
      case XML_ENTITY_DECL:
      case XML_ATTRIBUTE_NODE:
      case XML_NAMESPACE_DECL:
      case XML_TEXT_NODE:
        break;
      default:
        php_libxml_node_free_list((xmlNodePtr) node->properties, force);
      }
      php_libxml_node_free(node, force);
    }
  }
}

void php_libxml_node_free_resource(xmlNodePtr node, xmlNodePtr root) {
  php_libxml_node_free_resource(node, false);
  LibXmlDeferredTrees::decref(root);
}

String libxml_get_valid_file_path(const char* source) {
  return libxml_get_valid_file_path(String(source, CopyString));
}

String libxml_get_valid_file_path(const String& source) {
  bool isFileUri = false;
  bool isUri = false;

  String file_dest(source);

  Url url;
  if (url_parse(url, file_dest.data(), file_dest.size())) {
    isUri = true;
    if (url.scheme.same(s_file)) {
      file_dest = StringUtil::UrlDecode(url.path, false);
      isFileUri = true;
    }
  }

  if (url.scheme.empty() && (!isUri || isFileUri)) {
    file_dest = File::TranslatePath(file_dest);
  }
  return file_dest;
}

static void libxml_error_handler(void* /*userData*/, xmlErrorPtr error) {
  if (tl_libxml_request_data->m_suppress_error) {
    return;
  }
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
  Object ret{s_LibXMLError_class};
  ret->setProp(nullptr, s_level.get(), make_tv<KindOfInt64>(error.level));
  ret->setProp(nullptr, s_code.get(), make_tv<KindOfInt64>(error.code));
  ret->setProp(nullptr, s_column.get(), make_tv<KindOfInt64>(error.int2));
  if (error.message) {
    String message(error.message);
    ret->setProp(nullptr, s_message.get(), message.toCell());
  } else {
    ret->setProp(nullptr, s_message.get(), make_tv<KindOfNull>());
  }
  if (error.file) {
    String file(error.file);
    ret->setProp(nullptr, s_file.get(), file.toCell());
  } else {
    ret->setProp(nullptr, s_file.get(), make_tv<KindOfNull>());
  }
  ret->setProp(nullptr, s_line.get(), make_tv<KindOfInt64>(error.line));
  return ret;
}

Array HHVM_FUNCTION(libxml_get_errors) {
  xmlErrorVec* error_list = &tl_libxml_request_data->m_errors;
  const auto length = error_list->size();
  if (!length) {
    return empty_array();
  }
  PackedArrayInit ret(length);
  for (int64_t i = 0; i < length; i++) {
    ret.append(create_libxmlerror(error_list->at(i)));
  }
  return ret.toArray();
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
    tl_libxml_request_data->m_suppress_error = false;
    tl_libxml_request_data->m_errors.reset();
  } else {
    xmlSetStructuredErrorFunc(nullptr, libxml_error_handler);
    tl_libxml_request_data->m_use_error = true;
    tl_libxml_request_data->m_suppress_error = false;
  }
  return ret;
}

void HHVM_FUNCTION(libxml_suppress_errors, bool suppress_errors) {
  tl_libxml_request_data->m_suppress_error = suppress_errors;
}

bool HHVM_FUNCTION(libxml_disable_entity_loader, bool disable /* = true */) {
  bool old = tl_libxml_request_data->m_entity_loader_disabled;

  tl_libxml_request_data->m_entity_loader_disabled = disable;

  return old;
}

void HHVM_FUNCTION(libxml_set_streams_context, const Resource & context) {
  tl_libxml_request_data->m_streams_context =
    dyn_cast_or_null<StreamContext>(context);
}

///////////////////////////////////////////////////////////////////////////////
// Extension

struct LibXMLExtension final : Extension {
    LibXMLExtension() : Extension("libxml") {}

    void moduleLoad(const IniSetting::Map& ini, Hdf config) override {

      // Grab the external entity whitelist and set up the map, then register
      // the callback for external entity loading. data: is always supported
      // since it doesn't reference data outside of the current document.
      std::vector<std::string> whitelist;
      auto whitelistStr = Config::GetString(ini, config,
                                            "Eval.Libxml.ExtEntityWhitelist");
      folly::split(',', whitelistStr, whitelist, true);

      s_ext_entity_whitelist.reserve(1 + whitelist.size());
      s_ext_entity_whitelist.insert(makeStaticString("data"));
      for (auto const& str : whitelist) {
        s_ext_entity_whitelist.insert(makeStaticString(str));
      }
    }

    void moduleInit() override {
      HHVM_RC_INT_SAME(LIBXML_VERSION);
      HHVM_RC_STR_SAME(LIBXML_DOTTED_VERSION);
      HHVM_RC_STR(LIBXML_LOADED_VERSION, xmlParserVersion);

      // For use with loading xml
      HHVM_RC_INT(LIBXML_NOENT, XML_PARSE_NOENT);
      HHVM_RC_INT(LIBXML_DTDLOAD, XML_PARSE_DTDLOAD);
      HHVM_RC_INT(LIBXML_DTDATTR, XML_PARSE_DTDATTR);
      HHVM_RC_INT(LIBXML_DTDVALID, XML_PARSE_DTDVALID);
      HHVM_RC_INT(LIBXML_NOERROR, XML_PARSE_NOERROR);
      HHVM_RC_INT(LIBXML_NOWARNING, XML_PARSE_NOWARNING);
      HHVM_RC_INT(LIBXML_NOBLANKS, XML_PARSE_NOBLANKS);
      HHVM_RC_INT(LIBXML_XINCLUDE, XML_PARSE_XINCLUDE);
      HHVM_RC_INT(LIBXML_NSCLEAN, XML_PARSE_NSCLEAN);
      HHVM_RC_INT(LIBXML_NOCDATA, XML_PARSE_NOCDATA);
      HHVM_RC_INT(LIBXML_NONET, XML_PARSE_NONET);
      HHVM_RC_INT(LIBXML_PEDANTIC, XML_PARSE_PEDANTIC);
      HHVM_RC_INT(LIBXML_COMPACT, XML_PARSE_COMPACT);
      HHVM_RC_INT(LIBXML_NOXMLDECL, XML_SAVE_NO_DECL);
      HHVM_RC_INT(LIBXML_PARSEHUGE, XML_PARSE_HUGE);
      HHVM_RC_INT(LIBXML_NOEMPTYTAG, LIBXML_SAVE_NOEMPTYTAG);

      // Schema validation options
#if defined(LIBXML_SCHEMAS_ENABLED)
      HHVM_RC_INT(LIBXML_SCHEMA_CREATE, XML_SCHEMA_VAL_VC_I_CREATE);
#endif

      // Additional constants for use with loading html
#if LIBXML_VERSION >= 20707
      HHVM_RC_INT(LIBXML_HTML_NOIMPLIED, HTML_PARSE_NOIMPLIED);
#endif

#if LIBXML_VERSION >= 20708
      HHVM_RC_INT(LIBXML_HTML_NODEFDTD, HTML_PARSE_NODEFDTD);
#endif

      // Error levels
      HHVM_RC_INT(LIBXML_ERR_NONE, XML_ERR_NONE);
      HHVM_RC_INT(LIBXML_ERR_WARNING, XML_ERR_WARNING);
      HHVM_RC_INT(LIBXML_ERR_ERROR, XML_ERR_ERROR);
      HHVM_RC_INT(LIBXML_ERR_FATAL, XML_ERR_FATAL);

      HHVM_FE(libxml_get_errors);
      HHVM_FE(libxml_get_last_error);
      HHVM_FE(libxml_clear_errors);
      HHVM_FE(libxml_use_internal_errors);
      HHVM_FE(libxml_suppress_errors);
      HHVM_FE(libxml_disable_entity_loader);
      HHVM_FE(libxml_set_streams_context);

      loadSystemlib();

      s_LibXMLError_class = Unit::lookupClass(s_LibXMLError.get());

      // Set up callbacks to support stream wrappers for reading and writing
      // xml files and loading external entities.
      xmlParserInputBufferCreateFilenameDefault(libxml_create_input_buffer);
      xmlOutputBufferCreateFilenameDefault(libxml_create_output_buffer);
      s_default_entity_loader = xmlGetExternalEntityLoader();
      xmlSetExternalEntityLoader(libxml_ext_entity_loader);

      // These callbacks will fallback to alternate defaults in a threaded
      // context, we want to always use these functions.
      xmlThrDefParserInputBufferCreateFilenameDefault(
        libxml_create_input_buffer
      );
      xmlThrDefOutputBufferCreateFilenameDefault(libxml_create_output_buffer);
    }

    void requestInit() override {
      assertx(tl_libxml_trees.isNull());
      xmlResetLastError();
    }

    void requestShutdown() override {
      tl_libxml_trees.destroy();
    }

} s_libxml_extension;

///////////////////////////////////////////////////////////////////////////////
}
