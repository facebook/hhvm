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

#include "hphp/runtime/ext/domdocument/ext_domdocument.h"

#include <map>

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/ext/simplexml/ext_simplexml.h"
#include "hphp/runtime/ext/std/ext_std_classobj.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/ext/std/ext_std_file.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/string/ext_string.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/functional.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/string-vsnprintf.h"

#define DOM_XMLNS_NAMESPACE                             \
  (const xmlChar *) "http://www.w3.org/2000/xmlns/"

#define DOM_LOAD_STRING 0
#define DOM_LOAD_FILE 1

#define PHP_DOM_XPATH_QUERY 0
#define PHP_DOM_XPATH_EVALUATE 1
#define DOM_NODESET XML_XINCLUDE_START

namespace HPHP {

IMPLEMENT_DEFAULT_EXTENSION_VERSION(dom, 20031129);

///////////////////////////////////////////////////////////////////////////////
// parser error handling

#define PHP_LIBXML_CTX_ERROR 1
#define PHP_LIBXML_CTX_WARNING 2

#ifndef LIBXML2_NEW_BUFFER
# define xmlOutputBufferGetSize(buf)    ((buf)->buffer->use)
# define xmlOutputBufferGetContent(buf) ((buf)->buffer->content)
#endif

// defined in ext_simplexml.cpp
extern xmlNodePtr SimpleXMLElement_exportNode(const Object& sxe);

#define IMPLEMENT_CLASS_LOADER(CLASS)                                          \
struct CLASS : SystemLib::ClassLoader<#CLASS> {};

IMPLEMENT_CLASS_LOADER(DOMAttr)
IMPLEMENT_CLASS_LOADER(DOMCharacterData)
IMPLEMENT_CLASS_LOADER(DOMComment)
IMPLEMENT_CLASS_LOADER(DOMText)
IMPLEMENT_CLASS_LOADER(DOMCdataSection)
IMPLEMENT_CLASS_LOADER(DOMDocument)
IMPLEMENT_CLASS_LOADER(DOMDocumentFragment)
IMPLEMENT_CLASS_LOADER(DOMDocumentType)
IMPLEMENT_CLASS_LOADER(DOMEntity)
IMPLEMENT_CLASS_LOADER(DOMEntityReference)
IMPLEMENT_CLASS_LOADER(DOMNotation)
IMPLEMENT_CLASS_LOADER(DOMProcessingInstruction)
IMPLEMENT_CLASS_LOADER(DOMNameSpaceNode)
IMPLEMENT_CLASS_LOADER(DOMNamedNodeMap)
IMPLEMENT_CLASS_LOADER(DOMNodeList)
IMPLEMENT_CLASS_LOADER(DOMImplementation)

static void php_libxml_internal_error_handler(int error_type, void *ctx,
                      ATTRIBUTE_PRINTF_STRING const char *fmt,
                                              va_list ap) ATTRIBUTE_PRINTF(3,0);
static void php_libxml_internal_error_handler(int error_type, void *ctx,
                                              const char *fmt,
                                              va_list ap) {
  std::string msg;
  string_vsnprintf(msg, fmt, ap);

  /* remove any trailing \n */
  while (!msg.empty() && msg[msg.size() - 1] == '\n') {
    msg = msg.substr(0, msg.size() - 1);
  }
  if (libxml_use_internal_error()) {
    libxml_add_error(msg);
    return;
  }

  xmlParserCtxtPtr parser = (xmlParserCtxtPtr) ctx;
  if (parser != nullptr && parser->input != nullptr) {
    if (parser->input->filename) {
      switch (error_type) {
      case PHP_LIBXML_CTX_ERROR:
        raise_warning("%s in %s, line: %d", msg.c_str(),
                      parser->input->filename, parser->input->line);
        break;
      case PHP_LIBXML_CTX_WARNING:
        raise_notice("%s in %s, line: %d", msg.c_str(),
                     parser->input->filename, parser->input->line);
        break;
      default:
        assertx(false);
        break;
      }
    } else {
      switch (error_type) {
      case PHP_LIBXML_CTX_ERROR:
        raise_warning("%s in Entity, line: %d", msg.c_str(),
                      parser->input->line);
        break;
      case PHP_LIBXML_CTX_WARNING:
        raise_notice("%s in Entity, line: %d", msg.c_str(),
                     parser->input->line);
        break;
      default:
        assertx(false);
        break;
      }
    }
  } else {
    switch (error_type) {
    case PHP_LIBXML_CTX_ERROR:
      raise_warning("%s", msg.c_str());
      break;
    case PHP_LIBXML_CTX_WARNING:
      raise_notice("%s", msg.c_str());
      break;
    default:
      assertx(false);
      break;
    }
  }
}

/**
 * The error handler callbacks below are called from libxml code that
 * is compiled without frame pointers, so it's necessary to use a
 * VMRegGuard before calling libxml code that uses these error handler
 * callbacks.
 */

static void php_libxml_ctx_error(void *ctx,
  ATTRIBUTE_PRINTF_STRING const char *msg, ...) ATTRIBUTE_PRINTF(2,3);
static void php_libxml_ctx_error(void *ctx,
                          const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  try {
    php_libxml_internal_error_handler(PHP_LIBXML_CTX_ERROR, ctx, msg, args);
  } catch (...) {}
  va_end(args);
}

static void php_libxml_ctx_warning(void *ctx,
    ATTRIBUTE_PRINTF_STRING const char *msg, ...) ATTRIBUTE_PRINTF(2,3);
static void php_libxml_ctx_warning(void *ctx,
                            const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  try {
    php_libxml_internal_error_handler(PHP_LIBXML_CTX_WARNING, ctx, msg, args);
  } catch (...) {}
  va_end(args);
}


///////////////////////////////////////////////////////////////////////////////

// domexception errors
enum dom_exception_code {
  PHP_ERR                        = 0, // non-spec code for PHP errors
  INDEX_SIZE_ERR                 = 1,
  DOMSTRING_SIZE_ERR             = 2,
  HIERARCHY_REQUEST_ERR          = 3,
  WRONG_DOCUMENT_ERR             = 4,
  INVALID_CHARACTER_ERR          = 5,
  NO_DATA_ALLOWED_ERR            = 6,
  NO_MODIFICATION_ALLOWED_ERR    = 7,
  NOT_FOUND_ERR                  = 8,
  NOT_SUPPORTED_ERR              = 9,
  INUSE_ATTRIBUTE_ERR            = 10,
  INVALID_STATE_ERR              = 11,
  SYNTAX_ERR                     = 12, // Introduced in DOM Level 2
  INVALID_MODIFICATION_ERR       = 13, // Introduced in DOM Level 2
  NAMESPACE_ERR                  = 14, // Introduced in DOM Level 2
  INVALID_ACCESS_ERR             = 15, // Introduced in DOM Level 2
  VALIDATION_ERR                 = 16, // Introduced in DOM Level 3
};

static void php_dom_throw_error(dom_exception_code error_code,
                                bool strict_error) {
  const char *error_message;
  switch (error_code) {
  case INDEX_SIZE_ERR:
    error_message = "Index Size Error";
    break;
  case DOMSTRING_SIZE_ERR:
    error_message = "DOM String Size Error";
    break;
  case HIERARCHY_REQUEST_ERR:
    error_message = "Hierarchy Request Error";
    break;
  case WRONG_DOCUMENT_ERR:
    error_message = "Wrong Document Error";
    break;
  case INVALID_CHARACTER_ERR:
    error_message = "Invalid Character Error";
    break;
  case NO_DATA_ALLOWED_ERR:
    error_message = "No Data Allowed Error";
    break;
  case NO_MODIFICATION_ALLOWED_ERR:
    error_message = "No Modification Allowed Error";
    break;
  case NOT_FOUND_ERR:
    error_message = "Not Found Error";
    break;
  case NOT_SUPPORTED_ERR:
    error_message = "Not Supported Error";
    break;
  case INUSE_ATTRIBUTE_ERR:
    error_message = "Inuse Attribute Error";
    break;
  case INVALID_STATE_ERR:
    error_message = "Invalid State Error";
    break;
  case SYNTAX_ERR:
    error_message = "Syntax Error";
    break;
  case INVALID_MODIFICATION_ERR:
    error_message = "Invalid Modification Error";
    break;
  case NAMESPACE_ERR:
    error_message = "Namespace Error";
    break;
  case INVALID_ACCESS_ERR:
    error_message = "Invalid Access Error";
    break;
  case VALIDATION_ERR:
    error_message = "Validation Error";
    break;
  default:
    error_message = "Unhandled Error";
    break;
  }

  if (strict_error) {
    SystemLib::throwDOMExceptionObject(error_message);
  }
  raise_warning(std::string(error_message));
}

static bool dom_has_feature(const char *feature, const char *version) {
  bool retval = false;
  if (!(strcmp(version, "1.0") && strcmp(version,"2.0") &&
        strcmp(version, ""))) {
    if ((!strcasecmp(feature, "Core") && !strcmp(version, "1.0")) ||
        !strcasecmp(feature, "XML"))
      retval = true;
  }
  return retval;
}

static xmlNode *dom_get_elements_by_tag_name_ns_raw(xmlNodePtr nodep,
                                                    const char *ns,
                                                    const char *local,
                                                    int *cur, int index) {
  xmlNodePtr ret = nullptr;
  while (nodep != nullptr && (*cur <= index || index == -1)) {
    if (nodep->type == XML_ELEMENT_NODE) {
      if (xmlStrEqual(nodep->name, (xmlChar *)local) ||
          xmlStrEqual((xmlChar *)"*", (xmlChar *)local)) {
        if (!ns || !*ns ||
            (nodep->ns &&
             (xmlStrEqual(nodep->ns->href, (xmlChar *)ns) ||
              xmlStrEqual((xmlChar *)"*", (xmlChar *)ns)))) {
          if (*cur == index) {
            ret = nodep;
            break;
          }
          (*cur)++;
        }
      }
      ret = dom_get_elements_by_tag_name_ns_raw(nodep->children, ns, local,
                                                cur, index);
      if (ret != nullptr) {
        break;
      }
    }
    nodep = nodep->next;
  }
  return ret;
}

static void dom_normalize(xmlNodePtr nodep) {
  xmlNodePtr nextp, newnextp;
  xmlAttrPtr attr;
  xmlChar *strContent;
  for (xmlNodePtr child = nodep->children; child; child = child->next) {
    switch (child->type) {
    case XML_TEXT_NODE:
      nextp = child->next;
      while (nextp != nullptr) {
        if (nextp->type == XML_TEXT_NODE) {
          newnextp = nextp->next;
          strContent = xmlNodeGetContent(nextp);
          xmlNodeAddContent(child, strContent);
          xmlFree(strContent);
          libxml_register_node(nextp)->unlink(); // release nextp if unused
          nextp = newnextp;
        } else {
          break;
        }
      }
      break;
    case XML_ELEMENT_NODE:
      dom_normalize(child);
      attr = child->properties;
      while (attr != nullptr) {
        dom_normalize((xmlNodePtr)attr);
        attr = attr->next;
      }
      break;
    case XML_ATTRIBUTE_NODE:
      dom_normalize(child);
      break;
    default:
      break;
    }
  }
}

const StaticString
  s_query("query"),
  s_namespaces("namespaces");

static Variant dom_canonicalization(xmlNodePtr nodep, const String& file,
                                    bool exclusive, bool with_comments,
                                    const Variant& xpath_array,
                                    const Variant& ns_prefixes,
                                    int mode) {
  xmlDocPtr docp;
  xmlNodeSetPtr nodeset = nullptr;
  xmlChar **inclusive_ns_prefixes = nullptr;
  int ret = -1;
  xmlOutputBufferPtr buf;
  xmlXPathContextPtr ctxp=nullptr;
  xmlXPathObjectPtr xpathobjp=nullptr;

  docp = nodep->doc;
  if (!docp) {
    return false;
  }
  if (xpath_array.isNull()) {
    if (nodep->type != XML_DOCUMENT_NODE) {
      ctxp = xmlXPathNewContext(docp);
      ctxp->node = nodep;
      xpathobjp = xmlXPathEvalExpression
        ((xmlChar*)"(.//. | .//@* | .//namespace::*)", ctxp);
      ctxp->node = nullptr;
      if (xpathobjp && xpathobjp->type == XPATH_NODESET) {
        nodeset = xpathobjp->nodesetval;
      } else {
        if (xpathobjp) {
          xmlXPathFreeObject(xpathobjp);
        }
        xmlXPathFreeContext(ctxp);
        return false;
      }
    }
  } else {
    // xpath query from xpath_array
    Array arr = xpath_array.toArray();
    String xquery;
    if (!arr.exists(s_query)) {
      raise_warning("'query' missing from xpath array");
      return false;
    }
    auto const tmp = arr.lookup(s_query);
    if (!isStringType(tmp.type())) {
      raise_warning("'query' is not a string");
      return false;
    }
    xquery = tvCastToString(tmp);
    ctxp = xmlXPathNewContext(docp);
    ctxp->node = nodep;
    if (arr.exists(s_namespaces)) {
      auto const temp = arr.lookup(s_namespaces);
      if (isArrayLikeType(temp.type())) {
        auto ad = temp.val().parr;
        for (ArrayIter it = ArrayIter(ad); it; ++it) {
          Variant prefix = it.first();
          Variant tmpns = it.second();
          if (prefix.isString() || tmpns.isString()) {
            xmlXPathRegisterNs(ctxp, (xmlChar*)prefix.toString().data(),
                               (xmlChar*)tmpns.toString().data());
          }
        }
      }
    }
    xpathobjp = xmlXPathEvalExpression((xmlChar*)xquery.data(), ctxp);
    ctxp->node = nullptr;
    if (xpathobjp && xpathobjp->type == XPATH_NODESET) {
      nodeset = xpathobjp->nodesetval;
    } else {
      if (xpathobjp) {
        xmlXPathFreeObject(xpathobjp);
      }
      xmlXPathFreeContext(ctxp);
      return false;
    }
  }
  if (!ns_prefixes.isNull()) {
    if (exclusive) {
      int nscount = 0;
      inclusive_ns_prefixes = (xmlChar**)malloc
        ((ns_prefixes.toArray().size()+1) * sizeof(xmlChar *));
      for (ArrayIter it = ns_prefixes.toArray().begin(); it; ++it) {
        Variant tmpns = it.second();
        if (tmpns.isString()) {
          inclusive_ns_prefixes[nscount++] = (xmlChar*)tmpns.toString().data();
        }
      }
      inclusive_ns_prefixes[nscount] = nullptr;
    } else {
      raise_notice("Inclusive namespace prefixes only allowed in "
                   "exclusive mode.");
    }
  }
  if (mode == 1) {
    buf = xmlOutputBufferCreateFilename(file.c_str(), nullptr, 0);
  } else {
    buf = xmlAllocOutputBuffer(nullptr);
  }
  if (buf != nullptr) {
    ret = xmlC14NDocSaveTo(docp, nodeset, exclusive, inclusive_ns_prefixes,
                           with_comments, buf);
  }
  if (inclusive_ns_prefixes != nullptr) {
    free(inclusive_ns_prefixes);
  }
  if (xpathobjp != nullptr) {
    xmlXPathFreeObject(xpathobjp);
  }
  if (ctxp != nullptr) {
    xmlXPathFreeContext(ctxp);
  }
  Variant retval;
  if (buf == nullptr || ret < 0) {
    retval = false;
  } else {
    if (mode == 0) {
      ret = xmlOutputBufferGetSize(buf);
      if (ret > 0) {
        retval = String((char *)xmlOutputBufferGetContent(buf), ret, CopyString);
      } else {
        retval = empty_string();
      }
    }
  }
  if (buf) {
    int bytes;
    bytes = xmlOutputBufferClose(buf);
    if (mode == 1 && (ret >= 0)) {
      retval = bytes;
    }
  }
  return retval;
}

static bool dom_node_children_valid(xmlNodePtr node) {
  switch (node->type) {
  case XML_DOCUMENT_TYPE_NODE:
  case XML_DTD_NODE:
  case XML_PI_NODE:
  case XML_COMMENT_NODE:
  case XML_TEXT_NODE:
  case XML_CDATA_SECTION_NODE:
  case XML_NOTATION_NODE:
    return false;
  default:
    break;
  }
  return true;
}

static bool dom_node_is_read_only(xmlNodePtr node) {
  switch (node->type) {
  case XML_ENTITY_REF_NODE:
  case XML_ENTITY_NODE:
  case XML_DOCUMENT_TYPE_NODE:
  case XML_NOTATION_NODE:
  case XML_DTD_NODE:
  case XML_ELEMENT_DECL:
  case XML_ATTRIBUTE_DECL:
  case XML_ENTITY_DECL:
  case XML_NAMESPACE_DECL:
    return true;
  default:
    break;
  }
  return node->doc == nullptr;
}

static void dom_set_old_ns(xmlDoc *doc, xmlNs *ns) {
  xmlNs *cur;
  if (doc == nullptr) return;
  if (doc->oldNs == nullptr) {
    doc->oldNs = (xmlNsPtr) xmlMalloc(sizeof(xmlNs));
    if (doc->oldNs == nullptr) {
      return;
    }
    memset(doc->oldNs, 0, sizeof(xmlNs));
    doc->oldNs->type = XML_LOCAL_NAMESPACE;
    doc->oldNs->href = xmlStrdup(XML_XML_NAMESPACE);
    doc->oldNs->prefix = xmlStrdup((const xmlChar *)"xml");
  }
  cur = doc->oldNs;
  while (cur->next != nullptr) {
    cur = cur->next;
  }
  cur->next = ns;
}

static void dom_reconcile_ns(xmlDocPtr doc, xmlNodePtr nodep) {
  xmlNsPtr nsptr, nsdftptr, curns, prevns = nullptr;
  if (nodep->type == XML_ELEMENT_NODE) {
    // Following if block primarily used for inserting nodes created via
    // createElementNS
    if (nodep->nsDef != nullptr) {
      curns = nodep->nsDef;
      while (curns) {
        nsdftptr = curns->next;
        if (curns->href != nullptr) {
          if ((nsptr = xmlSearchNsByHref(doc, nodep->parent, curns->href)) &&
              (curns->prefix == nullptr ||
               xmlStrEqual(nsptr->prefix, curns->prefix))) {
            curns->next = nullptr;
            if (prevns == nullptr) {
              nodep->nsDef = nsdftptr;
            } else {
              prevns->next = nsdftptr;
            }
            dom_set_old_ns(doc, curns);
            curns = prevns;
          }
        }
        prevns = curns;
        curns = nsdftptr;
      }
    }
    xmlReconciliateNs(doc, nodep);
  }
}

static bool dom_hierarchy(xmlNodePtr parent, xmlNodePtr child) {
  if (parent == nullptr || child == nullptr || child->doc != parent->doc) {
    return true;
  }
  xmlNodePtr nodep = parent;
  while (nodep) {
    if (nodep == child) {
      return false;
    }
    nodep = nodep->parent;
  }
  return true;
}

static xmlNodePtr _php_dom_insert_fragment(xmlNodePtr nodep,
                                           xmlNodePtr prevsib,
                                           xmlNodePtr nextsib,
                                           xmlNodePtr fragment) {
  xmlNodePtr newchild, node;
  newchild = fragment->children;
  if (newchild) {
    if (prevsib == nullptr) {
      nodep->children = newchild;
    } else {
      prevsib->next = newchild;
    }
    newchild->prev = prevsib;
    if (nextsib == nullptr) {
      nodep->last = fragment->last;
    } else {
      fragment->last->next = nextsib;
      nextsib->prev = fragment->last;
    }
    node = newchild;
    while (node != nullptr) {
      node->parent = nodep;
      if (node->doc != nodep->doc) {
        xmlSetTreeDoc(node, nodep->doc);
      }
      if (node == fragment->last) {
        break;
      }
      node = node->next;
    }
    fragment->children = nullptr;
    fragment->last = nullptr;
  }
  return newchild;
}

static int dom_check_qname(const char *qname, char **localname, char **prefix,
                           int uri_len, int name_len) {
  if (name_len == 0) {
    return NAMESPACE_ERR;
  }
  *localname = (char *)xmlSplitQName2((xmlChar *)qname, (xmlChar **) prefix);
  if (*localname == nullptr) {
    *localname = (char *)xmlStrdup((xmlChar *)qname);
    if (*prefix == nullptr && uri_len == 0) {
      return 0;
    }
  }
  if (xmlValidateQName((xmlChar *) qname, 0) != 0) {
    return NAMESPACE_ERR;
  }
  if (*prefix != nullptr && uri_len == 0) {
    return NAMESPACE_ERR;
  }
  return 0;
}

static xmlNsPtr dom_get_ns(xmlNodePtr nodep, const char *uri, int *errorcode,
                           const char *prefix) {
  xmlNsPtr nsptr = nullptr;
  *errorcode = 0;
  if (!((prefix && !strcmp (prefix, "xml") &&
         strcmp(uri, (char *)XML_XML_NAMESPACE)) ||
        (prefix && !strcmp (prefix, "xmlns") &&
         strcmp(uri, (char *)DOM_XMLNS_NAMESPACE)) ||
        (prefix && !strcmp(uri, (char *)DOM_XMLNS_NAMESPACE) &&
         strcmp (prefix, "xmlns")))) {
    nsptr = xmlNewNs(nodep, (xmlChar *)uri, (xmlChar *)prefix);
  }
  if (nsptr == nullptr) {
    *errorcode = NAMESPACE_ERR;
  }
  return nsptr;
}

static xmlDocPtr dom_document_parser(DOMNode* domnode, bool isFile,
                                     const String& source,
                                     int options) {
  VMRegGuard _;

  xmlDocPtr ret = nullptr;
  xmlParserCtxtPtr ctxt = nullptr;

  auto domdoc = domnode->doc();
  bool validate, recover, resolve_externals, keep_blanks, substitute_ent;
  validate = domdoc->m_validateonparse;
  resolve_externals = domdoc->m_resolveexternals;
  keep_blanks = domdoc->m_preservewhitespace;
  substitute_ent = domdoc->m_substituteentities;
  recover = domdoc->m_recover;

  req::ptr<File> stream;

  if (isFile) {
    String file_dest = libxml_get_valid_file_path(source);
    if (!file_dest.empty()) {
      // This is considerably more verbose than just using
      // xmlCreateFileParserCtxt, but it allows us to bypass the external
      // entity loading path, which is locked down by default for security
      // reasons.
      stream = File::Open(file_dest, "rb");
      if (stream && !stream->isInvalid()) {
        // The XML context is also deleted in this function, so the ownership
        // of the File is kept locally in 'stream'.
        // The libxml_streams_IO_nop_close callback does nothing.
        ctxt = xmlCreateIOParserCtxt(nullptr, nullptr,
                                     libxml_streams_IO_read,
                                     libxml_streams_IO_nop_close,
                                     &stream,
                                     XML_CHAR_ENCODING_NONE);
      }
    }
  } else {
    ctxt = xmlCreateMemoryParserCtxt(source.data(), source.size());
  }

  if (ctxt == nullptr) return nullptr;

  /* If loading from memory, we need to set the base directory for the
   * document */
  if (!isFile) {
    String directory = g_context->getCwd();
    if (!directory.empty()) {
      if (ctxt->directory != nullptr) xmlFree(ctxt->directory);

      if (directory[directory.size() - 1] != '/') directory += "/";

      ctxt->directory =
        (char*)xmlCanonicPath((const xmlChar*)directory.c_str());
    }
  }

  ctxt->vctxt.error = php_libxml_ctx_error;
  ctxt->vctxt.warning = php_libxml_ctx_warning;
  if (ctxt->sax != nullptr) {
    ctxt->sax->error = php_libxml_ctx_error;
    ctxt->sax->warning = php_libxml_ctx_warning;
  }

  if (validate && ! (options & XML_PARSE_DTDVALID)) {
    options |= XML_PARSE_DTDVALID;
  }
  if (resolve_externals && ! (options & XML_PARSE_DTDATTR)) {
    options |= XML_PARSE_DTDATTR;
  }
  if (substitute_ent && ! (options & XML_PARSE_NOENT)) {
    options |= XML_PARSE_NOENT;
  }
  if (keep_blanks == 0 && ! (options & XML_PARSE_NOBLANKS)) {
    options |= XML_PARSE_NOBLANKS;
  }
  xmlCtxtUseOptions(ctxt, options);

  ctxt->recovery = recover;
  int old_error_reporting = 0;
  if (recover) {
    old_error_reporting = HHVM_FN(error_reporting)();
    HHVM_FN(error_reporting)(old_error_reporting | (int)ErrorMode::WARNING);
  }

  xmlParseDocument(ctxt);

  if (ctxt->wellFormed || recover) {
    ret = ctxt->myDoc;
    if (ctxt->recovery) {
      HHVM_FN(error_reporting)(old_error_reporting);
    }
    if (ret && ret->URL == nullptr) {
      if (isFile) {
        ret->URL = xmlStrdup((xmlChar*)source.c_str());
      } else {
        /* If loading from memory, set the base reference uri for the
         * document */
        if (ctxt->directory != nullptr) {
          ret->URL = xmlStrdup((xmlChar*)ctxt->directory);
        }
      }
    }
  } else {
    ret = nullptr;
    xmlFreeDoc(ctxt->myDoc);
    ctxt->myDoc = nullptr;
  }

  xmlFreeParserCtxt(ctxt);

  return ret;
}

namespace {

DOMNode* getDOMNode(const Object& node) {
  if (!node.instanceof(DOMNode::classof())) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat(
        "Invalid object. Expected DOMNode, received {}",
        node->getClassName().c_str()
      )
    );
  }
  return Native::data<DOMNode>(node);
}

DOMXPath* getDOMXPath(const Object& obj) {
  if (!obj.instanceof(DOMXPath::classof())) {
    SystemLib::throwInvalidArgumentExceptionObject(
      folly::sformat(
        "Invalid object. Expected DOMXPath, received {}",
        obj->getClassName().c_str()
      )
    );
  }
  return Native::data<DOMXPath>(obj);
}

}

static bool HHVM_METHOD(DOMDocument, _load, const String& source,
                        int64_t options, bool isFile) {
  if (source.empty()) {
    raise_warning("Empty string supplied as input");
    return false;
  }
  if (isFile && !FileUtil::isValidPath(source)) {
    raise_warning("Invalid file source");
    return false;
  }
  auto domdoc = Native::data<DOMNode>(this_);
  auto newdoc = dom_document_parser(domdoc, isFile, source, options);
  if (!newdoc) {
    return false;
  }

  auto olddoc = domdoc->node() ? domdoc->doc() : nullptr;
  domdoc->setNode((xmlNodePtr)newdoc);
  if (olddoc) {
    domdoc->doc()->copyProperties(olddoc);
  }
  return true;
}

static bool HHVM_METHOD(DOMDocument, _loadHTML, const String& source,
                        int64_t options, bool isFile) {
  VMRegGuard _;

  if (source.empty()) {
    raise_warning("Empty string supplied as input");
    return false;
  }

  htmlParserCtxtPtr ctxt;
  if (isFile) {
    if (!FileUtil::isValidPath(source)) {
      raise_warning("Invalid file source");
      return false;
    }

    ctxt = htmlCreateFileParserCtxt(source.data(), nullptr);
  } else {
    ctxt = htmlCreateMemoryParserCtxt(source.data(), source.size());
  }
  if (!ctxt) {
    return false;
  }
  ctxt->vctxt.error = php_libxml_ctx_error;
  ctxt->vctxt.warning = php_libxml_ctx_warning;
  if (ctxt->sax != nullptr) {
    ctxt->sax->error = php_libxml_ctx_error;
    ctxt->sax->warning = php_libxml_ctx_warning;
  }
  if (options) {
    htmlCtxtUseOptions(ctxt, options);
  }
  htmlParseDocument(ctxt);
  xmlDocPtr newdoc = ctxt->myDoc;
  htmlFreeParserCtxt(ctxt);
  if (!newdoc) {
    return false;
  }
  auto domdoc = Native::data<DOMNode>(this_);
  auto olddoc = domdoc->node() ? domdoc->doc() : nullptr;
  domdoc->setNode((xmlNodePtr)newdoc);
  if (olddoc) {
    domdoc->doc()->copyProperties(olddoc);
  }
  return true;
}

static bool _dom_document_relaxNG_validate(DOMNode* domdoc,
                                           const String& source, int type) {
  xmlRelaxNGParserCtxtPtr parser;
  xmlRelaxNGPtr           sptr;
  xmlRelaxNGValidCtxtPtr  vptr;
  int                     is_valid;

  if (source.empty()) {
    raise_warning("Invalid Schema source");
    return false;
  }

  xmlDocPtr docp = (xmlDocPtr)domdoc->nodep();

  switch (type) {
  case DOM_LOAD_FILE:
    {
      String valid_file = libxml_get_valid_file_path(source.data());
      if (valid_file.empty()) {
        raise_warning("Invalid RelaxNG file source");
        return false;
      }
      parser = xmlRelaxNGNewParserCtxt(valid_file.data());
    }
    break;
  case DOM_LOAD_STRING:
    parser = xmlRelaxNGNewMemParserCtxt(source.data(), source.size());
    /* If loading from memory, we need to set the base directory for
       the document. but it is not apparent how to do that for schema's */
    break;
  default:
    return false;
  }

  xmlRelaxNGSetParserErrors
    (parser, (xmlRelaxNGValidityErrorFunc) php_libxml_ctx_error,
     (xmlRelaxNGValidityWarningFunc) php_libxml_ctx_error, nullptr);
  sptr = xmlRelaxNGParse(parser);
  xmlRelaxNGFreeParserCtxt(parser);
  if (!sptr) {
    raise_warning("Invalid RelaxNG");
    return false;
  }

  vptr = xmlRelaxNGNewValidCtxt(sptr);
  if (!vptr) {
    xmlRelaxNGFree(sptr);
    raise_error("Invalid RelaxNG Validation Context");
    return false;
  }

  xmlRelaxNGSetValidErrors(vptr, php_libxml_ctx_error,
                           php_libxml_ctx_error, nullptr);
  is_valid = xmlRelaxNGValidateDoc(vptr, docp);
  xmlRelaxNGFree(sptr);
  xmlRelaxNGFreeValidCtxt(vptr);

  return is_valid == 0;
}

static bool _dom_document_schema_validate(DOMNode* domdoc,
                                          const String& source, int type) {
  xmlDocPtr docp = (xmlDocPtr)domdoc->nodep();
  xmlSchemaParserCtxtPtr  parser;
  xmlSchemaPtr            sptr;
  xmlSchemaValidCtxtPtr   vptr;
  int                     is_valid;

  if (source.empty()) {
    raise_warning("Invalid Schema source");
    return false;
  }

  switch (type) {
  case DOM_LOAD_FILE:
    {
      String valid_file = libxml_get_valid_file_path(source.data());
      if (valid_file.empty()) {
        raise_warning("Invalid Schema file source");
        return false;
      }
      parser = xmlSchemaNewParserCtxt(valid_file.data());
    }
    break;
  case DOM_LOAD_STRING:
    parser = xmlSchemaNewMemParserCtxt(source.data(), source.size());
    /* If loading from memory, we need to set the base directory for
       the document. but it is not apparent how to do that for schema's */
    break;
  default:
    return false;
  }

  xmlSchemaSetParserErrors
    (parser, (xmlSchemaValidityErrorFunc) php_libxml_ctx_error,
    (xmlSchemaValidityWarningFunc) php_libxml_ctx_error, nullptr);
  sptr = xmlSchemaParse(parser);
  xmlSchemaFreeParserCtxt(parser);
  if (!sptr) {
    raise_warning("Invalid Schema");
    return false;
  }

  vptr = xmlSchemaNewValidCtxt(sptr);
  if (!vptr) {
    xmlSchemaFree(sptr);
    raise_error("Invalid Schema Validation Context");
    return false;
  }

  xmlSchemaSetValidErrors(vptr, php_libxml_ctx_error,
                          php_libxml_ctx_error, nullptr);
  is_valid = xmlSchemaValidateDoc(vptr, docp);
  xmlSchemaFree(sptr);
  xmlSchemaFreeValidCtxt(vptr);

  return is_valid == 0;
}

static xmlNodePtr php_dom_free_xinclude_node(xmlNodePtr cur) {
  xmlNodePtr xincnode;
  xincnode = cur;
  cur = cur->next;
  libxml_register_node(xincnode)->unlink(); // release xincnode if unused
  return cur;
}

void php_dom_remove_xinclude_nodes(xmlNodePtr cur) {
  while (cur) {
    if (cur->type == XML_XINCLUDE_START) {
      cur = php_dom_free_xinclude_node(cur);
      /* XML_XINCLUDE_END node will be a sibling of XML_XINCLUDE_START */
      while (cur && cur->type != XML_XINCLUDE_END) {
        /* remove xinclude processing nodes from recursive xincludes */
        if (cur->type == XML_ELEMENT_NODE) {
          php_dom_remove_xinclude_nodes(cur->children);
        }
        cur = cur->next;
      }

      if (cur && cur->type == XML_XINCLUDE_END) {
        cur = php_dom_free_xinclude_node(cur);
      }
    } else {
      if (cur->type == XML_ELEMENT_NODE) {
        php_dom_remove_xinclude_nodes(cur->children);
      }
      cur = cur->next;
    }
  }
}

static void php_dom_xmlSetTreeDoc(xmlNodePtr tree, xmlDocPtr doc) {
  xmlAttrPtr prop;
  xmlNodePtr cur;
  if (tree) {
    if (tree->type == XML_ELEMENT_NODE) {
      prop = tree->properties;
      while (prop != nullptr) {
        prop->doc = doc;
        if (prop->children) {
          cur = prop->children;
          while (cur != nullptr) {
            php_dom_xmlSetTreeDoc(cur, doc);
            cur = cur->next;
          }
        }
        prop = prop->next;
      }
    }
    if (tree->children != nullptr) {
      cur = tree->children;
      while (cur != nullptr) {
        php_dom_xmlSetTreeDoc(cur, doc);
        cur = cur->next;
      }
    }
    tree->doc = doc;
  }
}

static xmlNodePtr dom_get_dom1_attribute(xmlNodePtr elem, xmlChar *name) {
  int len;
  const xmlChar *nqname;
  nqname = xmlSplitQName3(name, &len);
  if (nqname != nullptr) {
    xmlNsPtr ns;
    xmlChar *prefix = xmlStrndup(name, len);
    if (prefix && xmlStrEqual(prefix, (xmlChar*)"xmlns")) {
      ns = elem->nsDef;
      while (ns) {
        if (xmlStrEqual(ns->prefix, nqname)) {
          break;
        }
        ns = ns->next;
      }
      xmlFree(prefix);
      return (xmlNodePtr)ns;
    }
    ns = xmlSearchNs(elem->doc, elem, prefix);
    if (prefix != nullptr) {
      xmlFree(prefix);
    }
    if (ns != nullptr) {
      return (xmlNodePtr)xmlHasNsProp(elem, nqname, ns->href);
    }
  } else {
    if (xmlStrEqual(name, (xmlChar*)"xmlns")) {
      xmlNsPtr nsPtr = elem->nsDef;
      while (nsPtr) {
        if (nsPtr->prefix == nullptr) {
          return (xmlNodePtr)nsPtr;
        }
        nsPtr = nsPtr->next;
      }
      return nullptr;
    }
  }
  return (xmlNodePtr)xmlHasNsProp(elem, name, nullptr);
}

static xmlNsPtr dom_get_nsdecl(xmlNode *node, xmlChar *localName) {
  xmlNsPtr cur;
  xmlNs *ret = nullptr;
  if (node == nullptr) {
    return nullptr;
  }
  if (localName == nullptr || xmlStrEqual(localName, (xmlChar *)"")) {
    cur = node->nsDef;
    while (cur != nullptr) {
      if (cur->prefix == nullptr  && cur->href != nullptr) {
        ret = cur;
        break;
      }
      cur = cur->next;
    }
  } else {
    cur = node->nsDef;
    while (cur != nullptr) {
      if (cur->prefix != nullptr && xmlStrEqual(localName, cur->prefix)) {
        ret = cur;
        break;
      }
      cur = cur->next;
    }
  }
  return ret;
}

static String domClassname(xmlNodePtr obj) {
  switch (obj->type) {
  case XML_DOCUMENT_NODE:
  case XML_HTML_DOCUMENT_NODE: return DOMDocument::className();
  case XML_DTD_NODE:
  case XML_DOCUMENT_TYPE_NODE: return DOMDocumentType::className();
  case XML_ELEMENT_NODE:       return DOMElement::className();
  case XML_ATTRIBUTE_NODE:     return DOMAttr::className();
  case XML_TEXT_NODE:          return DOMText::className();
  case XML_COMMENT_NODE:       return DOMComment::className();
  case XML_PI_NODE:            return DOMProcessingInstruction::className();
  case XML_ENTITY_REF_NODE:    return DOMEntityReference::className();
  case XML_ENTITY_DECL:
  case XML_ELEMENT_DECL:       return DOMEntity::className();
  case XML_CDATA_SECTION_NODE: return DOMCdataSection::className();
  case XML_DOCUMENT_FRAG_NODE: return DOMDocumentFragment::className();
  case XML_NOTATION_NODE:      return DOMNotation::className();
  case XML_NAMESPACE_DECL:     return DOMNameSpaceNode::className();
  default:
    return String((StringData*)nullptr);
  }
}

Variant php_dom_create_object(xmlNodePtr obj,
                              req::ptr<XMLDocumentData> doc) {
  String clsname = domClassname(obj);
  if (!clsname) {
    raise_warning("Unsupported node type: %d", obj->type);
    return init_null();
  }
  if (doc) {
    // Need to lowercase because that is what registerNodeClass does
    // when it adds things to this map.
    auto lower_clsname = HHVM_FN(strtolower)(clsname);
    if (doc->m_classmap.exists(lower_clsname)) {
      // or const char * is not safe
      assertx(doc->m_classmap[lower_clsname].isString());
      clsname = doc->m_classmap[lower_clsname].toString();
    }
  }

  auto node = libxml_register_node(obj);

  Object od = node->getCache()
    ? Object{node->getCache()}
    : Object::attach(g_context->createObjectOnly(clsname.get()));

  auto* nodeobj = Native::data<DOMNode>(od);

  if (!nodeobj->node()) {
    nodeobj->setNode(node);
  }
  assertx(nodeobj->node() == node);

  if (doc) {
    nodeobj->setDoc(std::move(doc));
  }

  return od;
}

static Variant create_node_object(xmlNodePtr node,
                                  req::ptr<XMLDocumentData> doc) {
  if (!node) {
    return init_null();
  }
  Variant retval = php_dom_create_object(node, doc);
  if (retval.isNull()) {
    raise_warning("Cannot create required DOM object");
  }
  return retval;
}

static Variant create_node_object(xmlNodePtr obj) {
  return create_node_object(obj, req::ptr<XMLDocumentData>(nullptr));
}

static Variant create_node_object(xmlNodePtr obj, Object docObj) {
  if (docObj) {
    auto doc = getDOMNode(docObj);
    return create_node_object(obj, doc->doc());
  }
  return create_node_object(obj);
}

static Variant php_xpath_eval(DOMXPath* domxpath, const String& expr,
                              const Object& context, int type,
                              bool registerNodeNS) {
  xmlXPathObjectPtr xpathobjp;
  int nsnbr = 0, xpath_type;
  xmlDoc *docp = nullptr;
  xmlNsPtr *ns = nullptr;
  xmlXPathContextPtr ctxp = (xmlXPathContextPtr)domxpath->m_node;
  if (ctxp == nullptr) {
    raise_warning("Invalid XPath Context");
    return false;
  }
  docp = (xmlDocPtr)ctxp->doc;
  if (docp == nullptr) {
    raise_warning("Invalid XPath Document Pointer");
    return false;
  }
  xmlNodePtr nodep = nullptr;
  if (!context.isNull()) {
    DOMNode *domnode = getDOMNode(context);
    nodep = domnode->nodep();
  }
  if (!nodep) {
    nodep = xmlDocGetRootElement(docp);
  }
  if (nodep && docp != nodep->doc) {
    raise_warning("Node From Wrong Document");
    return false;
  }
  ctxp->node = nodep;
  /* Register namespaces in the node */
  if (registerNodeNS) {
    ns = xmlGetNsList(docp, nodep);
    if (ns != nullptr) {
      while (ns[nsnbr] != nullptr) nsnbr++;
    }
  }
  ctxp->namespaces = ns;
  ctxp->nsNr = nsnbr;
  checkVMRegStateGuarded();
  xpathobjp = xmlXPathEvalExpression((xmlChar*)expr.data(), ctxp);
  ctxp->node = nullptr;
  if (ns != nullptr) {
    xmlFree(ns);
    ctxp->namespaces = nullptr;
    ctxp->nsNr = 0;
  }
  if (!xpathobjp) {
    return false;
  }
  if (type == PHP_DOM_XPATH_QUERY) {
    xpath_type = XPATH_NODESET;
  } else {
    xpath_type = xpathobjp->type;
  }
  Variant ret;
  switch (xpath_type) {
  case XPATH_NODESET:
  {
    int i;
    xmlNodeSetPtr nodesetp;
    Array retval = Array::CreateVec();
    if (xpathobjp->type == XPATH_NODESET &&
        nullptr != (nodesetp = xpathobjp->nodesetval)) {
      for (i = 0; i < nodesetp->nodeNr; i++) {
        xmlNodePtr node = nodesetp->nodeTab[i];
        if (node->type == XML_NAMESPACE_DECL) {
          xmlNsPtr curns;
          //xmlNodePtr nsparent;
          //nsparent = node->_private;
          curns = xmlNewNs(nullptr, node->name, nullptr);
          if (node->children) {
            curns->prefix = xmlStrdup((xmlChar*)node->children);
          }
          if (node->children) {
            node = xmlNewDocNode(docp, nullptr, (xmlChar*)node->children,
                                 node->name);
          } else {
            node = xmlNewDocNode(docp, nullptr, (xmlChar*)"xmlns", node->name);
          }
          node->type = XML_NAMESPACE_DECL;
          //node->parent = nsparent;
          node->ns = curns;
        }
        // Ugh, the statement below creates a new object and
        // adds it to an array...
        retval.append(create_node_object(node, domxpath->m_doc));
      }
    }

    auto docData = Native::data<DOMNode>(domxpath->m_doc)->doc();
    Object node_list{DOMNodeList::classof()};
    auto list_data = Native::data<DOMIterable>(node_list);
    list_data->m_doc = docData;
    list_data->m_baseobjptr = retval;
    list_data->m_nodetype = DOM_NODESET;
    ret = std::move(node_list);
    break;
  }
  case XPATH_BOOLEAN:
    ret = (bool)(xpathobjp->boolval);
    break;
  case XPATH_NUMBER:
    ret = (double)(xpathobjp->floatval);
    break;
  case XPATH_STRING:
    ret = String((char*)xpathobjp->stringval, CopyString);
    break;
  default:
    ret = uninit_null();
    break;
  }
  xmlXPathFreeObject(xpathobjp);
  return ret;
}

static void php_set_attribute_id(xmlAttrPtr attrp, bool is_id) {
  if (is_id == 1 && attrp->atype != XML_ATTRIBUTE_ID) {
    xmlChar *id_val = xmlNodeListGetString(attrp->doc, attrp->children, 1);
    if (id_val != nullptr) {
      xmlAddID(nullptr, attrp->doc, id_val, attrp);
      xmlFree(id_val);
    }
  } else {
    if (attrp->atype == XML_ATTRIBUTE_ID) {
      xmlRemoveID(attrp->doc, attrp);
      attrp->atype = (xmlAttributeType)0;
    }
  }
}

static xmlNsPtr _dom_new_reconNs(xmlDocPtr doc, xmlNodePtr tree, xmlNsPtr ns) {
  xmlNsPtr def;
  xmlChar prefix[50];
  int counter = 1;
  if (tree == nullptr || ns == nullptr || ns->type != XML_NAMESPACE_DECL) {
    return nullptr;
  }
  /* Code taken from libxml2 (2.6.20) xmlNewReconciliedNs
   *
   * Find a close prefix which is not already in use.
   * Let's strip namespace prefixes longer than 20 chars!
   */
  if (ns->prefix == nullptr) {
    snprintf((char*)prefix, sizeof(prefix), "default");
  } else {
    snprintf((char*)prefix, sizeof(prefix), "%.20s", (char *)ns->prefix);
  }
  def = xmlSearchNs(doc, tree, prefix);
  while (def != nullptr) {
    if (counter > 1000) return(nullptr);
    if (ns->prefix == nullptr)
      snprintf((char*)prefix, sizeof(prefix), "default%d", counter++);
    else
      snprintf((char*)prefix, sizeof(prefix), "%.20s%d",
      (char*)ns->prefix, counter++);
    def = xmlSearchNs(doc, tree, prefix);
  }
  /*
   * OK, now we are ready to create a new one.
   */
  def = xmlNewNs(tree, ns->href, prefix);
  return(def);
}

static const char * getStringData(const String& str) {
  if (str.isNull()) {
    return nullptr;
  }
  return str.data();
}

static xmlNodePtr create_notation(const xmlChar *name,
                                  const xmlChar *ExternalID,
                                  const xmlChar *SystemID) {
  xmlEntityPtr ret;
  ret = (xmlEntityPtr) xmlMalloc(sizeof(xmlEntity));
  memset(ret, 0, sizeof(xmlEntity));
  ret->type = XML_NOTATION_NODE;
  ret->name = xmlStrdup(name);
  ret->ExternalID = xmlStrdup(ExternalID);
  ret->SystemID = xmlStrdup(SystemID);
  ret->length = 0;
  ret->content = nullptr;
  ret->URI = nullptr;
  ret->orig = nullptr;
  ret->children = nullptr;
  ret->parent = nullptr;
  ret->doc = nullptr;
  ret->_private = nullptr;
  ret->last = nullptr;
  ret->prev = nullptr;
  return((xmlNodePtr) ret);
}

struct nodeIterator {
  int cur;
  int index;
  xmlNode *node;
};

struct notationIterator {
  int cur;
  int index;
  xmlNotation *notation;
};

#if LIBXML_VERSION >= 20908
#define XMLCHAR_CONST const
#else
#define XMLCHAR_CONST
#endif
static void itemHashScanner(void* payload, void* data, XMLCHAR_CONST xmlChar* /*name*/) {
#undef XMLCHAR_CONST
  nodeIterator *priv = (nodeIterator *)data;
  if (priv->cur < priv->index) {
    priv->cur++;
  } else if (priv->node == nullptr) {
    priv->node = (xmlNode *)payload;
  }
}

static xmlNode *php_dom_libxml_hash_iter(xmlHashTable *ht, int index) {
  int htsize = xmlHashSize(ht);
  if (htsize > 0 && index < htsize) {
    nodeIterator iter;
    iter.cur = 0;
    iter.index = index;
    iter.node = nullptr;
    xmlHashScan(ht, itemHashScanner, &iter);
    return iter.node;
  }
  return nullptr;
}

static xmlNode *php_dom_libxml_notation_iter(xmlHashTable *ht, int index) {
  int htsize = xmlHashSize(ht);
  if (htsize > 0 && index < htsize) {
    notationIterator iter;
    iter.cur = 0;
    iter.index = index;
    iter.notation = nullptr;
    xmlHashScan(ht, itemHashScanner, &iter);
    xmlNotation *notep = iter.notation;
    return create_notation(notep->name, notep->PublicID, notep->SystemID);
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

Variant dummy_getter(const Object&) {
  raise_notice("Cannot read property");
  return init_null();
}

void dummy_setter(const Object&, const Variant&) {
  raise_error("Cannot write property");
}

struct DOMPropertyAccessor {
  const char * name;
  Variant (*getter)(const Object&);
  void (*setter)(const Object&, const Variant&);
};

const StaticString s_object_value_omitted("(object value omitted)");

struct hashdpa {
  size_t operator()(const DOMPropertyAccessor* da) const {
    return hash_string_i(da->name, strlen(da->name));
  }
};
struct cmpdpa {
  bool operator()(const DOMPropertyAccessor* da1,
                  const DOMPropertyAccessor* da2) const {
    return strcasecmp(da1->name, da2->name) == 0;
  }
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Base native property handler class, that is reused
 * by derived DOM classes with the similar handling.
 */
template <class Derived>
struct DOMPropHandler: Native::BasePropHandler {
  static Variant getProp(const Object& this_, const String& name) {
    return Derived::map.getter(name)(this_);
  }

  static Variant setProp(const Object& this_,
                         const String& name,
                         const Variant& value) {
    Derived::map.setter(name)(this_, value);
    return true;
  }

  static Variant issetProp(const Object& this_, const String& name) {
    return Derived::map.isset(this_, name);
  }

  static bool isPropSupported(const String& name, const String& /*op*/) {
    return Derived::map.isPropertySupported(name);
  }
};

///////////////////////////////////////////////////////////////////////////////

struct DOMPropertyAccessorMap :
      private hphp_hash_set<DOMPropertyAccessor*, hashdpa, cmpdpa> {
  explicit DOMPropertyAccessorMap(DOMPropertyAccessor* props,
                                  DOMPropertyAccessorMap *base = nullptr) {
    if (base) {
      *this = *base;
    }
    for (DOMPropertyAccessor *p = props; p->name; p++) {
      this->insert(p);
      m_props.push_back(p);
    }
  }

  bool isPropertySupported(const String& name) {
    auto dpa = DOMPropertyAccessor {
      name.data(), nullptr, nullptr
    };
    const_iterator iter = find(&dpa);
    return iter != end();
  }

  Variant (*getter(const String& name))(const Object&) {
    auto dpa = DOMPropertyAccessor {
      name.data(), nullptr, nullptr
    };
    const_iterator iter = find(&dpa);
    if (iter != end() && (*iter)->getter) {
      if (strcmp(dpa.name, (*iter)->name)) {
        raise_warning("Accessing DOMNode derived property '%s' with the "
                      "incorrect casing", dpa.name);
      }
      return (*iter)->getter;
    }
    return dummy_getter;
  }

  void (*setter(const String& name))(const Object&, const Variant&) {
    auto dpa = DOMPropertyAccessor {
      name.data(), nullptr, nullptr
    };
    const_iterator iter = find(&dpa);
    if (iter != end() && (*iter)->setter) {
      if (strcmp(dpa.name, (*iter)->name)) {
        raise_warning("Setting DOMNode derived property '%s' with the "
                      "incorrect casing", dpa.name);
      }
      return (*iter)->setter;
    }
    return dummy_setter;
  }

  bool isset(const Object& obj, const String& name) {
    auto dpa = DOMPropertyAccessor {
      name.data(), nullptr, nullptr
    };
    const_iterator iter = find(&dpa);
    if (iter != end() && (*iter)->getter) {
      if (strcmp(dpa.name, (*iter)->name)) {
        raise_warning("Accessing DOMNode derived property '%s' with the "
                      "incorrect casing", dpa.name);
      }
      return !(*iter)->getter(obj).isNull();
    }
    return false;
  }

  Array debugInfo(const Object& obj) {
    auto ret = obj->toArray();
    for (auto it : m_props) {
      auto value = it->getter(obj);
      if (value.isObject()) {
        value = s_object_value_omitted;
      }
      ret.set(String(it->name, CopyString), value);
    }
    return ret;
  }

private:
  std::vector<DOMPropertyAccessor*> m_props;
};

///////////////////////////////////////////////////////////////////////////////

void HHVM_METHOD(DOMDocument, __construct,
             const Variant& version /* = null_string */,
             const Variant& encoding /* = null_string */);

Object newDOMDocument(bool construct /* = true */) {
  Object doc{DOMDocument::classof()};
  if (LIKELY(construct)) {
    HHVM_MN(DOMDocument, __construct)(doc.get(), null_string, null_string);
  }
  return doc;
}

static Object newDOMNamedNodeMap(req::ptr<XMLDocumentData> doc, Object base,
                          int node_type, xmlHashTable* ht = nullptr) {
  Object nodemap{DOMNamedNodeMap::classof()};
  auto data = Native::data<DOMIterable>(nodemap);
  data->m_doc = doc;
  data->m_baseobj = base;
  data->m_nodetype = node_type;
  data->m_ht = ht;

  return nodemap;
}

static Object newDOMNodeList(req::ptr<XMLDocumentData> doc, Object base,
                             int node_type, String local = String(),
                             String ns = String()) {
  Object ret{DOMNodeList::classof()};
  auto data = Native::data<DOMIterable>(ret);
  data->m_doc = doc;
  data->m_baseobj = base;
  data->m_nodetype = node_type;
  data->m_local = local;
  data->m_ns = ns;

  return ret;
}

#define CHECK_NODE(nodepVar)                                                   \
  DOMNode *domnode = getDOMNode(obj);                                          \
  xmlNodePtr nodepVar = domnode->node() ? domnode->nodep() : nullptr;          \
  if (nodepVar == nullptr) {                                                   \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                                 \
    return init_null();                                                        \
  }                                                                            \
/**/

#define CHECK_WRITE_NODE(nodepVar)                                             \
  DOMNode *domnode = getDOMNode(obj);                                          \
  xmlNodePtr nodepVar = domnode->node() ? domnode->nodep() : nullptr;          \
  if (nodepVar == nullptr) {                                                   \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                                 \
    return;                                                                    \
  }                                                                            \
/**/

static Variant domnode_nodename_read(const Object& obj) {
  CHECK_NODE(nodep);
  xmlNsPtr ns;
  char *str = nullptr;
  xmlChar *qname = nullptr;
  switch (nodep->type) {
  case XML_ATTRIBUTE_NODE:
  case XML_ELEMENT_NODE:
    ns = nodep->ns;
    if (ns != nullptr && ns->prefix) {
      qname = xmlStrdup(ns->prefix);
      qname = xmlStrcat(qname, (const xmlChar*)":");
      qname = xmlStrcat(qname, nodep->name);
      str = (char*)qname;
    } else {
      str = (char*)nodep->name;
    }
    break;
  case XML_NAMESPACE_DECL:
    ns = nodep->ns;
    if (ns != nullptr && ns->prefix) {
      qname = xmlStrdup((const xmlChar*)"xmlns");
      qname = xmlStrcat(qname, (const xmlChar*)":");
      qname = xmlStrcat(qname, nodep->name);
      str = (char*)qname;
    } else {
      str = (char*)nodep->name;
    }
    break;
  case XML_DOCUMENT_TYPE_NODE:
  case XML_DTD_NODE:
  case XML_PI_NODE:
  case XML_ENTITY_DECL:
  case XML_ENTITY_REF_NODE:
  case XML_NOTATION_NODE:
    str = (char*)nodep->name;
    break;
  case XML_CDATA_SECTION_NODE:    return "#cdata-section";
  case XML_COMMENT_NODE:          return "#comment";
  case XML_HTML_DOCUMENT_NODE:
  case XML_DOCUMENT_NODE:         return "#document";
  case XML_DOCUMENT_FRAG_NODE:    return "#document-fragment";
  case XML_TEXT_NODE:             return "#text";
  default:
    raise_warning("Invalid Node Type");
    break;
  }
  String retval(str, CopyString);
  if (qname != nullptr) {
    xmlFree(qname);
  }
  return retval;
}

static Variant domnode_nodevalue_read(const Object& obj) {
  CHECK_NODE(nodep);
  char *str = nullptr;
  /* Access to Element node is implemented as a convience method */
  switch (nodep->type) {
  case XML_ATTRIBUTE_NODE:
  case XML_TEXT_NODE:
  case XML_ELEMENT_NODE:
  case XML_COMMENT_NODE:
  case XML_CDATA_SECTION_NODE:
  case XML_PI_NODE:
    str = (char*)xmlNodeGetContent(nodep);
    break;
  case XML_NAMESPACE_DECL:
    str = (char*)xmlNodeGetContent(nodep->children);
    break;
  default:
    str = nullptr;
    break;
  }
  if (str != nullptr) {
    String retval(str, CopyString);
    xmlFree(str);
    return retval;
  } else {
    return init_null();
  }
}

static void domnode_nodevalue_write(const Object& obj, const Variant& value) {
  CHECK_WRITE_NODE(nodep);
  /* Access to Element node is implemented as a convience method */
  switch (nodep->type) {
  case XML_ELEMENT_NODE:
  case XML_ATTRIBUTE_NODE:
    if (nodep->children) {
      php_libxml_node_free_resource((xmlNodePtr) nodep->children);
      nodep->children = nullptr;
    }
  case XML_TEXT_NODE:
  case XML_COMMENT_NODE:
  case XML_CDATA_SECTION_NODE:
  case XML_PI_NODE:
    {
      String valueStr = value.toString();
      xmlNodeSetContentLen(nodep,
                           (const xmlChar*)valueStr.data(),
                           valueStr.size() + 1);
    }
    break;
  default:
    break;
  }
}

static Variant domnode_nodetype_read(const Object& obj) {
  CHECK_NODE(nodep);
  Variant retval;
  /* Specs dictate that they are both type XML_DOCUMENT_TYPE_NODE */
  if (nodep->type == XML_DTD_NODE) {
    retval = XML_DOCUMENT_TYPE_NODE;
  } else {
    retval = nodep->type;
  }
  return retval;
}

static Variant domnode_parentnode_read(const Object& obj) {
  CHECK_NODE(nodep);
  return create_node_object(nodep->parent, domnode->doc());
}

static Variant domnode_childnodes_read(const Object& obj) {
  CHECK_NODE(nodep);
  if (!dom_node_children_valid(nodep)) {
    return init_null();
  }
  return newDOMNodeList(domnode->doc(), obj, XML_ELEMENT_NODE);
}

static Variant domnode_firstchild_read(const Object& obj) {
  CHECK_NODE(nodep);
  xmlNode *first = nullptr;
  if (dom_node_children_valid(nodep)) {
    first = nodep->children;
  }
  return create_node_object(first, domnode->doc());
}

static Variant domnode_lastchild_read(const Object& obj) {
  CHECK_NODE(nodep);
  xmlNode *last = nullptr;
  if (dom_node_children_valid(nodep)) {
    last = nodep->last;
  }
  return create_node_object(last, domnode->doc());
}

static Variant domnode_previoussibling_read(const Object& obj) {
  CHECK_NODE(nodep);
  return create_node_object(nodep->prev, domnode->doc());
}

static Variant domnode_nextsibling_read(const Object& obj) {
  CHECK_NODE(nodep);
  return create_node_object(nodep->next, domnode->doc());
}

static Variant domnode_attributes_read(const Object& obj) {
  CHECK_NODE(nodep);
  if (nodep->type == XML_ELEMENT_NODE) {
    return newDOMNamedNodeMap(domnode->doc(), obj, XML_ATTRIBUTE_NODE);
  }
  return init_null();
}

static Variant domnode_ownerdocument_read(const Object& obj) {
  CHECK_NODE(nodep);
  if (nodep->type == XML_DOCUMENT_NODE ||
      nodep->type == XML_HTML_DOCUMENT_NODE) {
    return init_null();
  }
  auto doc = domnode->doc();
  if (doc && (nodep->doc == doc->docp())) {
    return create_node_object((xmlNodePtr) doc->docp(), doc);
  } else {
    // The node wasn't created by this extension, so doesn't already have
    // a DOMDocument - make one. dom_import_xml() is one way for this to
    // happen.
    return create_node_object((xmlNodePtr) nodep->doc, doc);
  }
}

static Variant domnode_namespaceuri_read(const Object& obj) {
  CHECK_NODE(nodep);
  const char *str = nullptr;
  switch (nodep->type) {
  case XML_ELEMENT_NODE:
  case XML_ATTRIBUTE_NODE:
  case XML_NAMESPACE_DECL:
    if (nodep->ns != nullptr) {
      str = (const char*)nodep->ns->href;
    }
    break;
  default:
    break;
  }
  if (str) {
    return String(str, CopyString);
  }
  return init_null();
}

static Variant domnode_prefix_read(const Object& obj) {
  CHECK_NODE(nodep);
  xmlNsPtr ns;
  char *str = nullptr;
  switch (nodep->type) {
  case XML_ELEMENT_NODE:
  case XML_ATTRIBUTE_NODE:
  case XML_NAMESPACE_DECL:
    ns = nodep->ns;
    if (ns != nullptr && ns->prefix) {
      str = (char *)ns->prefix;
    }
    break;
  default:
    break;
  }

  if (str) {
    return String(str, CopyString);
  }
  return empty_string_variant();
}

static void domnode_prefix_write(const Object& obj, const Variant& value) {
  String svalue;
  xmlNode *nsnode = nullptr;
  xmlNsPtr ns = nullptr, curns;
  const char *strURI;
  const char *prefix;

  CHECK_WRITE_NODE(nodep);
  switch (nodep->type) {
  case XML_ELEMENT_NODE:
    nsnode = nodep;
    [[fallthrough]];
  case XML_ATTRIBUTE_NODE:
    if (nsnode == nullptr) {
      nsnode = nodep->parent;
      if (nsnode == nullptr) {
        nsnode = xmlDocGetRootElement(nodep->doc);
      }
    }
    svalue = value.toString();
    prefix = svalue.data();
    if (nsnode && nodep->ns != nullptr &&
        !xmlStrEqual(nodep->ns->prefix, (xmlChar *)prefix)) {
      strURI = (char *) nodep->ns->href;
      if (strURI == nullptr ||
          (!strcmp (prefix, "xml") &&
           strcmp(strURI, (const char *)XML_XML_NAMESPACE)) ||
          (nodep->type == XML_ATTRIBUTE_NODE && !strcmp (prefix, "xmlns") &&
           strcmp (strURI, (const char *)DOM_XMLNS_NAMESPACE)) ||
          (nodep->type == XML_ATTRIBUTE_NODE &&
           !strcmp ((const char*)nodep->name, "xmlns"))) {
        ns = nullptr;
      } else {
        curns = nsnode->nsDef;
        while (curns != nullptr) {
          if (xmlStrEqual((xmlChar *)prefix, curns->prefix) &&
              xmlStrEqual(nodep->ns->href, curns->href)) {
            ns = curns;
            break;
          }
          curns = curns->next;
        }
        if (ns == nullptr) {
          ns = xmlNewNs(nsnode, nodep->ns->href, (xmlChar *)prefix);
        }
      }

      if (ns == nullptr) {
        php_dom_throw_error(NAMESPACE_ERR, domnode->doc()->m_stricterror);
        return;
      }

      xmlSetNs(nodep, ns);
    }
    break;
  default:
    break;
  }
}

static Variant domnode_localname_read(const Object& obj) {
  CHECK_NODE(nodep);
  if (nodep->type == XML_ELEMENT_NODE ||
      nodep->type == XML_ATTRIBUTE_NODE ||
      nodep->type == XML_NAMESPACE_DECL) {
    return String((char *)(nodep->name), CopyString);
  }
  return init_null();
}

static Variant domnode_baseuri_read(const Object& obj) {
  CHECK_NODE(nodep);
  xmlChar *baseuri = xmlNodeGetBase(nodep->doc, nodep);
  if (baseuri) {
    String ret((char *)(baseuri), CopyString);
    xmlFree(baseuri);
    return ret;
  }
  return init_null();
}

static Variant domnode_textcontent_read(const Object& obj) {
  CHECK_NODE(nodep);
  char *str = (char *)xmlNodeGetContent(nodep);
  if (str) {
    String ret(str, CopyString);
    xmlFree(str);
    return ret;
  }
  return empty_string_variant();
}

static void domnode_textcontent_write(const Object& obj, const Variant& value) {
  CHECK_WRITE_NODE(nodep);

  if (nodep->type == XML_ELEMENT_NODE || nodep->type == XML_ATTRIBUTE_NODE) {
    if (nodep->children) {
      php_libxml_node_free_resource((xmlNodePtr) nodep->children);
      nodep->children = nullptr;
    }
  }

  xmlNodeSetContent(nodep, (xmlChar *) "");
  xmlNodeAddContent(nodep, (xmlChar *) value.toString().data());
}

static DOMPropertyAccessor domnode_properties[] = {
  { "nodeName",        domnode_nodename_read,      nullptr },
  { "nodeValue",       domnode_nodevalue_read,     domnode_nodevalue_write },
  { "nodeType",        domnode_nodetype_read,      nullptr },
  { "parentNode",      domnode_parentnode_read,    nullptr },
  { "childNodes",      domnode_childnodes_read,    nullptr },
  { "firstChild",      domnode_firstchild_read,    nullptr },
  { "lastChild",       domnode_lastchild_read,     nullptr },
  { "previousSibling", domnode_previoussibling_read, nullptr },
  { "nextSibling",     domnode_nextsibling_read,   nullptr },
  { "attributes",      domnode_attributes_read,    nullptr },
  { "ownerDocument",   domnode_ownerdocument_read, nullptr },
  { "namespaceURI",    domnode_namespaceuri_read,  nullptr },
  { "prefix",          domnode_prefix_read,        domnode_prefix_write },
  { "localName",       domnode_localname_read,     nullptr },
  { "baseURI",         domnode_baseuri_read,       nullptr },
  { "textContent",     domnode_textcontent_read,   domnode_textcontent_write },
  { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domnode_properties_map
((DOMPropertyAccessor*)domnode_properties);

struct DOMNodePropHandler : public DOMPropHandler<DOMNodePropHandler> {
  static constexpr DOMPropertyAccessorMap& map = domnode_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

Array HHVM_METHOD(DOMNode, __debugInfo) {
  auto* data = Native::data<DOMNode>(this_);
  if (!data->node()) {
    return this_->toArray();
  }
  return domnode_properties_map.debugInfo(Object{this_});
}

Variant HHVM_METHOD(DOMNode, appendChild,
                    const Object& newnode) {
  auto* data = Native::data<DOMNode>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return init_null();
  }

  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  auto* newdomnode = getDOMNode(newnode);
  xmlNodePtr child = newdomnode->nodep();
  if (!child) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return init_null();
  }
  xmlNodePtr new_child = nullptr;
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;

  if (dom_node_is_read_only(nodep) ||
      (child->parent != nullptr && dom_node_is_read_only(child->parent))) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  if (!dom_hierarchy(nodep, child)) {
    php_dom_throw_error(HIERARCHY_REQUEST_ERR, data->doc()->m_stricterror);
    return false;
  }
  if (!(child->doc == nullptr || child->doc == nodep->doc)) {
    php_dom_throw_error(WRONG_DOCUMENT_ERR, data->doc()->m_stricterror);
    return false;
  }
  if (child->type == XML_DOCUMENT_FRAG_NODE && child->children == nullptr) {
    raise_warning("Document Fragment is empty");
    return false;
  }
  if (child->parent != nullptr) {
    xmlUnlinkNode(child);
  }
  if (child->type == XML_TEXT_NODE && nodep->last != nullptr &&
      nodep->last->type == XML_TEXT_NODE) {
    child->parent = nodep;
    if (child->doc == nullptr) {
      xmlSetTreeDoc(child, nodep->doc);
    }
    new_child = child;
    if (nodep->children == nullptr) {
      nodep->children = child;
      nodep->last = child;
    } else {
      child = nodep->last;
      child->next = new_child;
      new_child->prev = child;
      nodep->last = new_child;
    }
  } else if (child->type == XML_ATTRIBUTE_NODE) {
    xmlAttrPtr lastattr;
    if (child->ns == nullptr) {
      lastattr = xmlHasProp(nodep, child->name);
    } else {
      lastattr = xmlHasNsProp(nodep, child->name, child->ns->href);
    }
    if (lastattr != nullptr && lastattr->type != XML_ATTRIBUTE_DECL) {
      if (lastattr != (xmlAttrPtr)child) {
        libxml_register_node((xmlNodePtr)lastattr)->unlink();
      }
    }
  } else if (child->type == XML_DOCUMENT_FRAG_NODE) {
    new_child = _php_dom_insert_fragment(nodep, nodep->last, nullptr, child);
  }
  if (new_child == nullptr) {
    new_child = xmlAddChild(nodep, child);
    if (new_child == nullptr) {
      raise_warning("Couldn't append node");
      return false;
    }
  }
  dom_reconcile_ns(nodep->doc, new_child);
  return create_node_object(new_child, data->doc());
}

DOMNode& DOMNode::operator=(const DOMNode& copy) {
  if (auto copyNode = copy.nodep()) {
    auto newNode = xmlDocCopyNode(copyNode, copyNode->doc, true /* deep */);
    setNode(newNode);
    if (auto d = copy.doc()) {
      setDoc(std::move(d));
    }
    return *this;
  }

  if (m_node) {
    assertx(m_node->getCache() &&
           Native::data<DOMNode>(m_node->getCache()) == this);
    m_node->clearCache();
    m_node = nullptr;
  }
  return *this;
}

Variant HHVM_METHOD(DOMNode, cloneNode,
                    bool deep /* = false */) {
  auto* data = Native::data<DOMNode>(this_);
  xmlNodePtr n = data->nodep();
  if (!n) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return init_null();
  }
  xmlNode* node = xmlDocCopyNode(n, n->doc, deep);
  if (!node) {
    return false;
  }
  // When deep is false Element nodes still require the attributes
  // Following taken from libxml as xmlDocCopyNode doesnt do this
  if (n->type == XML_ELEMENT_NODE && !deep) {
    if (n->nsDef != nullptr) {
      node->nsDef = xmlCopyNamespaceList(n->nsDef);
    }
    if (n->ns != nullptr) {
      xmlNsPtr ns;
      ns = xmlSearchNs(n->doc, node, n->ns->prefix);
      if (ns == nullptr) {
        ns = xmlSearchNs(n->doc, n, n->ns->prefix);
        if (ns != nullptr) {
          xmlNodePtr root = node;
          while (root->parent != nullptr) {
            root = root->parent;
          }
          node->ns = xmlNewNs(root, ns->href, ns->prefix);
        }
      } else {
        node->ns = ns;
      }
    }
    if (n->properties != nullptr) {
      node->properties = xmlCopyPropList(node, n->properties);
    }
  }
  return create_node_object(node, data->doc());
}

int64_t HHVM_METHOD(DOMNode, getLineNo) {
  auto* data = Native::data<DOMNode>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return 0;
  }
  return xmlGetLineNo(nodep);
}

bool HHVM_METHOD(DOMNode, hasAttributes) {
  auto* data = Native::data<DOMNode>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  if (nodep->type != XML_ELEMENT_NODE) {
    return false;
  }
  return nodep->properties;
}

bool HHVM_METHOD(DOMNode, hasChildNodes) {
  auto* data = Native::data<DOMNode>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  return nodep->children;
}

Variant HHVM_METHOD(DOMNode, insertBefore,
                    const Object& newnode,
                    const Variant& refnode /* = null */) {
  auto* data = Native::data<DOMNode>(this_);
  xmlNodePtr parentp = data->nodep();
  if (!parentp) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  if (!dom_node_children_valid(parentp)) {
    return false;
  }
  auto* domchildnode = getDOMNode(newnode);
  xmlNodePtr child = domchildnode->nodep();
  if (!child) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlNodePtr new_child = nullptr;
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;

  if (dom_node_is_read_only(parentp) ||
    (child->parent != nullptr && dom_node_is_read_only(child->parent))) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  if (!dom_hierarchy(parentp, child)) {
    php_dom_throw_error(HIERARCHY_REQUEST_ERR, stricterror);
    return false;
  }
  if (child->doc != parentp->doc && child->doc != nullptr) {
    php_dom_throw_error(WRONG_DOCUMENT_ERR, stricterror);
    return false;
  }
  if (child->type == XML_DOCUMENT_FRAG_NODE && child->children == nullptr) {
    raise_warning("Document Fragment is empty");
    return false;
  }
  if (!refnode.isNull()) {
    auto* domrefnode = getDOMNode(refnode.toObject());
    xmlNodePtr refp = domrefnode->nodep();
    if (!refp || refp->parent != parentp) {
      php_dom_throw_error(NOT_FOUND_ERR, stricterror);
      return false;
    }
    if (child->parent != nullptr) {
      xmlUnlinkNode(child);
    }
    if (child->type == XML_TEXT_NODE && (refp->type == XML_TEXT_NODE ||
      (refp->prev != nullptr && refp->prev->type == XML_TEXT_NODE))) {
      if (child->doc == nullptr) {
        xmlSetTreeDoc(child, parentp->doc);
      }
      new_child = child;
      new_child->parent = refp->parent;
      new_child->next = refp;
      new_child->prev = refp->prev;
      refp->prev = new_child;
      if (new_child->prev != nullptr) {
        new_child->prev->next = new_child;
      }
      if (new_child->parent != nullptr) {
        if (new_child->parent->children == refp) {
          new_child->parent->children = new_child;
        }
      }
    } else if (child->type == XML_ATTRIBUTE_NODE) {
      xmlAttrPtr lastattr;
      if (child->ns == nullptr) {
        lastattr = xmlHasProp(refp->parent, child->name);
      } else {
        lastattr = xmlHasNsProp(refp->parent, child->name, child->ns->href);
      }
      if (lastattr != nullptr && lastattr->type != XML_ATTRIBUTE_DECL) {
        if (lastattr != (xmlAttrPtr)child) {
          libxml_register_node((xmlNodePtr)lastattr)->unlink();
        } else {
          return create_node_object(child, data->doc());
        }
      }
    } else if (child->type == XML_DOCUMENT_FRAG_NODE) {
      new_child = _php_dom_insert_fragment(parentp, refp->prev, refp, child);
    }
    if (new_child == nullptr) {
      new_child = xmlAddPrevSibling(refp, child);
    }
  } else {
    if (child->parent != nullptr) {
      xmlUnlinkNode(child);
    }
    if (child->type == XML_TEXT_NODE && parentp->last != nullptr &&
        parentp->last->type == XML_TEXT_NODE) {
      child->parent = parentp;
      if (child->doc == nullptr) {
        xmlSetTreeDoc(child, parentp->doc);
      }
      new_child = child;
      if (parentp->children == nullptr) {
        parentp->children = child;
        parentp->last = child;
      } else {
        child = parentp->last;
        child->next = new_child;
        new_child->prev = child;
        parentp->last = new_child;
      }
    } else if (child->type == XML_ATTRIBUTE_NODE) {
      xmlAttrPtr lastattr;
      if (child->ns == nullptr)
        lastattr = xmlHasProp(parentp, child->name);
      else
        lastattr = xmlHasNsProp(parentp, child->name, child->ns->href);
      if (lastattr != nullptr && lastattr->type != XML_ATTRIBUTE_DECL) {
        if (lastattr != (xmlAttrPtr)child) {
          libxml_register_node((xmlNodePtr)lastattr)->unlink();
        } else {
          return create_node_object(child, data->doc());
        }
      }
    } else if (child->type == XML_DOCUMENT_FRAG_NODE) {
      new_child = _php_dom_insert_fragment(parentp, parentp->last, nullptr,
                                           child);
    }
    if (new_child == nullptr) {
      new_child = xmlAddChild(parentp, child);
    }
  }
  if (nullptr == new_child) {
    raise_warning("Couldn't add newnode as the previous sibling of refnode");
    return false;
  }
  dom_reconcile_ns(parentp->doc, new_child);
  return create_node_object(new_child, data->doc());
}

bool HHVM_METHOD(DOMNode, isDefaultNamespace,
                 const String& namespaceuri) {
  auto* data = Native::data<DOMNode>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlNsPtr nsptr;
  if (nodep->type == XML_DOCUMENT_NODE ||
      nodep->type == XML_HTML_DOCUMENT_NODE) {
    nodep = xmlDocGetRootElement((xmlDocPtr)nodep);
  }
  if (nodep && namespaceuri.size() > 0) {
    nsptr = xmlSearchNs(nodep->doc, nodep, nullptr);
    if (nsptr && xmlStrEqual(nsptr->href, (xmlChar*)namespaceuri.data())) {
      return true;
    }
  }
  return false;
}

bool HHVM_METHOD(DOMNode, isSameNode,
                 const Object& node) {
  auto* data = Native::data<DOMNode>(this_);
  auto* other_data = getDOMNode(node);
  return data->nodep() == other_data->nodep();
}

bool HHVM_METHOD(DOMNode, isSupported,
                 const String& feature,
                 const String& version) {
  return dom_has_feature(feature.data(), version.data());
}

Variant HHVM_METHOD(DOMNode, lookupNamespaceUri,
                    const Variant& namespaceuri) {
  if (!namespaceuri.isString() && !namespaceuri.isNull()) {
    raise_param_type_warning("DOMNode::lookupNamespaceUri", 1,
                             "string", *namespaceuri.asTypedValue());
    return init_null();
  }

  auto* data = Native::data<DOMNode>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return init_null();
  }
  xmlNsPtr nsptr;
  if (nodep->type == XML_DOCUMENT_NODE ||
      nodep->type == XML_HTML_DOCUMENT_NODE) {
    nodep = xmlDocGetRootElement((xmlDocPtr) nodep);
    if (nodep == nullptr) {
      return init_null();
    }
  }

  String nsuri = namespaceuri.toString();
  const char* ns = nsuri.data();
  if (namespaceuri.isNull()) {
    ns = nullptr;
  }

  nsptr = xmlSearchNs(nodep->doc, nodep, (xmlChar*)ns);
  if (nsptr && nsptr->href != nullptr) {
    return String((char *)nsptr->href, CopyString);
  }
  return init_null();
}

Variant HHVM_METHOD(DOMNode, lookupPrefix,
                    const String& prefix) {
  auto* data = Native::data<DOMNode>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return init_null();
  }
  xmlNodePtr lookupp;
  xmlNsPtr nsptr;
  if (prefix.size() > 0) {
    switch (nodep->type) {
    case XML_ELEMENT_NODE:
      lookupp = nodep;
      break;
    case XML_DOCUMENT_NODE:
    case XML_HTML_DOCUMENT_NODE:
      lookupp = xmlDocGetRootElement((xmlDocPtr)nodep);
      break;
    case XML_ENTITY_NODE:
    case XML_NOTATION_NODE:
    case XML_DOCUMENT_FRAG_NODE:
    case XML_DOCUMENT_TYPE_NODE:
    case XML_DTD_NODE:
      return init_null();
    default:
      lookupp = nodep->parent;
    }
    if (lookupp != nullptr &&
        (nsptr = xmlSearchNsByHref(lookupp->doc, lookupp,
                                   (xmlChar*)prefix.data()))) {
      if (nsptr->prefix != nullptr) {
        return String((char *)nsptr->prefix, CopyString);
      }
    }
  }
  return init_null();
}

void HHVM_METHOD(DOMNode, normalize) {
  auto* data = Native::data<DOMNode>(this_);
  if (!data->nodep()) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return;
  }
  dom_normalize(data->nodep());
}

Variant HHVM_METHOD(DOMNode, removeChild,
                    const Object& node) {
  auto* data = Native::data<DOMNode>(this_);
  xmlNodePtr children;
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  auto* domnode2 = getDOMNode(node);
  xmlNodePtr child = domnode2->nodep();
  if (!child) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(nodep) ||
    (child->parent != nullptr && dom_node_is_read_only(child->parent))) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  children = nodep->children;
  if (!children) {
    php_dom_throw_error(NOT_FOUND_ERR, stricterror);
    return false;
  }
  while (children) {
    if (children == child) {
      xmlUnlinkNode(child);
      return create_node_object(child, data->doc());
    }
    children = children->next;
  }
  php_dom_throw_error(NOT_FOUND_ERR, stricterror);
  return false;
}

Variant HHVM_METHOD(DOMNode, replaceChild,
                    const Object& newchildobj,
                    const Object& oldchildobj) {
  auto* data = Native::data<DOMNode>(this_);
  int foundoldchild = 0;
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  auto* domnewchildnode = getDOMNode(newchildobj);
  xmlNodePtr newchild = domnewchildnode->nodep();
  if (!newchild) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  auto* domoldchildnode = getDOMNode(oldchildobj);
  xmlNodePtr oldchild = domoldchildnode->nodep();
  if (!oldchild) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlNodePtr children = nodep->children;
  if (!children) {
    return false;
  }
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(nodep) ||
    (newchild->parent != nullptr && dom_node_is_read_only(newchild->parent))) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  if (newchild->doc != nodep->doc && newchild->doc != nullptr) {
    php_dom_throw_error(WRONG_DOCUMENT_ERR, stricterror);
    return false;
  }
  if (!dom_hierarchy(nodep, newchild)) {
    php_dom_throw_error(HIERARCHY_REQUEST_ERR, stricterror);
    return false;
  }
  // check for the old child and whether the new child is already a child
  while (children) {
    if (children == oldchild) {
      foundoldchild = 1;
      break;
    }
    children = children->next;
  }
  if (foundoldchild) {
    if (newchild->type == XML_DOCUMENT_FRAG_NODE) {
      xmlNodePtr prevsib, nextsib;
      prevsib = oldchild->prev;
      nextsib = oldchild->next;
      xmlUnlinkNode(oldchild);
      newchild = _php_dom_insert_fragment(nodep, prevsib, nextsib, newchild);
      if (newchild) {
        dom_reconcile_ns(nodep->doc, newchild);
      }
    } else if (oldchild != newchild) {
      if (newchild->doc == nullptr && nodep->doc != nullptr) {
        xmlSetTreeDoc(newchild, nodep->doc);
      }
      xmlReplaceNode(oldchild, newchild);
      dom_reconcile_ns(nodep->doc, newchild);
    }

    return create_node_object(oldchild, data->doc());
  }

  php_dom_throw_error(NOT_FOUND_ERR, data->doc()->m_stricterror);
  return false;
}

Variant HHVM_METHOD(DOMNode, C14N,
                    bool exclusive /* = false */,
                    bool with_comments /* = false */,
                    const Variant& xpath /* = null */,
                    const Variant& ns_prefixes /* = null */) {
  auto* data = Native::data<DOMNode>(this_);
  if (!data->nodep()) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  return dom_canonicalization(data->nodep(), "", exclusive, with_comments,
                              xpath, ns_prefixes, 0);
}

Variant HHVM_METHOD(DOMNode, C14Nfile,
                    const String& uri,
                    bool exclusive /* = false */,
                    bool with_comments /* = false */,
                    const Variant& xpath /* = null */,
                    const Variant& ns_prefixes /* = null */) {
  auto* data = Native::data<DOMNode>(this_);
  if (!data->nodep()) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  return dom_canonicalization(data->nodep(), uri, exclusive,
                              with_comments, xpath, ns_prefixes, 1);
}

Variant HHVM_METHOD(DOMNode, getNodePath) {
  auto* data = Native::data<DOMNode>(this_);
  xmlNodePtr n = data->nodep();
  if (!n) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return init_null();
  }
  char *value = (char*)xmlGetNodePath(n);
  if (value) {
    String ret(value, CopyString);
    xmlFree(value);
    return ret;
  }
  return init_null();
}

///////////////////////////////////////////////////////////////////////////////


#define CHECK_ATTR(attrp)                                                      \
  auto domattr = getDOMNode(obj);                                              \
  xmlAttrPtr attrp = (xmlAttrPtr)(domattr ? domattr->nodep() : nullptr);       \
  if (attrp == nullptr) {                                                      \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                                 \
    return init_null();                                                        \
  }                                                                            \
/**/

#define CHECK_WRITE_ATTR(attrp)                                                \
  auto domattr = getDOMNode(obj);                                              \
  xmlAttrPtr attrp = (xmlAttrPtr)(domattr ? domattr->nodep() : nullptr);       \
  if (attrp == nullptr) {                                                      \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                                 \
    return;                                                                    \
  }                                                                            \
/**/

static Variant domattr_name_read(const Object& obj) {
  CHECK_ATTR(attrp);
  return String((char *)(attrp->name), CopyString);
}

static Variant domattr_specified_read(const Object& /*obj*/) {
  /* T O D O */
  return true;
}

static Variant domattr_value_read(const Object& obj) {
  CHECK_ATTR(attrp);
  xmlChar *content = xmlNodeGetContent((xmlNodePtr) attrp);
  if (content) {
    String ret((const char*)content, CopyString);
    xmlFree(content);
    return ret;
  }
  return empty_string_variant();
}

static void domattr_value_write(const Object& obj, const Variant& value) {
  CHECK_WRITE_ATTR(attrp);
  String svalue = value.toString();
  xmlNodeSetContentLen((xmlNodePtr)attrp, (xmlChar*)svalue.data(),
                       svalue.size() + 1);
}

static Variant domattr_ownerelement_read(const Object& obj) {
  CHECK_NODE(nodep);
  return create_node_object(nodep->parent, domnode->doc());
}

static Variant domattr_schematypeinfo_read(const Object& /*obj*/) {
  raise_warning("Not yet implemented");
  return init_null();
}

static DOMPropertyAccessor domattr_properties[] = {
  { "name",           domattr_name_read,           nullptr },
  { "specified",      domattr_specified_read,      nullptr },
  { "value",          domattr_value_read,          domattr_value_write },
  { "ownerElement",   domattr_ownerelement_read,   nullptr },
  { "schemaTypeInfo", domattr_schematypeinfo_read, nullptr },
  { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domattr_properties_map
((DOMPropertyAccessor*)domattr_properties, &domnode_properties_map);

struct DOMAttrPropHandler : public DOMPropHandler<DOMAttrPropHandler> {
  static constexpr DOMPropertyAccessorMap& map = domattr_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

void HHVM_METHOD(DOMAttr, __construct,
                 const String& name,
                 const Variant& value /* = null_string */) {
  auto data = Native::data<DOMNode>(this_);
  int name_valid = xmlValidateName((xmlChar *)name.data(), 0);
  if (name_valid != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, 1);
    return;
  }
  const String& str_value = value.isNull() ? null_string : value.toString();
  data->setNode((xmlNodePtr)xmlNewProp(nullptr, (xmlChar*)name.data(),
                                       (xmlChar*)str_value.data()));

  if (!data->node()) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

Array HHVM_METHOD(DOMAttr, __debugInfo) {
  auto data = Native::data<DOMNode>(this_);
  if (!data->node()) {
    return this_->toArray();
  }
  return domattr_properties_map.debugInfo(Object{this_});
}

bool HHVM_METHOD(DOMAttr, isId) {
  auto data = Native::data<DOMNode>(this_);
  xmlAttrPtr attrp = (xmlAttrPtr)data->nodep();
  if (!attrp) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  return attrp->atype == XML_ATTRIBUTE_ID;
}

///////////////////////////////////////////////////////////////////////////////


static Variant dom_characterdata_data_read(const Object& obj) {
  CHECK_NODE(nodep);
  xmlChar *content = xmlNodeGetContent(nodep);
  if (content) {
    String ret((const char*)content, CopyString);
    xmlFree(content);
    return ret;
  }
  return empty_string_variant();
}

static void dom_characterdata_data_write(const Object& obj, const Variant& value) {
  CHECK_WRITE_NODE(nodep);
  String svalue = value.toString();
  xmlNodeSetContentLen(nodep, (xmlChar*)svalue.data(), svalue.size() + 1);
}

static Variant dom_characterdata_length_read(const Object& obj) {
  CHECK_NODE(nodep);
  int64_t length = 0;
  xmlChar *content = xmlNodeGetContent(nodep);
  if (content) {
    length = xmlUTF8Strlen(content);
    xmlFree(content);
  }
  return length;
}

static DOMPropertyAccessor domcharacterdata_properties[] = {
  { "data",   dom_characterdata_data_read,   dom_characterdata_data_write },
  { "length", dom_characterdata_length_read, nullptr },
  { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domcharacterdata_properties_map
((DOMPropertyAccessor*)domcharacterdata_properties, &domnode_properties_map);

struct DOMCharacterDataPropHandler :
  public DOMPropHandler<DOMCharacterDataPropHandler> {
  static constexpr DOMPropertyAccessorMap& map =
    domcharacterdata_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

Array HHVM_METHOD(DOMCharacterData, __debugInfo) {
  auto data = Native::data<DOMNode>(this_);
  if (!data->node()) {
    return this_->toArray();
  }
  return domcharacterdata_properties_map.debugInfo(Object{this_});
}

bool HHVM_METHOD(DOMCharacterData, appendData,
                 const String& arg) {
  auto data = Native::data<DOMNode>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  // Implement logic from libxml xmlTextConcat to add support for
  // comments and PI
  if ((nodep->content == (xmlChar *) &(nodep->properties)) ||
      ((nodep->doc != nullptr) && (nodep->doc->dict != nullptr) &&
       xmlDictOwns(nodep->doc->dict, nodep->content))) {
    nodep->content =
      xmlStrncatNew(nodep->content, (xmlChar*)arg.data(), arg.size());
  } else {
    nodep->content =
      xmlStrncat(nodep->content, (xmlChar*)arg.data(), arg.size());
  }
  nodep->properties = nullptr;
  return true;
}

bool HHVM_METHOD(DOMCharacterData, deleteData,
                 int64_t offset,
                 int64_t count) {
  auto data = Native::data<DOMNode>(this_);
  xmlNodePtr node = data->nodep();
  if (!node) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlChar *cur, *substring, *second;
  cur = xmlNodeGetContent(node);
  if (cur == nullptr) {
    return false;
  }
  int length = xmlUTF8Strlen(cur);
  if (offset < 0 || count < 0 || offset > length) {
    xmlFree(cur);
    php_dom_throw_error(INDEX_SIZE_ERR, data->doc()->m_stricterror);
    return false;
  }
  if (offset > 0) {
    substring = xmlUTF8Strsub(cur, 0, offset);
  } else {
    substring = nullptr;
  }
  if ((offset + count) > length) {
    count = length - offset;
  }
  second = xmlUTF8Strsub(cur, offset + count, length - offset);
  substring = xmlStrcat(substring, second);
  xmlNodeSetContent(node, substring);
  xmlFree(cur);
  xmlFree(second);
  xmlFree(substring);
  return true;
}

bool HHVM_METHOD(DOMCharacterData, insertData,
                 int64_t offset,
                 const String& data) {
  auto native_data = Native::data<DOMNode>(this_);
  xmlNodePtr node = native_data->nodep();
  if (!node) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlChar *cur, *first, *second;
  cur = xmlNodeGetContent(node);
  if (cur == nullptr) {
    return false;
  }
  int length = xmlUTF8Strlen(cur);
  if (offset < 0 || offset > length) {
    xmlFree(cur);
    php_dom_throw_error(INDEX_SIZE_ERR, native_data->doc()->m_stricterror);
    return false;
  }
  first = xmlUTF8Strndup(cur, offset);
  second = xmlUTF8Strsub(cur, offset, length - offset);
  xmlFree(cur);
  xmlNodeSetContent(node, first);
  xmlNodeAddContent(node, (xmlChar*)data.data());
  xmlNodeAddContent(node, second);
  xmlFree(first);
  xmlFree(second);
  return true;
}

bool HHVM_METHOD(DOMCharacterData, replaceData,
                 int64_t offset,
                 int64_t count,
                 const String& data) {
  auto native_data = Native::data<DOMNode>(this_);
  xmlNodePtr node = native_data->nodep();
  if (!node) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlChar *cur, *substring, *second = nullptr;
  cur = xmlNodeGetContent(node);
  if (cur == nullptr) {
    return false;
  }
  int length = xmlUTF8Strlen(cur);
  if (offset < 0 || count < 0 || offset > length) {
    xmlFree(cur);
    php_dom_throw_error(INDEX_SIZE_ERR, native_data->doc()->m_stricterror);
    return false;
  }
  if (offset > 0) {
    substring = xmlUTF8Strsub(cur, 0, offset);
  } else {
    substring = nullptr;
  }
  if ((offset + count) > length) {
    count = length - offset;
  }
  if (offset < length) {
    second = xmlUTF8Strsub(cur, offset + count, length - offset);
  }
  substring = xmlStrcat(substring, (xmlChar*)data.data());
  substring = xmlStrcat(substring, second);
  xmlNodeSetContent(node, substring);
  xmlFree(cur);
  if (second) {
    xmlFree(second);
  }
  xmlFree(substring);
  return true;
}

String HHVM_METHOD(DOMCharacterData, substringData,
                   int64_t offset,
                   int64_t count) {
  auto native_data = Native::data<DOMNode>(this_);
  xmlNodePtr node = native_data->nodep();
  if (!node) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlChar *cur, *substring;
  cur = xmlNodeGetContent(node);
  if (cur == nullptr) {
    return false;
  }
  int length = xmlUTF8Strlen(cur);
  if (offset < 0 || count < 0 || offset > length) {
    xmlFree(cur);
    php_dom_throw_error(INDEX_SIZE_ERR, native_data->doc()->m_stricterror);
    return false;
  }
  if ((offset + count) > length) {
    count = length - offset;
  }
  substring = xmlUTF8Strsub(cur, offset, count);
  xmlFree(cur);
  if (substring) {
    String ret((char*)substring, CopyString);
    xmlFree(substring);
    return ret;
  }
  return empty_string();
}

///////////////////////////////////////////////////////////////////////////////


void HHVM_METHOD(DOMComment, __construct,
                 const Variant& value /* = null_string */) {
  auto data = Native::data<DOMNode>(this_);
  const String& str_value = value.isNull() ? null_string : value.toString();
  data->setNode(xmlNewComment((xmlChar *)str_value.data()));
  if (!data->node()) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

///////////////////////////////////////////////////////////////////////////////


static Variant dom_text_whole_text_read(const Object& obj) {
  CHECK_NODE(node);

  /* Find starting text node */
  while (node->prev && ((node->prev->type == XML_TEXT_NODE) ||
                        (node->prev->type == XML_CDATA_SECTION_NODE))) {
    node = node->prev;
  }

  /* concatenate all adjacent text and cdata nodes */
  xmlChar *wholetext = nullptr;
  while (node && ((node->type == XML_TEXT_NODE) ||
                  (node->type == XML_CDATA_SECTION_NODE))) {
    wholetext = xmlStrcat(wholetext, node->content);
    node = node->next;
  }

  if (wholetext) {
    String ret((const char*)wholetext, CopyString);
    xmlFree(wholetext);
    return ret;
  }
  return empty_string_variant();
}

static DOMPropertyAccessor domtext_properties[] = {
  { "wholeText", dom_text_whole_text_read, nullptr },
  { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domtext_properties_map
((DOMPropertyAccessor*)domtext_properties, &domcharacterdata_properties_map);

struct DOMTextPropHandler :
  public DOMPropHandler<DOMTextPropHandler> {
  static constexpr DOMPropertyAccessorMap& map = domtext_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

void HHVM_METHOD(DOMText, __construct,
                 const Variant& value /* = null_string */) {
  auto data = Native::data<DOMNode>(this_);
  const String& str_value = value.isNull() ? null_string : value.toString();
  data->setNode(xmlNewText((xmlChar *)str_value.data()));
  if (!data->node()) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

Array HHVM_METHOD(DOMText, __debugInfo) {
  auto data = Native::data<DOMNode>(this_);
  if (!data->node()) {
    return this_->toArray();
  }
  return domtext_properties_map.debugInfo(Object{this_});
}

bool HHVM_METHOD(DOMText, isWhitespaceInElementContent) {
  auto data = Native::data<DOMNode>(this_);
  if (!data->nodep()) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  return xmlIsBlankNode(data->nodep());
}

bool HHVM_METHOD(DOMText, isElementContentWhitespace) {
  auto data = Native::data<DOMNode>(this_);
  if (!data->nodep()) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  return xmlIsBlankNode(data->nodep());
}

Variant HHVM_METHOD(DOMText, splitText,
                    int64_t offset) {
  auto data = Native::data<DOMNode>(this_);
  xmlNodePtr node = data->nodep();
  if (!node) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlChar *cur, *first, *second;
  xmlNodePtr nnode;
  if (node->type != XML_TEXT_NODE && node->type != XML_CDATA_SECTION_NODE) {
    return false;
  }
  cur = xmlNodeGetContent(node);
  if (cur == nullptr) {
    return false;
  }
  int length = xmlUTF8Strlen(cur);
  if (offset > length || offset < 0) {
    xmlFree(cur);
    return false;
  }
  first = xmlUTF8Strndup(cur, offset);
  second = xmlUTF8Strsub(cur, offset, length - offset);
  xmlFree(cur);
  xmlNodeSetContent(node, first);
  nnode = xmlNewDocText(node->doc, second);
  xmlFree(first);
  xmlFree(second);
  if (nnode == nullptr) {
    return false;
  }
  if (node->parent != nullptr) {
    nnode->type = XML_ELEMENT_NODE;
    xmlAddNextSibling(node, nnode);
    nnode->type = XML_TEXT_NODE;
  }
  Object ret{DOMText::classof()};
  auto text_data = Native::data<DOMNode>(ret);
  text_data->setNode(nnode);
  text_data->setDoc(data->doc());
  return ret;
}

///////////////////////////////////////////////////////////////////////////////


void HHVM_METHOD(DOMCdataSection, __construct,
                 const String& value) {
  auto data = Native::data<DOMNode>(this_);
  data->setNode(xmlNewCDataBlock(nullptr, (xmlChar *)value.data(),
                                 value.size()));
  if (!data->node()) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

///////////////////////////////////////////////////////////////////////////////


#define CHECK_DOC(docp)                                                        \
  auto domdoc = getDOMNode(obj);                                               \
  CHECK_THIS_DOC(docp, domdoc);                                                \

#define CHECK_THIS_DOC(docp, domdoc)                                           \
  xmlDocPtr docp = (xmlDocPtr)(domdoc ? domdoc->nodep() : nullptr);            \
  if (docp == nullptr) {                                                       \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                                 \
    return init_null();                                                        \
  }                                                                            \

#define CHECK_WRITE_DOC(docp)                                                  \
  auto domdoc = getDOMNode(obj);                                               \
  CHECK_WRITE_THIS_DOC(docp, domdoc)                                           \

#define CHECK_WRITE_THIS_DOC(docp, domdoc)                                     \
  xmlDocPtr docp = (xmlDocPtr)(domdoc ? domdoc->nodep() : nullptr);            \
  if (docp == nullptr) {                                                       \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                                 \
    return;                                                                    \
  }                                                                            \

static Variant dom_document_doctype_read(const Object& obj) {
  CHECK_DOC(docp);
  auto const& dtd = (xmlNodePtr)xmlGetIntSubset(docp);
  if (dtd == nullptr) {
    return init_null();
  }
  return create_node_object(dtd, obj);
}

static Variant dom_document_implementation_read(const Object& /*obj*/) {
  return Object{DOMImplementation::classof()};
}

static Variant dom_document_document_element_read(const Object& obj) {
  CHECK_DOC(docp);
  return create_node_object(xmlDocGetRootElement(docp), obj);
}

static Variant dom_document_encoding_read(const Object& obj) {
  CHECK_DOC(docp);
  char *encoding = (char *) docp->encoding;
  if (encoding) {
    return String(encoding, CopyString);
  }
  return init_null();
}

static void dom_document_encoding_write(const Object& obj,
                                        const Variant& value) {
  CHECK_WRITE_DOC(docp);

  String svalue = value.toString();
  xmlCharEncodingHandlerPtr handler =
    xmlFindCharEncodingHandler(svalue.data());

  if (handler) {
    xmlCharEncCloseFunc(handler);
    if (docp->encoding) {
      xmlFree((xmlChar *)docp->encoding);
    }
    docp->encoding = xmlStrdup((const xmlChar *)svalue.data());
  } else {
    raise_warning("Invalid Document Encoding");
  }
}

static Variant dom_document_standalone_read(const Object& obj) {
  CHECK_DOC(docp);
  return (bool)docp->standalone;
}

static void dom_document_standalone_write(const Object& obj,
                                          const Variant& value) {
  CHECK_WRITE_DOC(docp);
  int64_t standalone = value.toInt64();
  if (standalone > 0) {
    docp->standalone = 1;
  } else if (standalone < 0) {
    docp->standalone = -1;
  } else {
    docp->standalone = 0;
  }
}

static Variant dom_document_version_read(const Object& obj) {
  CHECK_DOC(docp);
  char *version = (char *) docp->version;
  if (version) {
    return String(version, CopyString);
  }
  return init_null();
}

static void dom_document_version_write(const Object& obj,
                                       const Variant& value) {
  CHECK_WRITE_DOC(docp);
  if (docp->version != nullptr) {
    xmlFree((xmlChar *)docp->version);
  }
  String svalue = value.toString();
  docp->version = xmlStrdup((const xmlChar *)svalue.data());
}

#define DOCPROP_READ_WRITE(member, name)                                \
  static Variant dom_document_ ##name## _read(const Object& obj) {      \
    CHECK_DOC(docp)                                                     \
    return (bool)domdoc->doc()->m_ ## member;                           \
  }                                                                     \
  static void dom_document_ ##name## _write(const Object& obj,          \
                                            const Variant& value) {     \
    CHECK_WRITE_DOC(docp)                                               \
    domdoc->doc()->m_ ## member = value.toBoolean();                    \
  }                                                                     \
/**/

DOCPROP_READ_WRITE(stricterror,        strict_error_checking );
DOCPROP_READ_WRITE(formatoutput,       format_output         );
DOCPROP_READ_WRITE(validateonparse,    validate_on_parse     );
DOCPROP_READ_WRITE(resolveexternals,   resolve_externals     );
DOCPROP_READ_WRITE(preservewhitespace, preserve_whitespace   );
DOCPROP_READ_WRITE(recover,            recover               );
DOCPROP_READ_WRITE(substituteentities, substitue_entities    );

static Variant dom_document_document_uri_read(const Object& obj) {
  CHECK_DOC(docp);
  char *url = (char *)docp->URL;
  if (url) {
    return String(url, CopyString);
  }
  return init_null();
}

static void dom_document_document_uri_write(const Object& obj,
                                            const Variant& value) {
  CHECK_WRITE_DOC(docp);
  if (docp->URL != nullptr) {
    xmlFree((xmlChar *) docp->URL);
  }
  String svalue = value.toString();
  docp->URL = xmlStrdup((const xmlChar *)svalue.data());
}

static Variant dom_document_config_read(const Object& /*obj*/) {
  return init_null();
}

/* }}} */

static DOMPropertyAccessor domdocument_properties[] = {
  { "doctype",             dom_document_doctype_read,          nullptr },
  { "implementation",      dom_document_implementation_read,   nullptr },
  { "documentElement",     dom_document_document_element_read, nullptr },
  { "actualEncoding",      dom_document_encoding_read,         nullptr },
  { "encoding",            dom_document_encoding_read,
    dom_document_encoding_write },
  { "xmlEncoding",         dom_document_encoding_read,         nullptr },
  { "standalone",          dom_document_standalone_read,
    dom_document_standalone_write },
  { "xmlStandalone",       dom_document_standalone_read,
    dom_document_standalone_write },
  { "version",             dom_document_version_read,
    dom_document_version_write },
  { "xmlVersion",          dom_document_version_read,
    dom_document_version_write },
  { "strictErrorChecking", dom_document_strict_error_checking_read,
    dom_document_strict_error_checking_write },
  { "documentURI",         dom_document_document_uri_read,
    dom_document_document_uri_write },
  { "config",              dom_document_config_read,           nullptr },
  { "formatOutput",        dom_document_format_output_read,
    dom_document_format_output_write },
  { "validateOnParse",     dom_document_validate_on_parse_read,
    dom_document_validate_on_parse_write },
  { "resolveExternals",    dom_document_resolve_externals_read,
    dom_document_resolve_externals_write },
  { "preserveWhiteSpace",  dom_document_preserve_whitespace_read,
    dom_document_preserve_whitespace_write },
  { "recover",             dom_document_recover_read,
    dom_document_recover_write },
  { "substituteEntities",  dom_document_substitue_entities_read,
    dom_document_substitue_entities_write },
  { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domdocument_properties_map
((DOMPropertyAccessor*)domdocument_properties, &domnode_properties_map);

struct DOMDocumentPropHandler : public DOMPropHandler<DOMDocumentPropHandler> {
  static constexpr DOMPropertyAccessorMap& map = domdocument_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

void HHVM_METHOD(DOMDocument, __construct,
                 const Variant& version /* = null_string */,
                 const Variant& encoding /* = null_string */) {
  xmlDoc *docp = xmlNewDoc(
      version.isNull() ? nullptr : (xmlChar*)version.toString().data());
  if (!docp) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
    return;
  }
  const String& str_encoding = encoding.isNull()
    ? null_string : encoding.toString();
  if (str_encoding.size() > 0) {
    docp->encoding = (const xmlChar*)xmlStrdup((xmlChar*)str_encoding.data());
  }
  auto* data = Native::data<DOMNode>(this_);
  data->setNode((xmlNodePtr)docp);
}

Array HHVM_METHOD(DOMDocument, __debugInfo) {
  auto* data = Native::data<DOMNode>(this_);
  if (!data->node()) {
    return this_->toArray();
  }
  return domdocument_properties_map.debugInfo(Object{this_});
}

Variant HHVM_METHOD(DOMDocument, createAttribute,
                    const String& name) {
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  if (xmlValidateName((xmlChar*)name.data(), 0) != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, data->doc()->m_stricterror);
    return false;
  }
  xmlAttrPtr node = xmlNewDocProp(docp, (xmlChar*)name.data(), nullptr);
  if (!node) {
    return false;
  }
  node->doc = docp;
  auto ret = php_dom_create_object((xmlNodePtr)node, data->doc());
  if (ret.isNull()) return false;
  return ret;
}

Variant HHVM_METHOD(DOMDocument, createAttributeNS,
                    const String& namespaceuri,
                    const String& qualifiedname) {
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  xmlNodePtr nodep = nullptr;
  xmlNsPtr nsptr;
  char *localname = nullptr, *prefix = nullptr;
  int errorcode;
  xmlNodePtr root = xmlDocGetRootElement(docp);
  if (root != nullptr) {
    errorcode = dom_check_qname((char*)qualifiedname.data(), &localname,
                                &prefix, namespaceuri.size(),
                                qualifiedname.size());
    if (errorcode == 0) {
      if (xmlValidateName((xmlChar*)localname, 0) == 0) {
        nodep = (xmlNodePtr)xmlNewDocProp(docp, (xmlChar*)localname, nullptr);
        if (nodep != nullptr && namespaceuri.size() > 0) {
          nsptr = xmlSearchNsByHref(nodep->doc, root,
                                    (xmlChar*)namespaceuri.data());
          if (nsptr == nullptr) {
            nsptr = dom_get_ns(root, (char*)namespaceuri.data(),
                               &errorcode, prefix);
          }
          xmlSetNs(nodep, nsptr);
        }
      } else {
        errorcode = INVALID_CHARACTER_ERR;
      }
    }
  } else {
    raise_warning("Document Missing Root Element");
    return false;
  }
  xmlFree(localname);
  if (prefix != nullptr) {
    xmlFree(prefix);
  }
  if (errorcode != 0) {
    if (nodep != nullptr) {
      xmlFreeProp((xmlAttrPtr) nodep);
    }
    php_dom_throw_error((dom_exception_code)errorcode,
                        data->doc()->m_stricterror);
    return false;
  }
  if (nodep == nullptr) {
    return false;
  }
  nodep->doc = docp;
  auto ret = php_dom_create_object(nodep, data->doc());
  if (ret.isNull()) return false;
  return ret;
}

Variant HHVM_METHOD(DOMDocument, createCDATASection,
                    const String& data) {
  auto* native_data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)native_data->nodep();
  xmlNode *node = xmlNewCDataBlock(docp, (xmlChar*)data.data(), data.size());
  if (!node) {
    return false;
  }
  node->doc = docp;
  auto ret = php_dom_create_object(node, native_data->doc());
  if (ret.isNull()) return false;
  return ret;
}

Variant HHVM_METHOD(DOMDocument, createComment,
                    const String& data) {
  auto* native_data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)native_data->nodep();
  xmlNode *node = xmlNewComment((xmlChar*)data.data());
  if (!node) {
    return false;
  }
  node->doc = docp;
  auto ret = php_dom_create_object(node, native_data->doc());
  if (ret.isNull()) return false;
  return ret;
}

Variant HHVM_METHOD(DOMDocument, createDocumentFragment) {
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  xmlNode *node = xmlNewDocFragment(docp);
  if (!node) {
    return false;
  }
  node->doc = docp;
  auto ret = php_dom_create_object(node, data->doc());
  if (ret.isNull()) return false;
  return ret;
}

Variant HHVM_METHOD(DOMDocument, createElement,
                    const String& name,
                    const Variant& value /*= null_string*/) {
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  if (xmlValidateName((xmlChar*)name.data(), 0) != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR,
                        data->doc()->m_stricterror);
    return false;
  }
  const String& str_value = value.isNull() ? null_string : value.toString();
  xmlNode *node = xmlNewDocNode(docp, nullptr, (xmlChar*)name.data(),
                                (xmlChar*)getStringData(str_value));
  if (!node) {
    return false;
  }
  node->doc = docp;
  auto ret = php_dom_create_object(node, data->doc());
  if (ret.isNull()) return false;
  return ret;
}

Variant HHVM_METHOD(DOMDocument, createElementNS,
                    const String& namespaceuri,
                    const String& qualifiedname,
                    const Variant& value /*= null_string*/) {
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  xmlNodePtr nodep = nullptr;
  xmlNsPtr nsptr = nullptr;
  char *localname = nullptr, *prefix = nullptr;
  int errorcode = dom_check_qname((char*)qualifiedname.data(), &localname,
                                  &prefix, namespaceuri.size(),
                                  qualifiedname.size());
  if (errorcode == 0) {
    if (xmlValidateName((xmlChar*)localname, 0) == 0) {
      const String& str_value = value.isNull() ? null_string : value.toString();
      nodep = xmlNewDocNode(docp, nullptr, (xmlChar*)localname,
                            (xmlChar*)getStringData(str_value));
      if (nodep != nullptr && !namespaceuri.isNull()) {
        nsptr = xmlSearchNsByHref(nodep->doc, nodep,
                                  (xmlChar*)namespaceuri.data());
        if (nsptr == nullptr) {
          nsptr = dom_get_ns(nodep, (char*)namespaceuri.data(),
                             &errorcode, prefix);
        }
        xmlSetNs(nodep, nsptr);
      }
    } else {
      errorcode = INVALID_CHARACTER_ERR;
    }
  }
  xmlFree(localname);
  if (prefix != nullptr) {
    xmlFree(prefix);
  }
  if (errorcode != 0) {
    if (nodep != nullptr) {
      xmlFreeNode(nodep);
    }
    php_dom_throw_error((dom_exception_code)errorcode,
                        data->doc()->m_stricterror);
    return false;
  }
  if (nodep == nullptr) {
    return false;
  }
  nodep->doc = docp;
  nodep->ns = nsptr;
  auto ret = php_dom_create_object(nodep, data->doc());
  if (ret.isNull()) return false;
  return ret;
}

Variant HHVM_METHOD(DOMDocument, createEntityReference,
                    const String& name) {
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  if (xmlValidateName((xmlChar*)name.data(), 0) != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, data->doc()->m_stricterror);
    return false;
  }
  xmlNode *node = xmlNewReference(docp, (xmlChar*)name.data());
  if (!node) {
    return false;
  }
  node->doc = docp;
  auto ret = php_dom_create_object(node, data->doc());
  if (ret.isNull()) return false;
  return ret;
}

Variant HHVM_METHOD(DOMDocument, createProcessingInstruction,
                    const String& target,
                    const Variant& data /* = null_string */) {
  auto* native_data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)native_data->nodep();
  if (xmlValidateName((xmlChar*)target.data(), 0) != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR,
                        native_data->doc()->m_stricterror);
    return false;
  }
  const String& str_data = data.isNull() ? null_string : data.toString();
  xmlNode *node = xmlNewPI((xmlChar*)target.data(),
                           (xmlChar*)getStringData(str_data));
  if (!node) {
    return false;
  }
  node->doc = docp;
  auto ret = php_dom_create_object(node, native_data->doc());
  if (ret.isNull()) return false;
  return ret;
}

Variant HHVM_METHOD(DOMDocument, createTextNode,
                    const String& data) {
  auto* native_data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)native_data->nodep();
  xmlNode *node = xmlNewDocText(docp, (xmlChar*)data.data());
  if (!node) {
    return false;
  }
  node->doc = docp;
  auto ret = php_dom_create_object(node, native_data->doc());
  if (ret.isNull()) return false;
  return ret;
}

Variant HHVM_METHOD(DOMDocument, getElementById,
                    const String& elementid) {
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  xmlAttrPtr attrp = xmlGetID(docp, (xmlChar*)elementid.data());
  if (attrp && attrp->parent) {
    return create_node_object(attrp->parent, Object{this_});
  }

  return init_null();
}

ObjectRet HHVM_METHOD(DOMDocument, getElementsByTagName,
                      const String& name) {
  auto* data = Native::data<DOMNode>(this_);
  return newDOMNodeList(data->doc(), Object{this_}, 0, name);
}

ObjectRet HHVM_METHOD(DOMDocument, getElementsByTagNameNS,
                      const String& namespaceuri,
                      const String& localname) {
  auto* data = Native::data<DOMNode>(this_);
  return newDOMNodeList(data->doc(), Object{this_}, 0, localname, namespaceuri);
}

Variant HHVM_METHOD(DOMDocument, importNode,
                    const Object& importednode,
                    bool deep /* = false */) {
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  DOMNode *domnode = getDOMNode(importednode);
  xmlNodePtr nodep = domnode->nodep();
  xmlNodePtr retnodep;
  long recursive = deep ? 1 : 0;
  if (nodep->type == XML_HTML_DOCUMENT_NODE ||
      nodep->type == XML_DOCUMENT_NODE ||
      nodep->type == XML_DOCUMENT_TYPE_NODE) {
    raise_warning("Cannot import: Node Type Not Supported");
    return false;
  }
  if (nodep->doc == docp) {
    retnodep = nodep;
  } else {
    if ((recursive == 0) && (nodep->type == XML_ELEMENT_NODE)) {
      recursive = 2;
    }
    retnodep = xmlDocCopyNode(nodep, docp, recursive);
    if (!retnodep) {
      return false;
    }
  }
  if ((retnodep->type == XML_ATTRIBUTE_NODE) && (nodep->ns != nullptr)) {
    xmlNsPtr nsptr = nullptr;
    xmlNodePtr root = xmlDocGetRootElement(docp);

    nsptr = xmlSearchNsByHref (nodep->doc, root, nodep->ns->href);
    if (nsptr == nullptr) {
      int errorcode;
      nsptr = dom_get_ns(root, (char *) nodep->ns->href, &errorcode, (char *) nodep->ns->prefix);
    }
    xmlSetNs(retnodep, nsptr);
  }
  return create_node_object(retnodep, data->doc());
}

void HHVM_METHOD(DOMDocument, normalizeDocument) {
  auto* data = Native::data<DOMNode>(this_);
  CHECK_WRITE_THIS_DOC(docp, data);
  dom_normalize(data->nodep());
}

bool HHVM_METHOD(DOMDocument, registerNodeClass,
                 const String& baseclass,
                 const String& extendedclass) {
  if (!HHVM_FN(class_exists)(baseclass)) {
    raise_error("Class %s does not exist", baseclass.data());
    return false;
  }
  if (!HHVM_FN(is_a)(baseclass, "DOMNode", true)) {
    raise_error("Class %s is not DOMNode or derived from it.",
                baseclass.data());
    return false;
  }
  if (!HHVM_FN(class_exists)(extendedclass)) {
    raise_error("Class %s does not exist", extendedclass.data());
    return false;
  }
  if (!HHVM_FN(is_subclass_of)(extendedclass, baseclass)) {
    raise_error("Class %s is not derived from %s.", extendedclass.data(),
                baseclass.data());
    return false;
  }
  auto* data = Native::data<DOMNode>(this_);
  data->doc()->m_classmap.set(HHVM_FN(strtolower)(baseclass), extendedclass);
  return true;
}

bool HHVM_METHOD(DOMDocument, relaxNGValidate,
                 const String& filename) {
  VMRegGuard _;
  auto* data = Native::data<DOMNode>(this_);
  return _dom_document_relaxNG_validate(data, filename, DOM_LOAD_FILE);
}

bool HHVM_METHOD(DOMDocument, relaxNGValidateSource, const String& source) {
  VMRegGuard _;
  auto* data = Native::data<DOMNode>(this_);
  return _dom_document_relaxNG_validate(data, source, DOM_LOAD_STRING);
}

Variant HHVM_METHOD(DOMDocument, save,
                    const String& file,
                    int64_t options /* = 0 */) {
  VMRegGuard _;
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  int bytes, format = 0, saveempty = 0;

  /* encoding handled by property on doc */
  format = data->doc()->m_formatoutput;
  if (options & LIBXML_SAVE_NOEMPTYTAG) {
    saveempty = xmlSaveNoEmptyTags;
    xmlSaveNoEmptyTags = 1;
  }
  bytes = xmlSaveFormatFileEnc(file.data(), docp, nullptr, format);
  if (options & LIBXML_SAVE_NOEMPTYTAG) {
    xmlSaveNoEmptyTags = saveempty;
  }
  if (bytes == -1) {
    return false;
  }
  return bytes;
}

Variant HHVM_METHOD(DOMDocument, saveHTMLFile,
                    const String& file) {
  VMRegGuard _;
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  int bytes, format = 0;

  /* encoding handled by property on doc */
  format = data->doc()->m_formatoutput;
  bytes = htmlSaveFileFormat(file.data(), docp, nullptr, format);
  if (bytes == -1) {
    return false;
  }
  return bytes;
}

Variant HHVM_METHOD(DOMDocument, saveXML,
                    const Variant& node /* = null_object */,
                    int64_t options /* = 0 */) {
  auto* data = Native::data<DOMNode>(this_);
  int saveempty = 0;

  if (options & LIBXML_SAVE_NOEMPTYTAG) {
    saveempty = xmlSaveNoEmptyTags;
    xmlSaveNoEmptyTags = 1;
  }

  const Object& obj_node = node.isNull() ? null_object : node.toObject();
  Variant ret = save_html_or_xml(data, /* as_xml = */ true, obj_node);

  if (options & LIBXML_SAVE_NOEMPTYTAG) {
    xmlSaveNoEmptyTags = saveempty;
  }

  return ret;
}

Variant HHVM_METHOD(DOMDocument, saveHTML,
                    const Variant& node /* = uninit_variant */) {
  auto* data = Native::data<DOMNode>(this_);
  const Object& obj_node = node.isNull() ? null_object : node.toObject();
  return save_html_or_xml(data, /* as_xml = */ false, obj_node);
}

Variant save_html_or_xml(DOMNode* obj,
                         bool as_xml,
                         const Object& node /* = null_object */) {
  VMRegGuard _;
  xmlDocPtr docp = (xmlDocPtr)obj->nodep();
  xmlBufferPtr buf;
  xmlChar *mem;
  int size;
  if (!node.isNull()) {
    DOMNode* domnode = getDOMNode(node);
    xmlNodePtr node = domnode->nodep();
    if (!node) {
      php_dom_throw_error(INVALID_STATE_ERR, 0);
      return false;
    }
    /* Dump contents of Node */
    if (node->doc != docp) {
      php_dom_throw_error(WRONG_DOCUMENT_ERR, obj->doc()->m_stricterror);
      return false;
    }
    buf = xmlBufferCreate();
    if (!buf) {
      raise_warning("Could not fetch buffer");
      return false;
    }
    if (as_xml) {
      xmlNodeDump(buf, docp, node, 0, obj->doc()->m_formatoutput);
    } else {
      htmlNodeDump(buf, docp, node);
    }
    mem = (xmlChar*)xmlBufferContent(buf);
    if (!mem) {
      xmlBufferFree(buf);
      return false;
    }
    String ret = String((char*)mem, CopyString);
    xmlBufferFree(buf);
    return ret;
  }

  if (as_xml) {
    xmlDocDumpFormatMemory(docp, &mem, &size, obj->doc()->m_formatoutput);
  } else {
#if LIBXML_VERSION >= 20623
    htmlDocDumpMemoryFormat(docp, &mem, &size, obj->doc()->m_formatoutput);
#else
    htmlDocDumpMemory(docp, &mem, &size);
#endif
  }

  if (!size) {
    if (mem) xmlFree(mem);
    return false;
  }
  String ret = String((char*)mem, size, CopyString);
  xmlFree(mem);
  return ret;
}

bool HHVM_METHOD(DOMDocument, schemaValidate,
                 const String& filename) {
  VMRegGuard _;
  auto* data = Native::data<DOMNode>(this_);
  return _dom_document_schema_validate(data, filename, DOM_LOAD_FILE);
}

bool HHVM_METHOD(DOMDocument, schemaValidateSource,
                 const String& source) {
  VMRegGuard _;
  auto* data = Native::data<DOMNode>(this_);
  return _dom_document_schema_validate(data, source, DOM_LOAD_STRING);
}

bool HHVM_METHOD(DOMDocument, validate) {
  VMRegGuard _;
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  xmlValidCtxt *cvp;
  if (docp->intSubset == nullptr) {
    raise_notice("No DTD given in XML-Document");
  }
  cvp = xmlNewValidCtxt();
  cvp->userData = nullptr;
  cvp->error    = (xmlValidityErrorFunc) php_libxml_ctx_error;
  cvp->warning  = (xmlValidityErrorFunc) php_libxml_ctx_error;
  bool ret = xmlValidateDocument(cvp, docp);
  xmlFreeValidCtxt(cvp);
  return ret;
}

Variant HHVM_METHOD(DOMDocument, xinclude,
                    int64_t options /* = 0 */) {
  VMRegGuard _;
  auto* data = Native::data<DOMNode>(this_);
  xmlDocPtr docp = (xmlDocPtr)data->nodep();
  int err = xmlXIncludeProcessFlags(docp, options);
  /* XML_XINCLUDE_START and XML_XINCLUDE_END nodes need to be removed as these
     are added via xmlXIncludeProcess to mark beginning and ending of
     xincluded document, but are not wanted in resulting document - must be
     done even if err as it could fail after having processed some xincludes */
  xmlNodePtr root = (xmlNodePtr) docp->children;
  while (root && root->type != XML_ELEMENT_NODE &&
         root->type != XML_XINCLUDE_START) {
    root = root->next;
  }
  if (root) {
    php_dom_remove_xinclude_nodes(root);
  }
  if (err) {
    return err;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////


void HHVM_METHOD(DOMDocumentFragment, __construct) {
  auto data = Native::data<DOMNode>(this_);
  data->setNode(xmlNewDocFragment(nullptr));
  if (!data->node()) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

bool HHVM_METHOD(DOMDocumentFragment, appendXML,
                 const String& data) {
  auto native_data = Native::data<DOMNode>(this_);
  xmlNodePtr nodep = native_data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  bool stricterror = native_data->doc()
    ? native_data->doc()->m_stricterror
    : true;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  if (!data.empty()) {
    xmlNodePtr lst;
    if (xmlParseBalancedChunkMemory(nodep->doc, nullptr, nullptr, 0,
                                    (xmlChar*)data.data(), &lst)) {
      return false;
    }
    /* Following needed due to bug in libxml2 <= 2.6.14
       ifdef after next libxml release as bug is fixed in their cvs */
    php_dom_xmlSetTreeDoc(lst, nodep->doc);
    /* End stupid hack */
    xmlAddChildList(nodep, lst);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

#define CHECK_DOCTYPE(dtdptr)                                                  \
  auto domdoctype = getDOMNode(obj);                                           \
  xmlDtdPtr dtdptr = (xmlDtdPtr)(domdoctype ? domdoctype->nodep()              \
                                            : nullptr);                        \
  if (dtdptr == nullptr) {                                                     \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                                 \
    return init_null();                                                        \
  }                                                                            \

static Variant dom_documenttype_name_read(const Object& obj) {
  CHECK_DOCTYPE(dtdptr);
  return String((char *)(dtdptr->name), CopyString);
}

static Variant dom_documenttype_entities_read(const Object& obj) {
  CHECK_DOCTYPE(doctypep);
  return newDOMNamedNodeMap(domdoctype->doc(), obj,
                            XML_ENTITY_NODE,
                            (xmlHashTable*) doctypep->entities);
}

static Variant dom_documenttype_notations_read(const Object& obj) {
  CHECK_DOCTYPE(doctypep);
  return newDOMNamedNodeMap(domdoctype->doc(), obj,
                            XML_NOTATION_NODE,
                            (xmlHashTable*) doctypep->notations);
}

static Variant dom_documenttype_public_id_read(const Object& obj) {
  CHECK_DOCTYPE(dtdptr);
  if (dtdptr->ExternalID) {
    return String((char *)(dtdptr->ExternalID), CopyString);
  }
  return empty_string_variant();
}

static Variant dom_documenttype_system_id_read(const Object& obj) {
  CHECK_DOCTYPE(dtdptr);
  if (dtdptr->SystemID) {
    return String((char *)(dtdptr->SystemID), CopyString);
  }
  return empty_string_variant();
}

static Variant dom_documenttype_internal_subset_read(const Object& obj) {
  CHECK_DOCTYPE(dtdptr);

  xmlDtd *intsubset;
  xmlOutputBuffer *buff = nullptr;
  xmlChar *strintsubset;
  if (dtdptr->doc != nullptr &&
      ((intsubset = dtdptr->doc->intSubset) != nullptr)) {
    buff = xmlAllocOutputBuffer(nullptr);
    if (buff != nullptr) {
      xmlNodeDumpOutput (buff, nullptr, (xmlNodePtr) intsubset, 0, 0, nullptr);
      xmlOutputBufferFlush(buff);
      strintsubset = xmlStrndup(xmlOutputBufferGetContent(buff),
                                xmlOutputBufferGetSize(buff));
      (void)xmlOutputBufferClose(buff);
      return String((char *)strintsubset, CopyString);
    }
  }
  return empty_string_variant();
}

static DOMPropertyAccessor domdocumenttype_properties[] = {
  { "name",           dom_documenttype_name_read,            nullptr },
  { "entities",       dom_documenttype_entities_read,        nullptr },
  { "notations",      dom_documenttype_notations_read,       nullptr },
  { "publicId",       dom_documenttype_public_id_read,       nullptr },
  { "systemId",       dom_documenttype_system_id_read,       nullptr },
  { "internalSubset", dom_documenttype_internal_subset_read, nullptr },
  { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domdocumenttype_properties_map
((DOMPropertyAccessor*)domdocumenttype_properties, &domnode_properties_map);

struct DOMDocumentTypePropHandler :
  public DOMPropHandler<DOMDocumentTypePropHandler> {
  static constexpr DOMPropertyAccessorMap& map = domdocumenttype_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

Array HHVM_METHOD(DOMDocumentType, __debugInfo) {
  auto data = Native::data<DOMNode>(this_);
  if (!data->node()) {
    return this_->toArray();
  }
  return domdocumenttype_properties_map.debugInfo(Object{this_});
}

///////////////////////////////////////////////////////////////////////////////


static Variant dom_element_tag_name_read(const Object& obj) {
  CHECK_NODE(nodep);
  xmlChar *qname;
  xmlNsPtr ns = nodep->ns;
  if (ns != nullptr && ns->prefix) {
    qname = xmlStrdup(ns->prefix);
    qname = xmlStrcat(qname, (xmlChar *)":");
    qname = xmlStrcat(qname, nodep->name);
    String ret((char *)qname, CopyString);
    xmlFree(qname);
    return ret;
  }
  return String((char *)nodep->name, CopyString);
}

static Variant dom_element_schema_type_info_read(const Object& /*obj*/) {
  return init_null();
}

static DOMPropertyAccessor domelement_properties[] = {
  { "tagName",        dom_element_tag_name_read,         nullptr},
  { "schemaTypeInfo", dom_element_schema_type_info_read, nullptr},
  { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domelement_properties_map
((DOMPropertyAccessor*)domelement_properties, &domnode_properties_map);

///////////////////////////////////////////////////////////////////////////////

struct DOMElementPropHandler : public DOMPropHandler<DOMElementPropHandler> {
  static constexpr DOMPropertyAccessorMap& map = domelement_properties_map;
};

void HHVM_METHOD(DOMElement, __construct,
                 const String& name,
                 const Variant& value /* = null_string */,
                 const Variant& namespaceuri /*= null_string*/) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr nodep = nullptr;
  char *localname = nullptr, *prefix = nullptr;
  int errorcode = 0;
  xmlNsPtr nsptr = nullptr;

  int name_valid = xmlValidateName((xmlChar *) name.data(), 0);
  if (name_valid != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, 1);
    return;
  }

  /* Namespace logic is separate and only when uri passed in to insure
     no BC breakage */
  const String& str_namespaceuri = namespaceuri.isNull()
                                 ? null_string
                                 : namespaceuri.toString();
  if (!str_namespaceuri.empty()) {
    errorcode = dom_check_qname(name.data(), &localname, &prefix,
                                str_namespaceuri.size(), name.size());
    if (errorcode == 0) {
      nodep = xmlNewNode (nullptr, (xmlChar *)localname);
      if (nodep != nullptr && !str_namespaceuri.empty()) {
        nsptr = dom_get_ns(nodep, str_namespaceuri.data(), &errorcode, prefix);
        xmlSetNs(nodep, nsptr);
      }
    }
    xmlFree(localname);
    if (prefix != nullptr) {
      xmlFree(prefix);
    }
    if (errorcode != 0) {
      if (nodep != nullptr) {
        xmlFreeNode(nodep);
      }
      php_dom_throw_error((dom_exception_code)errorcode, 1);
      return;
    }
  } else {
    /* If you don't pass a namespace uri, then you can't set a prefix */
    localname = (char*)xmlSplitQName2((xmlChar *)name.data(),
                                      (xmlChar **)&prefix);
    if (prefix != nullptr) {
      xmlFree(localname);
      xmlFree(prefix);
      php_dom_throw_error(NAMESPACE_ERR, 1);
      return;
    }
    nodep = xmlNewNode(nullptr, (xmlChar *) name.data());
  }

  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
    return;
  }

  const String& str_value = value.isNull() ? null_string : value.toString();
  if (!str_value.empty()) {
    xmlNodeSetContentLen(nodep, (xmlChar *)str_value.data(), str_value.size());
  }
  data->setNode(nodep);
}

Array HHVM_METHOD(DOMElement, __debugInfo) {
  auto* data = Native::data<DOMElement>(this_);
  if (!data->node()) {
    return this_->toArray();
  }
  return domelement_properties_map.debugInfo(Object{this_});
}

String HHVM_METHOD(DOMElement, getAttribute,
                   const String& name) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return empty_string();
  }
  xmlChar *value = nullptr;
  xmlNodePtr attr;
  attr = dom_get_dom1_attribute(nodep, (xmlChar*)name.data());
  if (attr) {
    switch (attr->type) {
    case XML_ATTRIBUTE_NODE:
      value = xmlNodeListGetString(attr->doc, attr->children, 1);
      break;
    case XML_NAMESPACE_DECL:
      value = xmlStrdup(((xmlNsPtr)attr)->href);
      break;
    default:
      value = xmlStrdup(((xmlAttributePtr)attr)->defaultValue);
    }
  }
  if (value) {
    String ret((char*)value, CopyString);
    xmlFree(value);
    return ret;
  }
  return empty_string();
}

Variant HHVM_METHOD(DOMElement, getAttributeNode,
                    const String& name) {
  auto data = Native::data<DOMElement>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlNodePtr attrp;
  attrp = dom_get_dom1_attribute(nodep, (xmlChar*)name.data());
  if (attrp == nullptr) {
    return false;
  }
  if (attrp->type == XML_NAMESPACE_DECL) {
    xmlNsPtr curns;
    //xmlNodePtr nsparent;
    //nsparent = attrp->_private;
    curns = xmlNewNs(nullptr, attrp->name, nullptr);
    if (attrp->children) {
      curns->prefix = xmlStrdup((xmlChar*)attrp->children);
    }
    if (attrp->children) {
      attrp = xmlNewDocNode(nodep->doc, nullptr, (xmlChar *) attrp->children,
                            attrp->name);
    } else {
      attrp = xmlNewDocNode(nodep->doc, nullptr, (xmlChar *)"xmlns",
                            attrp->name);
    }
    attrp->type = XML_NAMESPACE_DECL;
    //attrp->parent = nsparent;
    attrp->ns = curns;
  }
  return php_dom_create_object((xmlNodePtr)attrp, data->doc());
}

Variant HHVM_METHOD(DOMElement, getAttributeNodeNS,
                    const String& namespaceuri,
                    const String& localname) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr elemp = data->nodep();
  if (!elemp) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return init_null();
  }
  xmlAttrPtr attrp;
  attrp = xmlHasNsProp(elemp, (xmlChar*)localname.data(),
                       (xmlChar*)namespaceuri.data());
  if (attrp == nullptr) {
    return init_null();
  }
  return php_dom_create_object((xmlNodePtr)attrp, data->doc());
}

String HHVM_METHOD(DOMElement, getAttributeNS,
                   const String& namespaceuri,
                   const String& localname) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr elemp = data->nodep();
  if (!elemp) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return empty_string();
  }
  xmlNsPtr nsptr;
  xmlChar *strattr;
  strattr = xmlGetNsProp(elemp, (xmlChar*)localname.data(),
                         (xmlChar*)namespaceuri.data());
  if (strattr != nullptr) {
    String ret((char*)strattr, CopyString);
    xmlFree(strattr);
    return ret;
  } else {
    if (xmlStrEqual((xmlChar*)namespaceuri.data(),
                    (xmlChar*)DOM_XMLNS_NAMESPACE)) {
      nsptr = dom_get_nsdecl(elemp, (xmlChar*)localname.data());
      if (nsptr != nullptr) {
        return String((char*)nsptr->href, CopyString);
      }
    }
  }
  return empty_string();
}

Object HHVM_METHOD(DOMElement, getElementsByTagName,
                   const String& name) {
  auto* data = Native::data<DOMElement>(this_);
  return newDOMNodeList(data->doc(), Object{this_}, 0, name);
}

Object HHVM_METHOD(DOMElement, getElementsByTagNameNS,
                   const String& namespaceuri,
                   const String& localname) {
  auto* data = Native::data<DOMElement>(this_);
  return newDOMNodeList(data->doc(), Object{this_}, 0, localname,
                        namespaceuri);
}

bool HHVM_METHOD(DOMElement, hasAttribute,
                 const String& name) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  return dom_get_dom1_attribute(nodep, (xmlChar*)name.data()) != nullptr;
}

bool HHVM_METHOD(DOMElement, hasAttributeNS,
                 const String& namespaceuri,
                 const String& localname) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr elemp = data->nodep();
  if (!elemp) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlNs *nsp;
  xmlChar *value = xmlGetNsProp(elemp, (xmlChar*)localname.data(),
                                (xmlChar*)namespaceuri.data());
  if (value != nullptr) {
    xmlFree(value);
    return true;
  } else {
    if (xmlStrEqual((xmlChar*)namespaceuri.data(),
                    (xmlChar*)DOM_XMLNS_NAMESPACE)) {
      nsp = dom_get_nsdecl(elemp, (xmlChar*)localname.data());
      if (nsp != nullptr) {
        return true;
      }
    }
  }
  return false;
}

bool HHVM_METHOD(DOMElement, removeAttribute,
                 const String& name) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlNodePtr attrp;
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  attrp = dom_get_dom1_attribute(nodep, (xmlChar*)name.data());
  if (attrp == nullptr) {
    return false;
  }
  switch (attrp->type) {
  case XML_ATTRIBUTE_NODE:
    libxml_register_node(attrp)->unlink(); // release attrp if unused
    break;
  case XML_NAMESPACE_DECL:
    return false;
  default:
    break;
  }
  return true;
}

Variant HHVM_METHOD(DOMElement, removeAttributeNode,
                    const Object& oldattr) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  auto attr = getDOMNode(oldattr);
  xmlAttrPtr attrp = (xmlAttrPtr)attr->nodep();
  if (!attrp) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  if (attrp->type != XML_ATTRIBUTE_NODE || attrp->parent != nodep) {
    php_dom_throw_error(NOT_FOUND_ERR, data->doc()->m_stricterror);
    return false;
  }
  auto attrNode = libxml_register_node((xmlNodePtr)attrp);
  xmlUnlinkNode((xmlNodePtr)attrp);
  return php_dom_create_object((xmlNodePtr)attrp, data->doc());
}

Variant HHVM_METHOD(DOMElement, removeAttributeNS,
                    const String& namespaceuri,
                    const String& localname) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlAttr *attrp;
  xmlNsPtr nsptr;
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return init_null();
  }
  attrp = xmlHasNsProp(nodep, (xmlChar*)localname.data(),
                       (xmlChar*)namespaceuri.data());
  nsptr = dom_get_nsdecl(nodep, (xmlChar*)localname.data());
  if (nsptr != nullptr) {
    if (xmlStrEqual((xmlChar*)namespaceuri.data(), nsptr->href)) {
      if (nsptr->href != nullptr) {
        xmlFree((char*)nsptr->href);
        nsptr->href = nullptr;
      }
      if (nsptr->prefix != nullptr) {
        xmlFree((char*)nsptr->prefix);
        nsptr->prefix = nullptr;
      }
    } else {
      return init_null();
    }
  }
  if (attrp && attrp->type != XML_ATTRIBUTE_DECL) {
    // release attrp if unused
    libxml_register_node((xmlNodePtr)attrp)->unlink();
  }
  return init_null();
}

Variant HHVM_METHOD(DOMElement, setAttribute,
                    const String& name,
                    const String& value) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlNodePtr attr = nullptr;
  int name_valid;
  if (name.empty()) {
    raise_warning("Attribute Name is required");
    return false;
  }
  name_valid = xmlValidateName((xmlChar*)name.data(), 0);
  if (name_valid != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, 1);
    return false;
  }
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  attr = dom_get_dom1_attribute(nodep, (xmlChar*)name.data());
  if (attr != nullptr) {
    switch (attr->type) {
    case XML_ATTRIBUTE_NODE:
      break;
    case XML_NAMESPACE_DECL:
      return false;
    default:
      break;
    }
  }
  if (xmlStrEqual((xmlChar*)name.data(), (xmlChar*)"xmlns")) {
    if (xmlNewNs(nodep, (xmlChar*)value.data(), nullptr)) {
      return true;
    }
  } else {
    attr = (xmlNodePtr)xmlSetProp(nodep, (xmlChar*)name.data(),
                                  (xmlChar*)value.data());
  }
  if (!attr) {
    raise_warning("No such attribute '%s'", name.data());
    return false;
  }
  return php_dom_create_object(attr, data->doc());
}

Variant HHVM_METHOD(DOMElement, setAttributeNode,
                    const Object& newattr) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  auto domattr = getDOMNode(newattr);
  xmlAttrPtr attrp = (xmlAttrPtr)domattr->nodep();
  if (!attrp) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlAttr *existattrp = nullptr;
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  if (attrp->type != XML_ATTRIBUTE_NODE) {
    raise_warning("Attribute node is required");
    return false;
  }
  if (!(attrp->doc == nullptr || attrp->doc == nodep->doc)) {
    php_dom_throw_error(WRONG_DOCUMENT_ERR, data->doc()->m_stricterror);
    return false;
  }
  existattrp = xmlHasProp(nodep, attrp->name);
  if (existattrp != nullptr && existattrp->type != XML_ATTRIBUTE_DECL) {
    xmlUnlinkNode((xmlNodePtr)existattrp);
  }
  if (attrp->parent != nullptr) {
    xmlUnlinkNode((xmlNodePtr)attrp);
  }
  xmlAddChild(nodep, (xmlNodePtr)attrp);
  /* Returns old property if removed otherwise null */
  if (existattrp != nullptr) {
    return php_dom_create_object((xmlNodePtr)existattrp, data->doc());
  }
  return init_null();
}

Variant HHVM_METHOD(DOMElement, setAttributeNodeNS,
                    const Object& newattr) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNs *nsp;
  xmlAttr *existattrp = nullptr;
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  auto domattr = getDOMNode(newattr);
  xmlAttrPtr attrp = (xmlAttrPtr)domattr->nodep();
  if (!attrp) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  if (attrp->type != XML_ATTRIBUTE_NODE) {
    raise_warning("Attribute node is required");
    return false;
  }
  if (!(attrp->doc == nullptr || attrp->doc == nodep->doc)) {
    php_dom_throw_error(WRONG_DOCUMENT_ERR, data->doc()->m_stricterror);
    return false;
  }
  nsp = attrp->ns;
  if (nsp != nullptr) {
    existattrp = xmlHasNsProp(nodep, nsp->href, attrp->name);
  } else {
    existattrp = xmlHasProp(nodep, attrp->name);
  }
  if (existattrp != nullptr && existattrp->type != XML_ATTRIBUTE_DECL) {
    xmlUnlinkNode((xmlNodePtr)existattrp);
  }
  if (attrp->parent != nullptr) {
    xmlUnlinkNode((xmlNodePtr)attrp);
  }
  xmlAddChild(nodep, (xmlNodePtr) attrp);
  /* Returns old property if removed otherwise null */
  if (existattrp != nullptr) {
    return php_dom_create_object((xmlNodePtr)existattrp, data->doc());
  }
  return init_null();
}

Variant HHVM_METHOD(DOMElement, setAttributeNS,
                    const String& namespaceuri,
                    const String& name,
                    const String& value) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr elemp = data->nodep();
  if (!elemp) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return false;
  }
  xmlNsPtr nsptr;
  char *localname = nullptr, *prefix = nullptr;
  int errorcode = 0, is_xmlns = 0, name_valid;
  if (name.empty()) {
    raise_warning("Attribute Name is required");
    return false;
  }
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(elemp)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return init_null();
  }
  errorcode = dom_check_qname((char*)name.data(), &localname, &prefix,
                              namespaceuri.size(), name.size());
  if (errorcode == 0) {
    if (namespaceuri.size() > 0) {
      if ((xmlStrEqual((xmlChar *) prefix, (xmlChar *)"xmlns") ||
          (prefix == nullptr &&
          xmlStrEqual((xmlChar *) localname, (xmlChar *)"xmlns"))) &&
          xmlStrEqual((xmlChar *) namespaceuri.data(),
                      (xmlChar *)DOM_XMLNS_NAMESPACE)) {
        is_xmlns = 1;
        if (prefix == nullptr) {
          nsptr = dom_get_nsdecl(elemp, nullptr);
        } else {
          nsptr = dom_get_nsdecl(elemp, (xmlChar *)localname);
        }
      } else {
        nsptr = xmlSearchNsByHref(elemp->doc, elemp,
                                  (xmlChar*)namespaceuri.data());
        if (nsptr && nsptr->prefix == nullptr) {
          xmlNsPtr tmpnsptr;
          tmpnsptr = nsptr->next;
          while (tmpnsptr) {
            if ((tmpnsptr->prefix != nullptr) && (tmpnsptr->href != nullptr) &&
              (xmlStrEqual(tmpnsptr->href, (xmlChar*)namespaceuri.data()))) {
              nsptr = tmpnsptr;
              break;
            }
            tmpnsptr = tmpnsptr->next;
          }
          if (tmpnsptr == nullptr) {
            nsptr = _dom_new_reconNs(elemp->doc, elemp, nsptr);
          }
        }
      }
      if (nsptr == nullptr) {
        if (prefix == nullptr) {
          if (is_xmlns == 1) {
            xmlNewNs(elemp, (xmlChar *)value.data(), nullptr);
            xmlReconciliateNs(elemp->doc, elemp);
          } else {
            errorcode = NAMESPACE_ERR;
          }
        } else {
          if (is_xmlns == 1) {
            xmlNewNs(elemp, (xmlChar*)value.data(), (xmlChar*)localname);
          } else {
            nsptr = dom_get_ns(elemp, (char*)namespaceuri.data(),
                               &errorcode, prefix);
          }
          xmlReconciliateNs(elemp->doc, elemp);
        }
      } else {
        if (is_xmlns == 1) {
          if (nsptr->href) {
            xmlFree((xmlChar*)nsptr->href);
          }
          nsptr->href = xmlStrdup((xmlChar*)value.data());
        }
      }

      if (errorcode == 0 && is_xmlns == 0) {
        xmlSetNsProp(elemp, nsptr, (xmlChar*)localname,
                     (xmlChar*)value.data());
      }
    } else {
      name_valid = xmlValidateName((xmlChar*)localname, 0);
      if (name_valid != 0) {
        errorcode = INVALID_CHARACTER_ERR;
        stricterror = 1;
      } else {
        xmlSetProp(elemp, (xmlChar*)localname, (xmlChar*)value.data());
      }
    }
  }
  xmlFree(localname);
  if (prefix != nullptr) {
    xmlFree(prefix);
  }
  if (errorcode != 0) {
    php_dom_throw_error((dom_exception_code)errorcode, stricterror);
  }
  return init_null();
}

Variant HHVM_METHOD(DOMElement, setIDAttribute,
                    const String& name,
                    bool isid) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return init_null();
  }
  xmlAttrPtr attrp;
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return init_null();
  }
  attrp = xmlHasNsProp(nodep, (xmlChar*)name.data(), nullptr);
  if (attrp == nullptr || attrp->type == XML_ATTRIBUTE_DECL) {
    php_dom_throw_error(NOT_FOUND_ERR, data->doc()->m_stricterror);
  } else {
    php_set_attribute_id(attrp, isid);
  }
  return init_null();
}

Variant HHVM_METHOD(DOMElement, setIDAttributeNode,
                    const Object& idattr,
                    bool isid) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr nodep = data->nodep();
  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return init_null();
  }
  auto domattr = getDOMNode(idattr);
  xmlAttrPtr attrp = (xmlAttrPtr)domattr->nodep();
  if (!attrp) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return init_null();
  }
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return init_null();
  }
  if (attrp->parent != nodep) {
    php_dom_throw_error(NOT_FOUND_ERR, data->doc()->m_stricterror);
  } else {
    php_set_attribute_id(attrp, isid);
  }
  return init_null();
}

Variant HHVM_METHOD(DOMElement, setIDAttributeNS,
                    const String& namespaceuri,
                    const String& localname,
                    bool isid) {
  auto* data = Native::data<DOMElement>(this_);
  xmlNodePtr elemp = data->nodep();
  if (!elemp) {
    php_dom_throw_error(INVALID_STATE_ERR, 0);
    return init_null();
  }
  xmlAttrPtr attrp;
  bool stricterror = data->doc() ? data->doc()->m_stricterror : true;
  if (dom_node_is_read_only(elemp)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return init_null();
  }
  attrp = xmlHasNsProp(elemp, (xmlChar*)localname.data(),
                       (xmlChar*)namespaceuri.data());
  if (attrp == nullptr || attrp->type == XML_ATTRIBUTE_DECL) {
    php_dom_throw_error(NOT_FOUND_ERR, data->doc()->m_stricterror);
  } else {
    php_set_attribute_id(attrp, isid);
  }
  return init_null();
}

///////////////////////////////////////////////////////////////////////////////

#define CHECK_ENTITY(nodep)                                                    \
  auto domentity = getDOMNode(obj);                                            \
  xmlEntity *nodep = (xmlEntity*)(domentity ? domentity->nodep()               \
                                            : nullptr);                        \
  if (nodep == nullptr) {                                                      \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                                 \
    return init_null();                                                        \
  }                                                                            \

static Variant dom_entity_public_id_read(const Object& obj) {
  CHECK_ENTITY(nodep);
  if (nodep->etype != XML_EXTERNAL_GENERAL_UNPARSED_ENTITY) {
    return init_null();
  }
  return String((char *)(nodep->ExternalID), CopyString);
}

static Variant dom_entity_system_id_read(const Object& obj) {
  CHECK_ENTITY(nodep);
  if (nodep->etype != XML_EXTERNAL_GENERAL_UNPARSED_ENTITY) {
    return init_null();
  }
  return String((char *)(nodep->SystemID), CopyString);
}

static Variant dom_entity_notation_name_read(const Object& obj) {
  CHECK_ENTITY(nodep);
  if (nodep->etype != XML_EXTERNAL_GENERAL_UNPARSED_ENTITY) {
    return init_null();
  }
  char *content = (char*)xmlNodeGetContent((xmlNodePtr) nodep);
  String ret(content, CopyString);
  xmlFree(content);
  return ret;
}

static Variant dom_entity_actual_encoding_read(const Object& /*obj*/) {
  return init_null();
}

static void dom_entity_actual_encoding_write(const Object& /*obj*/,
                                             const Variant& /*value*/) {
  // do nothing
}

static Variant dom_entity_encoding_read(const Object& /*obj*/) {
  return init_null();
}

static void
dom_entity_encoding_write(const Object& /*obj*/, const Variant& /*value*/) {
  // do nothing
}

static Variant dom_entity_version_read(const Object& /*obj*/) {
  return init_null();
}

static void
dom_entity_version_write(const Object& /*obj*/, const Variant& /*value*/) {
  // do nothing
}

static DOMPropertyAccessor domentity_properties[] = {
 { "publicId",       dom_entity_public_id_read,       nullptr },
 { "systemId",       dom_entity_system_id_read,       nullptr },
 { "notationName",   dom_entity_notation_name_read,   nullptr },
 { "actualEncoding", dom_entity_actual_encoding_read,
   dom_entity_actual_encoding_write },
 { "encoding",       dom_entity_encoding_read,
   dom_entity_encoding_write },
 { "version",        dom_entity_version_read,
   dom_entity_version_write },
 { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domentity_properties_map
((DOMPropertyAccessor*)domentity_properties, &domnode_properties_map);

struct DOMEntityPropHandler : public DOMPropHandler<DOMEntityPropHandler> {
  static constexpr DOMPropertyAccessorMap& map = domentity_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

Array HHVM_METHOD(DOMEntity, __debugInfo) {
  auto data = Native::data<DOMNode>(this_);
  if (!data->node()) {
    return this_->toArray();
  }
  return domentity_properties_map.debugInfo(Object{this_});
}

///////////////////////////////////////////////////////////////////////////////


void HHVM_METHOD(DOMEntityReference, __construct,
                 const String& name) {
  auto data = Native::data<DOMNode>(this_);
  int name_valid = xmlValidateName((xmlChar *)name.data(), 0);
  if (name_valid != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, 1);
    return;
  }

  data->setNode(xmlNewReference(nullptr, (xmlChar*)name.data()));
  if (!data->node()) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

///////////////////////////////////////////////////////////////////////////////

#define CHECK_NOTATION(nodep)                                                  \
  auto domnotation = getDOMNode(obj);                                          \
  xmlEntity *nodep = (xmlEntity*)(domnotation ? domnotation->nodep()           \
                                              : nullptr);                      \
  if (nodep == nullptr) {                                                      \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                                 \
    return init_null();                                                        \
  }                                                                            \

static Variant dom_notation_public_id_read(const Object& obj) {
  CHECK_NOTATION(nodep);
  if (nodep->ExternalID) {
    return String((char *)(nodep->ExternalID), CopyString);
  }
  return empty_string_variant();
}

static Variant dom_notation_system_id_read(const Object& obj) {
  CHECK_NOTATION(nodep);
  if (nodep->SystemID) {
    return String((char *)(nodep->SystemID), CopyString);
  }
  return empty_string_variant();
}

static DOMPropertyAccessor domnotation_properties[] = {
 { "publicId",   dom_notation_public_id_read, nullptr },
 { "systemId",   dom_notation_system_id_read, nullptr },
 { "nodeName",   domnode_nodename_read,       nullptr },
 { "nodeValue",  domnode_nodevalue_read,      domnode_nodevalue_write },
 { "attributes", domnode_attributes_read,     nullptr },
 { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domnotation_properties_map
((DOMPropertyAccessor*)domnotation_properties);

struct DOMNotationPropHandler : public DOMPropHandler<DOMNotationPropHandler> {
  static constexpr DOMPropertyAccessorMap& map = domnotation_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

Array HHVM_METHOD(DOMNotation, __debugInfo) {
  auto* data = Native::data<DOMNode>(this_);
  if (!data->node()) {
    return this_->toArray();
  }
  return domnotation_properties_map.debugInfo(Object{this_});
}

///////////////////////////////////////////////////////////////////////////////

static Variant dom_processinginstruction_target_read(const Object& obj) {
  CHECK_NODE(nodep);
  return String((char *)(nodep->name), CopyString);
}

static Variant dom_processinginstruction_data_read(const Object& obj) {
  CHECK_NODE(nodep);
  xmlChar *content = xmlNodeGetContent(nodep);
  if (content) {
    String ret((char*)content, CopyString);
    xmlFree(content);
    return ret;
  }
  return empty_string_variant();
}

static void dom_processinginstruction_data_write(const Object& obj,
                                                 const Variant& value) {
  CHECK_WRITE_NODE(nodep);
  String svalue = value.toString();
  xmlNodeSetContentLen(nodep, (xmlChar*)svalue.data(), svalue.size() + 1);
}

static DOMPropertyAccessor domprocessinginstruction_properties[] = {
  { "target", dom_processinginstruction_target_read, nullptr },
  { "data",   dom_processinginstruction_data_read,
    dom_processinginstruction_data_write },
  { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domprocessinginstruction_properties_map
((DOMPropertyAccessor*)domprocessinginstruction_properties,
 &domnode_properties_map);

struct DOMProcessingInstructionPropHandler :
  public DOMPropHandler<DOMProcessingInstructionPropHandler> {
  static constexpr DOMPropertyAccessorMap& map =
    domprocessinginstruction_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

void HHVM_METHOD(DOMProcessingInstruction, __construct,
                 const String& name,
                 const Variant& value /*= null_string*/) {
  int name_valid = xmlValidateName((xmlChar *)name.data(), 0);
  if (name_valid != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, 1);
    return;
  }

  auto* data = Native::data<DOMNode>(this_);
  const String& str_value = value.isNull() ? null_string : value.toString();
  data->setNode(xmlNewPI((xmlChar *)name.data(), (xmlChar *)str_value.data()));
  if (!data->node()) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

Array HHVM_METHOD(DOMProcessingInstruction, __debugInfo) {
  auto* data = Native::data<DOMNode>(this_);
  if (!data->node()) {
    return this_->toArray();
  }
  return domprocessinginstruction_properties_map.debugInfo(Object{this_});
}

///////////////////////////////////////////////////////////////////////////////


static Variant dom_namednodemap_length_read(const Object& obj) {
  auto objmap = Native::data<DOMIterable>(obj);

  int count = 0;
  if (objmap->m_nodetype == XML_NOTATION_NODE ||
      objmap->m_nodetype == XML_ENTITY_NODE) {
    if (objmap->m_ht) {
      count = xmlHashSize(objmap->m_ht);
    }
  } else {
    xmlNodePtr nodep = objmap->getBaseNodeData()->nodep();
    if (nodep) {
      xmlAttrPtr curnode = nodep->properties;
      if (curnode) {
        count++;
        while (curnode->next != nullptr) {
          count++;
          curnode = curnode->next;
        }
      }
    }
  }
  return count;
}

static DOMPropertyAccessor domnamednodemap_properties[] = {
  { "length", dom_namednodemap_length_read, nullptr },
  { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domnamednodemap_properties_map
((DOMPropertyAccessor*)domnamednodemap_properties);

struct DOMNamedNodeMapPropHandler :
  public DOMPropHandler<DOMNamedNodeMapPropHandler> {
  static constexpr DOMPropertyAccessorMap& map = domnamednodemap_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_METHOD(DOMNamedNodeMap, getNamedItem,
                    const String& name) {
  auto* data = Native::data<DOMIterable>(this_);
  xmlNodePtr itemnode = nullptr;
  if (data->m_nodetype == XML_NOTATION_NODE
      || data->m_nodetype == XML_ENTITY_NODE) {
    if (data->m_ht) {
      if (data->m_nodetype == XML_ENTITY_NODE) {
        itemnode = (xmlNodePtr)xmlHashLookup(data->m_ht, (xmlChar*)name.data());
      } else {
        xmlNotation *notep =
          (xmlNotation *)xmlHashLookup(data->m_ht,
                                       (xmlChar*)name.data());
        if (notep) {
          itemnode = create_notation(notep->name, notep->PublicID,
                                     notep->SystemID);
        }
      }
    }
  } else {
    xmlNodePtr nodep = data->getBaseNodeData()->nodep();
    if (nodep) {
      itemnode = (xmlNodePtr)xmlHasProp(nodep, (xmlChar*)name.data());
    }
  }

  if (itemnode) {
    Variant ret = php_dom_create_object(itemnode, data->m_doc);
    if (ret.isNull()) {
      raise_warning("Cannot create required DOM object");
      return false;
    }
    return ret;
  }
  return init_null();
}

Variant HHVM_METHOD(DOMNamedNodeMap, getNamedItemNS,
                    const String& namespaceuri,
                    const String& localname) {
  auto data = Native::data<DOMIterable>(this_);
  xmlNodePtr itemnode = nullptr;
  if (data->m_nodetype == XML_NOTATION_NODE
      || data->m_nodetype == XML_ENTITY_NODE) {
    if (data->m_ht) {
      if (data->m_nodetype == XML_ENTITY_NODE) {
        itemnode = (xmlNodePtr)xmlHashLookup(data->m_ht,
                                             (xmlChar*)localname.data());
      } else {
        xmlNotation *notep =
          (xmlNotation *)xmlHashLookup(data->m_ht, (xmlChar*)localname.data());
        if (notep) {
          itemnode = create_notation(notep->name, notep->PublicID,
                                     notep->SystemID);
        }
      }
    }
  } else {
    xmlNodePtr nodep = data->getBaseNodeData()->nodep();
    if (nodep) {
      itemnode = (xmlNodePtr)xmlHasNsProp(nodep, (xmlChar*)localname.data(),
                                          (xmlChar*)namespaceuri.data());
    }
  }

  if (itemnode) {
    Variant ret = php_dom_create_object(itemnode, data->m_doc);
    if (ret.isNull()) {
      raise_warning("Cannot create required DOM object");
      return false;
    }
    return ret;
  }
  return init_null();
}

Variant HHVM_METHOD(DOMNamedNodeMap, item,
                    int64_t index) {
  auto* data = Native::data<DOMIterable>(this_);
  if (index >= 0) {
    xmlNodePtr itemnode = nullptr;
    if (data->m_nodetype == XML_NOTATION_NODE
        || data->m_nodetype == XML_ENTITY_NODE) {
      if (data->m_ht) {
        if (data->m_nodetype == XML_ENTITY_NODE) {
          itemnode = php_dom_libxml_hash_iter(data->m_ht, index);
        } else {
          itemnode = php_dom_libxml_notation_iter(data->m_ht, index);
        }
      }
    } else {
      xmlNodePtr nodep = data->getBaseNodeData()->nodep();
      if (nodep) {
        xmlNodePtr curnode = (xmlNodePtr)nodep->properties;
        int count = 0;
        while (count < index && curnode != nullptr) {
          count++;
          curnode = (xmlNodePtr)curnode->next;
        }
        itemnode = curnode;
      }
    }

    if (itemnode) {
      Variant ret = php_dom_create_object(itemnode, data->m_doc);
      if (ret.isNull()) {
        raise_warning("Cannot create required DOM object");
        return false;
      }
      return ret;
    }
  }

  return init_null();
}

Array HHVM_METHOD(DOMNamedNodeMap, __debugInfo) {
  return domnamednodemap_properties_map.debugInfo(Object{this_});
}

Object HHVM_METHOD(DOMNamedNodeMap, getIterator) {
  auto data = Native::data<DOMIterable>(this_);
  Object ret{DOMNodeIterator::classof()};
  DOMNodeIterator* iter = Native::data<DOMNodeIterator>(ret);
  iter->set_iterator(this_, data);
  iter->setKeyIsNamed();
  return ret;
}

///////////////////////////////////////////////////////////////////////////////


static Variant dom_nodelist_length_read(const Object& obj) {
  auto objmap = Native::data<DOMIterable>(obj);

  int count = 0;
  if (objmap->m_ht) {
    count = xmlHashSize(objmap->m_ht);
  } else {
    if (objmap->m_nodetype == DOM_NODESET) {
      count = objmap->m_baseobjptr.size();
    } else {
      xmlNodePtr nodep = objmap->getBaseNodeData()->nodep();
      if (nodep) {
        if (objmap->m_nodetype == XML_ATTRIBUTE_NODE ||
            objmap->m_nodetype == XML_ELEMENT_NODE) {
          xmlNodePtr curnode = nodep->children;
          if (curnode) {
            count++;
            while (curnode->next != nullptr) {
              count++;
              curnode = curnode->next;
            }
          }
        } else {
          if (nodep->type == XML_DOCUMENT_NODE ||
              nodep->type == XML_HTML_DOCUMENT_NODE) {
            nodep = xmlDocGetRootElement((xmlDoc *) nodep);
          } else {
            nodep = nodep->children;
          }
          dom_get_elements_by_tag_name_ns_raw
            (nodep, objmap->m_ns.data(), objmap->m_local.data(), &count, -1);
        }
      }
    }
  }

  return count;
}

static DOMPropertyAccessor domnodelist_properties[] = {
  { "length", dom_nodelist_length_read, nullptr },
  { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domnodelist_properties_map
((DOMPropertyAccessor*)domnodelist_properties);

struct DOMNodeListPropHandler : public DOMPropHandler<DOMNodeListPropHandler> {
  static constexpr DOMPropertyAccessorMap& map = domnodelist_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

Array HHVM_METHOD(DOMNodeList, __debugInfo) {
  return domnodelist_properties_map.debugInfo(Object{this_});
}

Variant HHVM_METHOD(DOMNodeList, item,
                    int64_t index) {
  auto data = Native::data<DOMIterable>(this_);
  xmlNodePtr itemnode = nullptr;
  xmlNodePtr nodep, curnode;
  int count = 0;
  if (index >= 0) {
    if (data->m_ht) {
      if (data->m_nodetype == XML_ENTITY_NODE) {
        itemnode = php_dom_libxml_hash_iter(data->m_ht, index);
      } else {
        itemnode = php_dom_libxml_notation_iter(data->m_ht, index);
      }
    } else {
      if (data->m_nodetype == DOM_NODESET) {
        if (data->m_baseobjptr.exists(index)) {
          return data->m_baseobjptr[index];
        }
      } else if (!data->m_baseobj.isNull()) {
        nodep = data->getBaseNodeData()->nodep();
        if (nodep) {
          if (data->m_nodetype == XML_ATTRIBUTE_NODE ||
              data->m_nodetype == XML_ELEMENT_NODE) {
            curnode = nodep->children;
            while (count < index && curnode != nullptr) {
              count++;
              curnode = curnode->next;
            }
            itemnode = curnode;
          } else {
            if (nodep->type == XML_DOCUMENT_NODE ||
                nodep->type == XML_HTML_DOCUMENT_NODE) {
              nodep = xmlDocGetRootElement((xmlDoc *) nodep);
            } else {
              nodep = nodep->children;
            }
            itemnode = dom_get_elements_by_tag_name_ns_raw(nodep,
                data->m_ns.data(), data->m_local.data(), &count, index);
          }
        }
      }
    }
    if (itemnode) {
      return create_node_object(itemnode, data->m_doc);
    }
  }
  return Object();
}

Object HHVM_METHOD(DOMNodeList, getIterator) {
  auto data = Native::data<DOMIterable>(this_);
  Object ret{DOMNodeIterator::classof()};
  DOMNodeIterator* iter = Native::data<DOMNodeIterator>(ret);
  iter->set_iterator(this_, data);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////


Variant HHVM_METHOD(DOMImplementation, createDocument,
                    const Variant& namespaceuri /* = null_string */,
                    const Variant& qualifiedname /* = null_string */,
                    const Variant& doctypeobj /* = null_object */) {
  xmlDoc *docp;
  xmlNode *nodep;
  xmlNsPtr nsptr = nullptr;
  int errorcode = 0;
  char *prefix = nullptr, *localname = nullptr;
  xmlDtdPtr doctype = nullptr;
  const String& str_namespaceuri = namespaceuri.isNull()
                                 ? null_string
                                 : namespaceuri.toString();
  const String& str_qualifiedname = qualifiedname.isNull()
                                  ? null_string
                                  : qualifiedname.toString();
  const Object& obj_doctypeobj = doctypeobj.isNull()
                               ? null_object
                               : doctypeobj.toObject();
  if (!obj_doctypeobj.isNull()) {
    auto *domdoctype = getDOMNode(obj_doctypeobj);
    doctype = (xmlDtdPtr)domdoctype->nodep();
    if (!doctype) {
      php_dom_throw_error(INVALID_STATE_ERR, 0);
      return false;
    }
    if (doctype->type == XML_DOCUMENT_TYPE_NODE) {
      raise_warning("Invalid DocumentType object");
      return false;
    }
    if (doctype->doc != nullptr) {
      php_dom_throw_error(WRONG_DOCUMENT_ERR, 1);
      return false;
    }
  }
  if (str_qualifiedname.size() > 0) {
    errorcode = dom_check_qname((char*)str_qualifiedname.data(),
                                (char**)&localname, (char**)&prefix, 1,
                                str_qualifiedname.size());
    if (errorcode == 0 && str_namespaceuri.size() > 0) {
      nsptr = xmlNewNs(nullptr, (xmlChar*)str_namespaceuri.data(),
                       (xmlChar*)prefix);
      if (nsptr == nullptr) {
        errorcode = NAMESPACE_ERR;
      }
    }
  }
  if (prefix != nullptr) {
    xmlFree(prefix);
  }
  if (errorcode != 0) {
    if (localname != nullptr) {
      xmlFree(localname);
    }
    php_dom_throw_error((dom_exception_code)errorcode, 1);
    return false;
  }
  /* currently letting libxml2 set the version string */
  docp = xmlNewDoc(nullptr);
  if (!docp) {
    if (localname != nullptr) {
      xmlFree(localname);
    }
    return false;
  }
  if (doctype != nullptr) {
    docp->intSubset = doctype;
    doctype->parent = docp;
    doctype->doc = docp;
    docp->children = (xmlNodePtr)doctype;
    docp->last = (xmlNodePtr)doctype;
  }
  if (localname != nullptr) {
    nodep = xmlNewDocNode(docp, nsptr, (xmlChar*)localname, nullptr);
    if (!nodep) {
      if (doctype != nullptr) {
        docp->intSubset = nullptr;
        doctype->parent = nullptr;
        doctype->doc = nullptr;
        docp->children = nullptr;
        docp->last = nullptr;
      }
      xmlFreeDoc(docp);
      xmlFree(localname);
      /* Need some type of error here */
      raise_warning("Unexpected Error");
      return false;
    }
    nodep->nsDef = nsptr;
    xmlDocSetRootElement(docp, nodep);
    xmlFree(localname);
  }
  Object ret{DOMDocument::classof()};
  auto doc_data = Native::data<DOMNode>(ret);
  doc_data->setNode((xmlNodePtr)docp);
  if (doctype) {
    libxml_register_node((xmlNodePtr)doctype)->setDoc(doc_data->doc());
  }
  return ret;
}

Variant HHVM_METHOD(DOMImplementation, createDocumentType,
                    const Variant& qualifiedname /* = null_string */,
                    const Variant& publicid /* = null_string */,
                    const Variant& systemid /* = null_string */) {
  xmlDtd *doctype;
  xmlChar *pch1 = nullptr, *pch2 = nullptr, *localname = nullptr;
  xmlURIPtr uri;
  const String& str_qualifiedname = qualifiedname.isNull()
                                  ? null_string
                                  : qualifiedname.toString();
  const String& str_publicid = publicid.isNull()
                             ? null_string
                             : publicid.toString();
  const String& str_systemid = systemid.isNull()
                             ? null_string
                             : systemid.toString();
  if (str_qualifiedname.empty()) {
    raise_warning("qualifiedname is required");
    return false;
  }
  if (str_publicid.size() > 0) {
    pch1 = (xmlChar*)str_publicid.data();
  }
  if (str_systemid.size() > 0) {
    pch2 = (xmlChar*)str_systemid.data();
  }
  uri = xmlParseURI((char*)str_qualifiedname.data());
  if (uri != nullptr && uri->opaque != nullptr) {
    localname = xmlStrdup((xmlChar*)uri->opaque);
    if (xmlStrchr(localname, (xmlChar)':') != nullptr) {
      php_dom_throw_error(NAMESPACE_ERR, 1);
      xmlFreeURI(uri);
      xmlFree(localname);
      return false;
    }
  } else {
    localname = xmlStrdup((xmlChar*)str_qualifiedname.data());
  }
  if (uri) {
    xmlFreeURI(uri);
  }
  doctype = xmlCreateIntSubset(nullptr, localname, pch1, pch2);
  xmlFree(localname);
  if (doctype == nullptr) {
    raise_warning("Unable to create DocumentType");
    return false;
  }
  return create_node_object((xmlNodePtr)doctype);
}

bool HHVM_METHOD(DOMImplementation, hasFeature,
                 const String& feature,
                 const String& version) {
  return dom_has_feature(feature.data(), version.data());
}

///////////////////////////////////////////////////////////////////////////////


static Variant dom_xpath_document_read(const Object& obj) {
  xmlDoc *docp = nullptr;
  DOMXPath* xpath = getDOMXPath(obj);
  xmlXPathContextPtr ctx = (xmlXPathContextPtr)xpath->m_node;
  if (ctx) {
    docp = (xmlDocPtr)ctx->doc;
  }
  auto doc_data = Native::data<DOMNode>(xpath->m_doc);
  // If document in the context is the same, return it.
  if ((xmlDocPtr)doc_data->nodep() == docp) {
    return xpath->m_doc;
  }
  // Otherwise, create a new doc.
  return create_node_object((xmlNodePtr)docp, xpath->m_doc);
}

static DOMPropertyAccessor domxpath_properties[] = {
  { "document", dom_xpath_document_read, nullptr },
  { nullptr, nullptr, nullptr}
};
static DOMPropertyAccessorMap domxpath_properties_map
((DOMPropertyAccessor*)domxpath_properties);

struct DOMXPathPropHandler : public DOMPropHandler<DOMXPathPropHandler> {
  static constexpr DOMPropertyAccessorMap& map = domxpath_properties_map;
};

///////////////////////////////////////////////////////////////////////////////

static void dom_xpath_ext_function_php(xmlXPathParserContextPtr ctxt,
                                       int nargs, int type) {
  int error = 0;
  DOMXPath* intern = (DOMXPath*) ctxt->context->userData;
  if (intern == nullptr) {
    xmlGenericError(xmlGenericErrorContext,
                    "xmlExtFunctionTest: failed to get the internal object\n");
    error = 1;
  } else if (intern->m_registerPhpFunctions == 0) {
    xmlGenericError(xmlGenericErrorContext,
                    "xmlExtFunctionTest: PHP Object did not register "
                    "PHP functions\n");
    error = 1;
  }

  xmlXPathObjectPtr obj;
  if (error == 1) {
    for (int i = nargs - 1; i >= 0; i--) {
      obj = valuePop(ctxt);
      xmlXPathFreeObject(obj);
    }
    return;
  }

  Array args_vec = Array::CreateVec();
  for (int i = nargs - 2; i >= 0; i--) {
    Variant arg;
    obj = valuePop(ctxt);
    switch (obj->type) {
    case XPATH_STRING:
      arg = String((char *)obj->stringval, CopyString);
      break;
    case XPATH_BOOLEAN:
      arg = (bool)obj->boolval;
      break;
    case XPATH_NUMBER:
      arg = (double)obj->floatval;
      break;
    case XPATH_NODESET:
      if (type == 1) {
        char *str = (char *)xmlXPathCastToString(obj);
        arg = String(str, CopyString);
        xmlFree(str);
      } else if (type == 2) {
        Array argArr = Array::CreateVec();
        if (obj->nodesetval && obj->nodesetval->nodeNr > 0) {
          for (int j = 0; j < obj->nodesetval->nodeNr; j++) {
            xmlNodePtr node = obj->nodesetval->nodeTab[j];
            /* not sure, if we need this... it's copied from xpath.c */
            if (node->type == XML_NAMESPACE_DECL) {
              // xmlNodePtr nsparent = (xmlNodePtr)node->_private;
              xmlNodePtr nsparent = nullptr;
              xmlNsPtr curns = xmlNewNs(nullptr, node->name, nullptr);
              if (node->children) {
                curns->prefix = xmlStrdup((xmlChar *)node->children);
              }
              if (node->children) {
                node = xmlNewDocNode(node->doc, nullptr,
                                     (xmlChar *)node->children, node->name);
              } else {
                node = xmlNewDocNode(node->doc, nullptr,
                                     (xmlChar *)"xmlns", node->name);
              }
              node->type = XML_NAMESPACE_DECL;
              node->parent = nsparent;
              node->ns = curns;
            }
            argArr.append(create_node_object(node, intern->m_doc));
          }
        }
        arg = Variant(argArr);
      }
      break;
    default:
      arg = String((char *)xmlXPathCastToString(obj), CopyString);
    }
    xmlXPathFreeObject(obj);
    args_vec.append(arg);
  }

  /* Reverse order to pop values off ctxt stack */
  Array args;
  for (auto i = args_vec.size(); i > 0; i--) {
    args.append(args_vec.lookup(safe_cast<int64_t>(i - 1)));
  }

  obj = valuePop(ctxt);
  if (obj->stringval == nullptr) {
    xmlXPathFreeObject(obj);
    raise_warning("Handler name must be a string");
    return;
  }
  String handler((char*)obj->stringval, CopyString);
  xmlXPathFreeObject(obj);

  if (!is_callable(handler)) {
    raise_warning("Unable to call handler %s()", handler.data());
  } else if (intern->m_registerPhpFunctions == 2 &&
             !intern->m_registered_phpfunctions.exists(handler)) {
    /* Push an empty string, so that we at least have an xslt result... */
    valuePush(ctxt, xmlXPathNewString((xmlChar *)""));
    raise_warning("Not allowed to call handler '%s()'.", handler.data());
  } else {
    Variant retval = vm_call_user_func(handler, args);
    if (retval.isObject() &&
        retval.getObjectData()->instanceof(DOMNode::classof())) {
      if (intern->m_node_list.empty()) {
        intern->m_node_list = Array::CreateVec();
      }
      intern->m_node_list.append(retval);
      auto* node_data = Native::data<DOMNode>(retval.toObject());
      xmlNode *nodep = node_data->nodep();
      if (nodep) valuePush(ctxt, xmlXPathNewNodeSet(nodep));
    } else if (retval.is(KindOfBoolean)) {
      valuePush(ctxt, xmlXPathNewBoolean(retval.toBoolean()));
    } else if (retval.isObject()) {
      valuePush(ctxt, xmlXPathNewString((xmlChar *)""));
      raise_warning("A PHP Object cannot be converted to an XPath-string");
    } else {
      String sretval = retval.toString();
      valuePush(ctxt, xmlXPathNewString((xmlChar*)sretval.data()));
    }
  }
}

static void dom_xpath_ext_function_string_php(xmlXPathParserContextPtr ctxt,
                                              int nargs) {
  dom_xpath_ext_function_php(ctxt, nargs, 1);
}

static void dom_xpath_ext_function_object_php(xmlXPathParserContextPtr ctxt,
                                              int nargs) {
  dom_xpath_ext_function_php(ctxt, nargs, 2);
}

void DOMXPath::sweep() {
  if (m_node) {
    xmlXPathFreeContext((xmlXPathContextPtr)m_node);
    m_node = nullptr;
  }
}

void HHVM_METHOD(DOMXPath, __construct,
                 const Variant& doc) {
  auto* data = Native::data<DOMXPath>(this_);
  data->m_doc = doc.toObject();
  if (!data->m_doc->instanceof(DOMNode::classof())) {
    SystemLib::throwExceptionObject(String("DOMXPath::__construct expects "
                                           "parameter 1 to be DOMNode"));
    return;
  }
  auto doc_data = Native::data<DOMNode>(data->m_doc);
  xmlDocPtr docp = (xmlDocPtr)doc_data->nodep();
  xmlXPathContextPtr ctx = xmlXPathNewContext(docp);
  if (ctx == nullptr) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
    return;
  }

  xmlXPathRegisterFuncNS(ctx, (const xmlChar *) "functionString",
                         (const xmlChar *) "http://php.net/xpath",
                         dom_xpath_ext_function_string_php);
  xmlXPathRegisterFuncNS(ctx, (const xmlChar *) "function",
                         (const xmlChar *) "http://php.net/xpath",
                         dom_xpath_ext_function_object_php);
  data->m_node = ctx;
  ctx->userData = data;
}

Array HHVM_METHOD(DOMXPath, __debugInfo) {
  auto* data = Native::data<DOMXPath>(this_);
  if (!data->m_node) {
    return this_->toArray();
  }
  return domxpath_properties_map.debugInfo(Object{this_});
}

Variant HHVM_METHOD(DOMXPath, evaluate,
                    const String& expr,
                    const Variant& context /* = null_object */,
                    bool registerNodeNS /* = true */) {
  auto* data = Native::data<DOMXPath>(this_);
  const Object& obj_context = context.isNull()
                            ? null_object
                            : context.toObject();
  VMRegGuard _;
  return php_xpath_eval(data, expr, obj_context, PHP_DOM_XPATH_EVALUATE,
                        registerNodeNS);
}

Variant HHVM_METHOD(DOMXPath, query,
                    const String& expr,
                    const Variant& context /* = null_object */,
                    bool registerNodeNS /* = true */) {
  auto* data = Native::data<DOMXPath>(this_);
  const Object& obj_context = context.isNull()
                            ? null_object
                            : context.toObject();
  VMRegGuard _;
  return php_xpath_eval(data, expr, obj_context, PHP_DOM_XPATH_QUERY,
                        registerNodeNS);
}

bool HHVM_METHOD(DOMXPath, registerNamespace,
                 const String& prefix,
                 const String& uri) {
  auto* data = Native::data<DOMXPath>(this_);
  xmlXPathContextPtr ctxp = (xmlXPathContextPtr)data->m_node;
  if (ctxp == nullptr) {
    raise_warning("Invalid XPath Context");
    return false;
  }
  return xmlXPathRegisterNs(ctxp, (xmlChar*)prefix.data(),
                            (xmlChar*)uri.data()) == 0;
}

Variant HHVM_METHOD(DOMXPath, registerPHPFunctions,
                    const Variant& funcs /* = null */) {
  auto* data = Native::data<DOMXPath>(this_);
  if (funcs.isArray()) {
    Array arr = funcs.toArray();
    for (ArrayIter iter(arr); iter; ++iter) {
      data->m_registered_phpfunctions.set(iter.second(), "1");
    }
    data->m_registerPhpFunctions = 2;
    return true;
  }
  if (funcs.isString()) {
    data->m_registered_phpfunctions.set(funcs, "1");
    data->m_registerPhpFunctions = 2;
  } else {
    data->m_registerPhpFunctions = 1;
  }
  return init_null();
}

///////////////////////////////////////////////////////////////////////////////

void DOMNodeIterator::reset_iterator() {
  assertx(m_objmap);
  xmlNodePtr curnode = nullptr;
  if (m_objmap->m_nodetype != XML_ENTITY_NODE &&
      m_objmap->m_nodetype != XML_NOTATION_NODE) {
    m_index = -1;
    if (m_objmap->m_nodetype == DOM_NODESET) {
      m_iter = ArrayIter(m_objmap->m_baseobjptr);
    } else {
      xmlNodePtr nodep = m_objmap->getBaseNodeData()->nodep();
      if (!nodep) {
        goto err;
      }
      if (m_objmap->m_nodetype == XML_ATTRIBUTE_NODE ||
          m_objmap->m_nodetype == XML_ELEMENT_NODE) {
        if (m_objmap->m_nodetype == XML_ATTRIBUTE_NODE) {
          curnode = (xmlNodePtr)nodep->properties;
        } else {
          curnode = (xmlNodePtr)nodep->children;
        }
      } else {
        if (nodep->type == XML_DOCUMENT_NODE ||
            nodep->type == XML_HTML_DOCUMENT_NODE) {
          nodep = xmlDocGetRootElement((xmlDoc *) nodep);
        } else {
          nodep = nodep->children;
        }
        m_index = 0;
        int previndex = 0;
        curnode = dom_get_elements_by_tag_name_ns_raw
          (nodep, m_objmap->m_ns.data(), m_objmap->m_local.data(),
           &previndex, m_index);
      }
    }
    ++m_index;
  } else {
    if (m_objmap->m_nodetype == XML_ENTITY_NODE) {
      curnode = php_dom_libxml_hash_iter(m_objmap->m_ht, 0);
    } else {
      curnode = php_dom_libxml_notation_iter(m_objmap->m_ht, 0);
    }
    m_index = 1;
  }
 err:
  if (curnode) {
    auto doc = m_objmap->getBaseNodeData()->doc();
    m_curobj = create_node_object(curnode, doc).toObject();
  } else {
    m_curobj.reset();
  }
}

void DOMNodeIterator::set_iterator(ObjectData* o, DOMIterable *objmap) {
  m_o.reset(o);
  m_objmap = objmap;
  reset_iterator();
}

Variant HHVM_METHOD(DOMNodeIterator, current) {
  auto* data = Native::data<DOMNodeIterator>(this_);
  if (data->m_iter) {
    return data->m_iter.second();
  }
  return data->m_curobj;
}

Variant HHVM_METHOD(DOMNodeIterator, key) {
  auto* data = Native::data<DOMNodeIterator>(this_);
  if (data->m_iter) {
    return data->m_iter.first();
  }
  if (data->m_keyIsNamed) {
    DOMNode* node_data = Native::data<DOMNode>(data->m_curobj);
    xmlNodePtr curnode = node_data->nodep();
    if (!curnode) {
      php_dom_throw_error(INVALID_STATE_ERR, 0);
      return false;
    }
    return String((const char *)curnode->name, CopyString);
  }
  return data->m_index;
}

void HHVM_METHOD(DOMNodeIterator, next) {
  auto* data = Native::data<DOMNodeIterator>(this_);
  if (data->m_iter) {
    data->m_iter.next();
    return;
  }

  XMLNode curnode = nullptr;
  if (data->m_objmap->m_nodetype != XML_ENTITY_NODE &&
      data->m_objmap->m_nodetype != XML_NOTATION_NODE) {
    DOMNode* node_data = Native::data<DOMNode>(data->m_curobj);
    curnode = node_data->node();
    if (data->m_objmap->m_nodetype == XML_ATTRIBUTE_NODE ||
        data->m_objmap->m_nodetype == XML_ELEMENT_NODE) {
      if (!curnode->nodep()) {
        php_dom_throw_error(INVALID_STATE_ERR, 0);
        return;
      }
      curnode = libxml_register_node(curnode->nodep()->next);
    } else {
      /* Nav the tree evey time as this is LIVE */
      xmlNodePtr basenode =
        data->m_objmap->getBaseNodeData()->nodep();
      if (!basenode) {
        php_dom_throw_error(INVALID_STATE_ERR, 0);
        return;
      }
      if (basenode && (basenode->type == XML_DOCUMENT_NODE ||
                       basenode->type == XML_HTML_DOCUMENT_NODE)) {
        basenode = xmlDocGetRootElement((xmlDoc *) basenode);
      } else if (basenode) {
        basenode = basenode->children;
      } else {
        goto err;
      }
      int previndex = 0;

      curnode = libxml_register_node(dom_get_elements_by_tag_name_ns_raw
        (basenode, data->m_objmap->m_ns.data(), data->m_objmap->m_local.data(),
         &previndex, data->m_index));
    }
    ++data->m_index;
  } else {
    if (data->m_objmap->m_nodetype == XML_ENTITY_NODE) {
      curnode =
        libxml_register_node(php_dom_libxml_hash_iter(data->m_objmap->m_ht,
                                                      data->m_index));
    } else {
      curnode =
        libxml_register_node(php_dom_libxml_notation_iter(data->m_objmap->m_ht,
                                                          data->m_index));
    }
    ++data->m_index;
  }
err:
  if (curnode && curnode->nodep()) {
    auto doc = data->m_objmap->getBaseNodeData()->doc();
    data->m_curobj = create_node_object(curnode->nodep(), doc).toObject();
  } else {
    data->m_curobj.reset();
  }
  return;
}

void HHVM_METHOD(DOMNodeIterator, rewind) {
  auto* data = Native::data<DOMNodeIterator>(this_);
  data->m_iter.reset();
  data->m_index = -1;
  data->reset_iterator();
}

bool HHVM_METHOD(DOMNodeIterator, valid) {
  auto* data = Native::data<DOMNodeIterator>(this_);
  if (data->m_iter) {
    return !data->m_iter.end();
  }
  return !data->m_curobj.isNull();
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(dom_import_simplexml,
                      const Object& node) {
  xmlNodePtr nodep = SimpleXMLElement_exportNode(node);

  if (nodep && (nodep->type == XML_ELEMENT_NODE ||
                nodep->type == XML_ATTRIBUTE_NODE)) {
    return create_node_object(nodep);
  } else {
    raise_warning("Invalid Nodetype to import");
    return init_null();
  }
}

///////////////////////////////////////////////////////////////////////////////

struct DOMDocumentExtension final : Extension {
  DOMDocumentExtension() : Extension("domdocument", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void moduleRegisterNative() override {
    HHVM_ME(DOMNode, appendChild);
    HHVM_ME(DOMNode, cloneNode);
    HHVM_ME(DOMNode, getLineNo);
    HHVM_ME(DOMNode, hasAttributes);
    HHVM_ME(DOMNode, hasChildNodes);
    HHVM_ME(DOMNode, insertBefore);
    HHVM_ME(DOMNode, isDefaultNamespace);
    HHVM_ME(DOMNode, isSameNode);
    HHVM_ME(DOMNode, isSupported);
    HHVM_ME(DOMNode, lookupNamespaceUri);
    HHVM_ME(DOMNode, lookupPrefix);
    HHVM_ME(DOMNode, normalize);
    HHVM_ME(DOMNode, removeChild);
    HHVM_ME(DOMNode, replaceChild);
    HHVM_ME(DOMNode, C14N);
    HHVM_ME(DOMNode, C14Nfile);
    HHVM_ME(DOMNode, getNodePath);
    HHVM_ME(DOMNode, __debugInfo);
    Native::registerNativeDataInfo<DOMNode>(Native::NDIFlags::NO_SWEEP);
    Native::registerNativePropHandler<DOMNodePropHandler>(DOMNode::className());

    HHVM_ME(DOMAttr, __construct);
    HHVM_ME(DOMAttr, isId);
    HHVM_ME(DOMAttr, __debugInfo);
    Native::registerNativePropHandler<DOMAttrPropHandler>(DOMAttr::className());

    HHVM_ME(DOMCharacterData, appendData);
    HHVM_ME(DOMCharacterData, deleteData);
    HHVM_ME(DOMCharacterData, insertData);
    HHVM_ME(DOMCharacterData, replaceData);
    HHVM_ME(DOMCharacterData, substringData);
    HHVM_ME(DOMCharacterData, __debugInfo);
    Native::registerNativePropHandler<DOMCharacterDataPropHandler>(
      DOMCharacterData::className());

    HHVM_ME(DOMComment, __construct);

    HHVM_ME(DOMText, __construct);
    HHVM_ME(DOMText, isWhitespaceInElementContent);
    HHVM_ME(DOMText, isElementContentWhitespace);
    HHVM_ME(DOMText, splitText);
    HHVM_ME(DOMText, __debugInfo);
    Native::registerNativePropHandler<DOMTextPropHandler>(DOMText::className());

    HHVM_ME(DOMCdataSection, __construct);

    HHVM_ME(DOMDocument, __construct);
    HHVM_ME(DOMDocument, createAttribute);
    HHVM_ME(DOMDocument, createAttributeNS);
    HHVM_ME(DOMDocument, createCDATASection);
    HHVM_ME(DOMDocument, createComment);
    HHVM_ME(DOMDocument, createDocumentFragment);
    HHVM_ME(DOMDocument, createElement);
    HHVM_ME(DOMDocument, createElementNS);
    HHVM_ME(DOMDocument, createEntityReference);
    HHVM_ME(DOMDocument, createProcessingInstruction);
    HHVM_ME(DOMDocument, createTextNode);
    HHVM_ME(DOMDocument, getElementById);
    HHVM_ME(DOMDocument, getElementsByTagName);
    HHVM_ME(DOMDocument, getElementsByTagNameNS);
    HHVM_ME(DOMDocument, importNode);
    HHVM_ME(DOMDocument, _load);
    HHVM_ME(DOMDocument, _loadHTML);
    HHVM_ME(DOMDocument, normalizeDocument);
    HHVM_ME(DOMDocument, registerNodeClass);
    HHVM_ME(DOMDocument, relaxNGValidate);
    HHVM_ME(DOMDocument, relaxNGValidateSource);
    HHVM_ME(DOMDocument, save);
    HHVM_ME(DOMDocument, saveHTML);
    HHVM_ME(DOMDocument, saveHTMLFile);
    HHVM_ME(DOMDocument, saveXML);
    HHVM_ME(DOMDocument, schemaValidate);
    HHVM_ME(DOMDocument, schemaValidateSource);
    HHVM_ME(DOMDocument, validate);
    HHVM_ME(DOMDocument, xinclude);
    HHVM_ME(DOMDocument, __debugInfo);
    Native::registerNativePropHandler<DOMDocumentPropHandler>(
      DOMDocument::className());

    HHVM_ME(DOMDocumentFragment, __construct);
    HHVM_ME(DOMDocumentFragment, appendXML);

    HHVM_ME(DOMDocumentType, __debugInfo);
    Native::registerNativePropHandler<DOMDocumentTypePropHandler>(
      DOMDocumentType::className());

    HHVM_ME(DOMElement, __construct);
    HHVM_ME(DOMElement, getAttribute);
    HHVM_ME(DOMElement, getAttributeNode);
    HHVM_ME(DOMElement, getAttributeNodeNS);
    HHVM_ME(DOMElement, getAttributeNS);
    HHVM_ME(DOMElement, getElementsByTagName);
    HHVM_ME(DOMElement, getElementsByTagNameNS);
    HHVM_ME(DOMElement, hasAttribute);
    HHVM_ME(DOMElement, hasAttributeNS);
    HHVM_ME(DOMElement, removeAttribute);
    HHVM_ME(DOMElement, removeAttributeNode);
    HHVM_ME(DOMElement, removeAttributeNS);
    HHVM_ME(DOMElement, setAttribute);
    HHVM_ME(DOMElement, setAttributeNode);
    HHVM_ME(DOMElement, setAttributeNodeNS);
    HHVM_ME(DOMElement, setAttributeNS);
    HHVM_ME(DOMElement, setIDAttribute);
    HHVM_ME(DOMElement, setIDAttributeNode);
    HHVM_ME(DOMElement, setIDAttributeNS);
    HHVM_ME(DOMElement, __debugInfo);
    Native::registerNativeDataInfo<DOMElement>(Native::NDIFlags::NO_SWEEP);
    Native::registerNativePropHandler<DOMElementPropHandler>(
      DOMElement::className());

    HHVM_ME(DOMEntity, __debugInfo);
    Native::registerNativePropHandler<DOMEntityPropHandler>(
      DOMEntity::className());

    HHVM_ME(DOMEntityReference, __construct);

    HHVM_ME(DOMNotation, __debugInfo);
    Native::registerNativePropHandler<DOMNotationPropHandler>(
      DOMNotation::className());

    HHVM_ME(DOMProcessingInstruction, __construct);
    HHVM_ME(DOMProcessingInstruction, __debugInfo);
    Native::registerNativePropHandler<DOMProcessingInstructionPropHandler>(
      DOMProcessingInstruction::className());

    HHVM_ME(DOMNodeIterator, current);
    HHVM_ME(DOMNodeIterator, key);
    HHVM_ME(DOMNodeIterator, next);
    HHVM_ME(DOMNodeIterator, rewind);
    HHVM_ME(DOMNodeIterator, valid);
    Native::registerNativeDataInfo<DOMNodeIterator>(Native::NDIFlags::NO_SWEEP);

    HHVM_ME(DOMNamedNodeMap, getNamedItem);
    HHVM_ME(DOMNamedNodeMap, getNamedItemNS);
    HHVM_ME(DOMNamedNodeMap, item);
    HHVM_ME(DOMNamedNodeMap, getIterator);
    Native::registerNativePropHandler<DOMNamedNodeMapPropHandler>(
      DOMNamedNodeMap::className());

    HHVM_ME(DOMNodeList, item);
    HHVM_ME(DOMNodeList, getIterator);
    HHVM_ME(DOMNodeList, __debugInfo);
    Native::registerNativePropHandler<DOMNodeListPropHandler>(
      DOMNodeList::className());

    Native::registerNativeDataInfo<DOMIterable>(
      DOMNamedNodeMap::className().get(), Native::NDIFlags::NO_SWEEP);
    Native::registerNativeDataInfo<DOMIterable>(
      DOMNodeList::className().get(), Native::NDIFlags::NO_SWEEP);

    HHVM_ME(DOMImplementation, createDocument);
    HHVM_ME(DOMImplementation, createDocumentType);
    HHVM_ME(DOMImplementation, hasFeature);

    HHVM_ME(DOMXPath, __construct);
    HHVM_ME(DOMXPath, evaluate);
    HHVM_ME(DOMXPath, query);
    HHVM_ME(DOMXPath, registerNamespace);
    HHVM_ME(DOMXPath, registerPHPFunctions);
    HHVM_ME(DOMXPath, __debugInfo);
    Native::registerNativeDataInfo<DOMXPath>();
    Native::registerNativePropHandler<DOMXPathPropHandler>(DOMXPath::className());

    HHVM_FE(dom_import_simplexml);

    HHVM_RC_INT_SAME(DOMSTRING_SIZE_ERR);
    HHVM_RC_INT(DOM_HIERARCHY_REQUEST_ERR, HIERARCHY_REQUEST_ERR);
    HHVM_RC_INT(DOM_INDEX_SIZE_ERR, INDEX_SIZE_ERR);
    HHVM_RC_INT(DOM_INUSE_ATTRIBUTE_ERR, INUSE_ATTRIBUTE_ERR);
    HHVM_RC_INT(DOM_INVALID_ACCESS_ERR, INVALID_ACCESS_ERR);
    HHVM_RC_INT(DOM_INVALID_CHARACTER_ERR, INVALID_CHARACTER_ERR);
    HHVM_RC_INT(DOM_INVALID_MODIFICATION_ERR, INVALID_MODIFICATION_ERR);
    HHVM_RC_INT(DOM_INVALID_STATE_ERR, INVALID_STATE_ERR);
    HHVM_RC_INT(DOM_NAMESPACE_ERR, NAMESPACE_ERR);
    HHVM_RC_INT(DOM_NOT_FOUND_ERR, NOT_FOUND_ERR);
    HHVM_RC_INT(DOM_NOT_SUPPORTED_ERR, NOT_SUPPORTED_ERR);
    HHVM_RC_INT(DOM_NO_DATA_ALLOWED_ERR, NO_DATA_ALLOWED_ERR);
    HHVM_RC_INT(DOM_NO_MODIFICATION_ALLOWED_ERR, NO_MODIFICATION_ALLOWED_ERR);
    HHVM_RC_INT(DOM_PHP_ERR, PHP_ERR);
    HHVM_RC_INT(DOM_SYNTAX_ERR, SYNTAX_ERR);
    HHVM_RC_INT(DOM_VALIDATION_ERR, VALIDATION_ERR);
    HHVM_RC_INT(DOM_WRONG_DOCUMENT_ERR, WRONG_DOCUMENT_ERR);

    HHVM_RC_INT_SAME(XML_ELEMENT_NODE);
    HHVM_RC_INT_SAME(XML_ATTRIBUTE_NODE);
    HHVM_RC_INT_SAME(XML_TEXT_NODE);
    HHVM_RC_INT_SAME(XML_CDATA_SECTION_NODE);
    HHVM_RC_INT_SAME(XML_ENTITY_REF_NODE);
    HHVM_RC_INT_SAME(XML_ENTITY_NODE);
    HHVM_RC_INT_SAME(XML_PI_NODE);
    HHVM_RC_INT_SAME(XML_COMMENT_NODE);
    HHVM_RC_INT_SAME(XML_DOCUMENT_NODE);
    HHVM_RC_INT_SAME(XML_DOCUMENT_TYPE_NODE);
    HHVM_RC_INT_SAME(XML_DOCUMENT_FRAG_NODE);
    HHVM_RC_INT_SAME(XML_NOTATION_NODE);
    HHVM_RC_INT_SAME(XML_HTML_DOCUMENT_NODE);
    HHVM_RC_INT_SAME(XML_DTD_NODE);
    HHVM_RC_INT(XML_ELEMENT_DECL_NODE,   XML_ELEMENT_DECL);
    HHVM_RC_INT(XML_ATTRIBUTE_DECL_NODE, XML_ATTRIBUTE_DECL);
    HHVM_RC_INT(XML_ENTITY_DECL_NODE,    XML_ENTITY_DECL);
    HHVM_RC_INT(XML_NAMESPACE_DECL_NODE, XML_NAMESPACE_DECL);

    HHVM_RC_INT_SAME(XML_LOCAL_NAMESPACE);
#ifdef XML_GLOBAL_NAMESPACE
    HHVM_RC_INT_SAME(XML_GLOBAL_NAMESPACE);
#endif

    HHVM_RC_INT_SAME(XML_ATTRIBUTE_CDATA);
    HHVM_RC_INT_SAME(XML_ATTRIBUTE_ID);
    HHVM_RC_INT_SAME(XML_ATTRIBUTE_IDREF);
    HHVM_RC_INT_SAME(XML_ATTRIBUTE_IDREFS);
    HHVM_RC_INT(XML_ATTRIBUTE_ENTITY, XML_ATTRIBUTE_ENTITIES);
    HHVM_RC_INT_SAME(XML_ATTRIBUTE_NMTOKEN);
    HHVM_RC_INT_SAME(XML_ATTRIBUTE_NMTOKENS);
    HHVM_RC_INT_SAME(XML_ATTRIBUTE_ENUMERATION);
    HHVM_RC_INT_SAME(XML_ATTRIBUTE_NOTATION);
  }
} s_domdocument_extension;

}
