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

#include <runtime/ext/ext_domdocument.h>
#include <runtime/ext/ext_file.h>
#include <runtime/ext/ext_class.h>
#include <runtime/base/runtime_error.h>
#include <runtime/ext/ext_function.h>

#define DOM_XMLNS_NAMESPACE                             \
  (const xmlChar *) "http://www.w3.org/2000/xmlns/"

#define DOM_LOAD_STRING 0
#define DOM_LOAD_FILE 1

#define LIBXML_SAVE_NOEMPTYTAG 1<<2

#define PHP_DOM_XPATH_QUERY 0
#define PHP_DOM_XPATH_EVALUATE 1
#define DOM_NODESET XML_XINCLUDE_START

using namespace std;

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(dom);
///////////////////////////////////////////////////////////////////////////////
// parser error handling

#define PHP_LIBXML_CTX_ERROR 1
#define PHP_LIBXML_CTX_WARNING 2

static void php_libxml_internal_error_handler(int error_type, void *ctx,
                                              const char *fmt, va_list ap) {
  string msg;
  Logger::VSNPrintf(msg, fmt, ap);

  /* remove any trailing \n */
  while (!msg.empty() && msg[msg.size() - 1] == '\n') {
    msg = msg.substr(0, msg.size() - 1);
  }

  xmlParserCtxtPtr parser = (xmlParserCtxtPtr) ctx;
  if (parser != NULL && parser->input != NULL) {
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
        ASSERT(false);
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
        ASSERT(false);
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
      ASSERT(false);
      break;
    }
  }
}

static void php_libxml_ctx_error(void *ctx, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  try {
    php_libxml_internal_error_handler(PHP_LIBXML_CTX_ERROR, ctx, msg, args);
  } catch (...) {}
  va_end(args);
}

static void php_libxml_ctx_warning(void *ctx, const char *msg, ...) {
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
    c_domexception *e = NEW(c_domexception)();
    e->t___construct(error_message);
    throw Object(e);
  }
  raise_warning(error_message);
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
  xmlNodePtr ret = NULL;
  while (nodep != NULL && (*cur <= index || index == -1)) {
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
      if (ret != NULL) {
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
      while (nextp != NULL) {
        if (nextp->type == XML_TEXT_NODE) {
          newnextp = nextp->next;
          strContent = xmlNodeGetContent(nextp);
          xmlNodeAddContent(child, strContent);
          xmlFree(strContent);
          xmlUnlinkNode(nextp);
          nextp = newnextp;
        } else {
          break;
        }
      }
      break;
    case XML_ELEMENT_NODE:
      dom_normalize(child);
      attr = child->properties;
      while (attr != NULL) {
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

static Variant dom_canonicalization(xmlNodePtr nodep, CStrRef file,
                                    bool exclusive, bool with_comments,
                                    CVarRef xpath_array, CVarRef ns_prefixes,
                                    int mode) {
  xmlDocPtr docp;
  xmlNodeSetPtr nodeset = NULL;
  xmlChar **inclusive_ns_prefixes = NULL;
  int ret = -1;
  xmlOutputBufferPtr buf;
  xmlXPathContextPtr ctxp=NULL;
  xmlXPathObjectPtr xpathobjp=NULL;

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
      ctxp->node = NULL;
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
    if (!arr.exists("query")) {
      raise_warning("'query' missing from xpath array");
      return false;
    }
    Variant tmp = arr.rvalAt("query");
    if (!tmp.isString()) {
      raise_warning("'query' is not a string");
      return false;
    }
    xquery = tmp.toString();
    ctxp = xmlXPathNewContext(docp);
    ctxp->node = nodep;
    if (arr.exists("namespaces")) {
      Variant tmp = arr.rvalAt("namespaces");
      if (tmp.isArray()) {
        for (ArrayIter it = tmp.toArray().begin(); !it; ++it) {
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
    ctxp->node = NULL;
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
      for (ArrayIter it = ns_prefixes.toArray().begin(); !it; ++it) {
        Variant tmpns = it.second();
        if (tmpns.isString()) {
          inclusive_ns_prefixes[nscount++] = (xmlChar*)tmpns.toString().data();
        }
      }
      inclusive_ns_prefixes[nscount] = NULL;
    } else {
      raise_notice("Inclusive namespace prefixes only allowed in "
                   "exlcusive mode.");
    }
  }
  if (mode == 1) {
    buf = xmlOutputBufferCreateFilename(file, NULL, 0);
  } else {
    buf = xmlAllocOutputBuffer(NULL);
  }
  if (buf != NULL) {
    ret = xmlC14NDocSaveTo(docp, nodeset, exclusive, inclusive_ns_prefixes,
                           with_comments, buf);
  }
  if (inclusive_ns_prefixes != NULL) {
    free(inclusive_ns_prefixes);
  }
  if (xpathobjp != NULL) {
    xmlXPathFreeObject(xpathobjp);
  }
  if (ctxp != NULL) {
    xmlXPathFreeContext(ctxp);
  }
  Variant retval;
  if (buf == NULL || ret < 0) {
    retval = false;
  } else {
    if (mode == 0) {
      ret = buf->buffer->use;
      if (ret > 0) {
        retval = String((char *)buf->buffer->content, ret, CopyString);
      } else {
        retval = String();
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
  return node->doc == NULL;
}

static void dom_set_old_ns(xmlDoc *doc, xmlNs *ns) {
  xmlNs *cur;
  if (doc == NULL) return;
  if (doc->oldNs == NULL) {
    doc->oldNs = (xmlNsPtr) xmlMalloc(sizeof(xmlNs));
    if (doc->oldNs == NULL) {
      return;
    }
    memset(doc->oldNs, 0, sizeof(xmlNs));
    doc->oldNs->type = XML_LOCAL_NAMESPACE;
    doc->oldNs->href = xmlStrdup(XML_XML_NAMESPACE);
    doc->oldNs->prefix = xmlStrdup((const xmlChar *)"xml");
  }
  cur = doc->oldNs;
  while (cur->next != NULL) {
    cur = cur->next;
  }
  cur->next = ns;
}

static void dom_reconcile_ns(xmlDocPtr doc, xmlNodePtr nodep) {
  xmlNsPtr nsptr, nsdftptr, curns, prevns = NULL;
  if (nodep->type == XML_ELEMENT_NODE) {
    // Following if block primarily used for inserting nodes created via
    // createElementNS
    if (nodep->nsDef != NULL) {
      curns = nodep->nsDef;
      while (curns) {
        nsdftptr = curns->next;
        if (curns->href != NULL) {
          if ((nsptr = xmlSearchNsByHref(doc, nodep->parent, curns->href)) &&
              (curns->prefix == NULL ||
               xmlStrEqual(nsptr->prefix, curns->prefix))) {
            curns->next = NULL;
            if (prevns == NULL) {
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
  if (parent == NULL || child == NULL || child->doc != parent->doc) {
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
    if (prevsib == NULL) {
      nodep->children = newchild;
    } else {
      prevsib->next = newchild;
    }
    newchild->prev = prevsib;
    if (nextsib == NULL) {
      nodep->last = fragment->last;
    } else {
      fragment->last->next = nextsib;
      nextsib->prev = fragment->last;
    }
    node = newchild;
    while (node != NULL) {
      node->parent = nodep;
      if (node->doc != nodep->doc) {
        xmlSetTreeDoc(node, nodep->doc);
      }
      if (node == fragment->last) {
        break;
      }
      node = node->next;
    }
    fragment->children = NULL;
    fragment->last = NULL;
  }
  return newchild;
}

static int dom_check_qname(const char *qname, char **localname, char **prefix,
                           int uri_len, int name_len) {
  if (name_len == 0) {
    return NAMESPACE_ERR;
  }
  *localname = (char *)xmlSplitQName2((xmlChar *)qname, (xmlChar **) prefix);
  if (*localname == NULL) {
    *localname = (char *)xmlStrdup((xmlChar *)qname);
    if (*prefix == NULL && uri_len == 0) {
      return 0;
    }
  }
  if (xmlValidateQName((xmlChar *) qname, 0) != 0) {
    return NAMESPACE_ERR;
  }
  if (*prefix != NULL && uri_len == 0) {
    return NAMESPACE_ERR;
  }
  return 0;
}

static xmlNsPtr dom_get_ns(xmlNodePtr nodep, const char *uri, int *errorcode,
                           const char *prefix) {
  xmlNsPtr nsptr = NULL;
  *errorcode = 0;
  if (!((prefix && !strcmp (prefix, "xml") &&
         strcmp(uri, (char *)XML_XML_NAMESPACE)) ||
        (prefix && !strcmp (prefix, "xmlns") &&
         strcmp(uri, (char *)DOM_XMLNS_NAMESPACE)) ||
        (prefix && !strcmp(uri, (char *)DOM_XMLNS_NAMESPACE) &&
         strcmp (prefix, "xmlns")))) {
    nsptr = xmlNewNs(nodep, (xmlChar *)uri, (xmlChar *)prefix);
  }
  if (nsptr == NULL) {
    *errorcode = NAMESPACE_ERR;
  }
  return nsptr;
}

static String _dom_get_valid_file_path(const char *source) {
  int isFileUri = 0;

  xmlURI *uri = xmlCreateURI();
  xmlChar *escsource = xmlURIEscapeStr((xmlChar*)source, (xmlChar*)":");
  xmlParseURIReference(uri, (char*)escsource);
  xmlFree(escsource);

  if (uri->scheme != NULL) {
    /* absolute file uris - libxml only supports localhost or empty host */
    if (strncasecmp(source, "file:///",8) == 0) {
      isFileUri = 1;
      source += 7;
    } else if (strncasecmp(source, "file://localhost/",17) == 0) {
      isFileUri = 1;
      source += 16;
    }
  }

  String file_dest = String(source, CopyString);
  if ((uri->scheme == NULL || isFileUri)) {
    file_dest = File::TranslatePath(file_dest);
  }
  xmlFreeURI(uri);
  return file_dest;
}

static xmlDocPtr dom_document_parser(c_domdocument * domdoc, int mode,
                                     char *source, int options) {
  xmlDocPtr ret = NULL;
  xmlParserCtxtPtr ctxt = NULL;

  bool validate, recover, resolve_externals, keep_blanks, substitute_ent;
  validate = domdoc->m_validateonparse;
  resolve_externals = domdoc->m_resolveexternals;
  keep_blanks = domdoc->m_preservewhitespace;
  substitute_ent = domdoc->m_substituteentities;
  recover = domdoc->m_recover;

  xmlInitParser();

  if (mode == DOM_LOAD_FILE) {
    String file_dest = _dom_get_valid_file_path(source);
    if (!file_dest.empty()) {
      ctxt = xmlCreateFileParserCtxt(file_dest.data());
    }
  } else {
    ctxt = xmlCreateDocParserCtxt((xmlChar*)source);
  }

  if (ctxt == NULL) {
    return NULL;
  }

  /* If loading from memory, we need to set the base directory
     for the document */
  if (mode != DOM_LOAD_FILE) {
    String directory = g_context->getCwd();
    if (!directory.empty()) {
      if (ctxt->directory != NULL) {
        xmlFree((char *) ctxt->directory);
      }
      if (directory[directory.size() - 1] != '/') {
        directory += '/';
      }
      ctxt->directory =
        (char*)xmlCanonicPath((const xmlChar*)directory.c_str());
    }
  }

  ctxt->vctxt.error = php_libxml_ctx_error;
  ctxt->vctxt.warning = php_libxml_ctx_warning;
  if (ctxt->sax != NULL) {
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
    old_error_reporting = g_context->getErrorReportingLevel();
    g_context->setErrorReportingLevel(old_error_reporting | k_E_WARNING);
  }

  xmlParseDocument(ctxt);

  if (ctxt->wellFormed || recover) {
    ret = ctxt->myDoc;
    if (ctxt->recovery) {
      g_context->setErrorReportingLevel(old_error_reporting);
    }
    /* If loading from memory, set the base reference uri for the document */
    if (ret && ret->URL == NULL && ctxt->directory != NULL) {
      ret->URL = xmlStrdup((xmlChar*)ctxt->directory);
    }
  } else {
    ret = NULL;
    xmlFreeDoc(ctxt->myDoc);
    ctxt->myDoc = NULL;
  }

  xmlFreeParserCtxt(ctxt);

  return ret;
}

static Variant dom_parse_document(c_domdocument *domdoc, CStrRef source,
                                  int options, int mode) {
  if (source.empty()) {
    raise_warning("Empty string supplied as input");
    return false;
  }
  xmlDoc *newdoc =
    dom_document_parser(domdoc, mode, (char*)source.data(), options);
  if (!newdoc) {
    return false;
  }
  if (domdoc->m_node) {
    xmlFreeDoc((xmlDocPtr)domdoc->m_node);
  }
  domdoc->m_node = (xmlNodePtr)newdoc;
  return true;
}

static Variant dom_load_html(c_domdocument *domdoc, CStrRef source, int mode) {
  if (source.empty()) {
    raise_warning("Empty string supplied as input");
    return false;
  }
  htmlParserCtxtPtr ctxt;
  if (mode == DOM_LOAD_FILE) {
    ctxt = htmlCreateFileParserCtxt(source.data(), NULL);
  } else {
    ctxt = htmlCreateMemoryParserCtxt(source.data(), source.size());
  }
  if (!ctxt) {
    return false;
  }
  ctxt->vctxt.error = php_libxml_ctx_error;
  ctxt->vctxt.warning = php_libxml_ctx_warning;
  if (ctxt->sax != NULL) {
    ctxt->sax->error = php_libxml_ctx_error;
    ctxt->sax->warning = php_libxml_ctx_warning;
  }
  htmlParseDocument(ctxt);
  xmlDocPtr newdoc = ctxt->myDoc;
  htmlFreeParserCtxt(ctxt);
  if (!newdoc) {
    return false;
  }
  if (domdoc->m_node) {
    xmlFreeDoc((xmlDocPtr)domdoc->m_node);
  }
  domdoc->m_node = (xmlNodePtr)newdoc;
  return domdoc;
}

static bool _dom_document_relaxNG_validate(c_domdocument *domdoc,
                                           CStrRef source, int type) {
  xmlRelaxNGParserCtxtPtr parser;
  xmlRelaxNGPtr           sptr;
  xmlRelaxNGValidCtxtPtr  vptr;
  int                     is_valid;

  if (source.empty()) {
    raise_warning("Invalid Schema source");
    return false;
  }

  xmlDocPtr docp = (xmlDocPtr)domdoc->m_node;

  switch (type) {
  case DOM_LOAD_FILE:
    {
      String valid_file = _dom_get_valid_file_path(source.data());
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
     (xmlRelaxNGValidityWarningFunc) php_libxml_ctx_error, parser);
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
                           php_libxml_ctx_error, vptr);
  is_valid = xmlRelaxNGValidateDoc(vptr, docp);
  xmlRelaxNGFree(sptr);
  xmlRelaxNGFreeValidCtxt(vptr);

  return is_valid == 0;
}

static bool _dom_document_schema_validate(c_domdocument * domdoc,
                                          CStrRef source, int type) {
  xmlDocPtr docp = (xmlDocPtr)domdoc->m_node;
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
      String valid_file = _dom_get_valid_file_path(source.data());
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
    (xmlSchemaValidityWarningFunc) php_libxml_ctx_error, parser);
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
                          php_libxml_ctx_error, vptr);
  is_valid = xmlSchemaValidateDoc(vptr, docp);
  xmlSchemaFree(sptr);
  xmlSchemaFreeValidCtxt(vptr);

  return is_valid == 0;
}

static void php_libxml_node_free(xmlNodePtr node) {
  if (node) {
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

static void php_libxml_node_free_list(xmlNodePtr node) {
  xmlNodePtr curnode;

  if (node != NULL) {
    curnode = node;
    while (curnode != NULL) {
      node = curnode;
      switch (node->type) {
      /* Skip property freeing for the following types */
      case XML_NOTATION_NODE:
        break;
      case XML_ENTITY_REF_NODE:
        php_libxml_node_free_list((xmlNodePtr) node->properties);
        break;
      case XML_ATTRIBUTE_NODE:
        if ((node->doc != NULL) &&
            (((xmlAttrPtr) node)->atype == XML_ATTRIBUTE_ID)) {
          xmlRemoveID(node->doc, (xmlAttrPtr) node);
        }
      case XML_ATTRIBUTE_DECL:
      case XML_DTD_NODE:
      case XML_DOCUMENT_TYPE_NODE:
      case XML_ENTITY_DECL:
      case XML_NAMESPACE_DECL:
      case XML_TEXT_NODE:
        php_libxml_node_free_list(node->children);
        break;
      default:
        php_libxml_node_free_list(node->children);
        php_libxml_node_free_list((xmlNodePtr) node->properties);
      }

      curnode = node->next;
      xmlUnlinkNode(node);
      node->doc = NULL;
      php_libxml_node_free(node);
    }
  }
}

void php_libxml_node_free_resource(xmlNodePtr node) {
  if (node) {
    switch (node->type) {
    case XML_DOCUMENT_NODE:
    case XML_HTML_DOCUMENT_NODE:
      break;
    default:
      if (node->parent == NULL || node->type == XML_NAMESPACE_DECL) {
        php_libxml_node_free_list((xmlNodePtr) node->children);
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
          php_libxml_node_free_list((xmlNodePtr) node->properties);
        }
        node->doc = NULL;
        php_libxml_node_free(node);
      }
    }
  }
}

static xmlNodePtr php_dom_free_xinclude_node(xmlNodePtr cur) {
  xmlNodePtr xincnode;
  xincnode = cur;
  cur = cur->next;
  xmlUnlinkNode(xincnode);
  php_libxml_node_free_resource(xincnode);
  return cur;
}

static void php_dom_remove_xinclude_nodes(xmlNodePtr cur) {
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
      while (prop != NULL) {
        prop->doc = doc;
        if (prop->children) {
          cur = prop->children;
          while (cur != NULL) {
            php_dom_xmlSetTreeDoc(cur, doc);
            cur = cur->next;
          }
        }
        prop = prop->next;
      }
    }
    if (tree->children != NULL) {
      cur = tree->children;
      while (cur != NULL) {
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
  if (nqname != NULL) {
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
    if (prefix != NULL) {
      xmlFree(prefix);
    }
    if (ns != NULL) {
      return (xmlNodePtr)xmlHasNsProp(elem, nqname, ns->href);
    }
  } else {
    if (xmlStrEqual(name, (xmlChar*)"xmlns")) {
      xmlNsPtr nsPtr = elem->nsDef;
      while (nsPtr) {
        if (nsPtr->prefix == NULL) {
          return (xmlNodePtr)nsPtr;
        }
        nsPtr = nsPtr->next;
      }
      return NULL;
    }
  }
  return (xmlNodePtr)xmlHasNsProp(elem, name, NULL);
}

static xmlNsPtr dom_get_nsdecl(xmlNode *node, xmlChar *localName) {
  xmlNsPtr cur;
  xmlNs *ret = NULL;
  if (node == NULL) {
    return NULL;
  }
  if (localName == NULL || xmlStrEqual(localName, (xmlChar *)"")) {
    cur = node->nsDef;
    while (cur != NULL) {
      if (cur->prefix == NULL  && cur->href != NULL) {
        ret = cur;
        break;
      }
      cur = cur->next;
    }
  } else {
    cur = node->nsDef;
    while (cur != NULL) {
      if (cur->prefix != NULL && xmlStrEqual(localName, cur->prefix)) {
        ret = cur;
        break;
      }
      cur = cur->next;
    }
  }
  return ret;
}

static Variant php_dom_create_object(xmlNodePtr obj, sp_domdocument doc,
                                     bool owner = false) {
  const char *clsname = NULL;
  switch (obj->type) {
  case XML_DOCUMENT_NODE:
  case XML_HTML_DOCUMENT_NODE: clsname = "domdocument";              break;
  case XML_DTD_NODE:
  case XML_DOCUMENT_TYPE_NODE: clsname = "domdocumenttype";          break;
  case XML_ELEMENT_NODE:       clsname = "domelement";               break;
  case XML_ATTRIBUTE_NODE:     clsname = "domattr";                  break;
  case XML_TEXT_NODE:          clsname = "domtext";                  break;
  case XML_COMMENT_NODE:       clsname = "domcomment";               break;
  case XML_PI_NODE:            clsname = "domprocessinginstruction"; break;
  case XML_ENTITY_REF_NODE:    clsname = "domentityreference";       break;
  case XML_ENTITY_DECL:
  case XML_ELEMENT_DECL:       clsname = "domentity";                break;
  case XML_CDATA_SECTION_NODE: clsname = "domcdatasection";          break;
  case XML_DOCUMENT_FRAG_NODE: clsname = "domdocumentfragment";      break;
  case XML_NOTATION_NODE:      clsname = "domnotation";              break;
  default:
    raise_warning("Unsupported node type: %d", obj->type);
    return null;
  }

  if (doc.get() && doc->m_classmap.exists(clsname)) {
    ASSERT(doc->m_classmap[clsname].isString()); // or const char * is not safe
    clsname = doc->m_classmap[clsname].toString().data();
  }
  Object wrapper = create_object(clsname, Array(), false);
  c_domnode *nodeobj = wrapper.getTyped<c_domnode>();
  nodeobj->m_node = obj;
  nodeobj->m_doc = doc;
  nodeobj->m_owner = owner;
  return wrapper;
}

static Variant create_node_object(xmlNodePtr node, sp_domdocument doc,
                                  bool owner = false) {
  if (!node) {
    return null;
  }
  Variant retval = php_dom_create_object(node, doc, owner);
  if (retval.isNull()) {
    raise_warning("Cannot create required DOM object");
  }
  return retval;
}

static Variant php_xpath_eval(c_domxpath * domxpath, CStrRef expr,
                              CObjRef context, int type) {
  xmlXPathObjectPtr xpathobjp;
  int nsnbr = 0, xpath_type;
  xmlDoc *docp = NULL;
  xmlNsPtr *ns;
  xmlXPathContextPtr ctxp = (xmlXPathContextPtr)domxpath->m_node;
  if (ctxp == NULL) {
    raise_warning("Invalid XPath Context");
    return false;
  }
  docp = (xmlDocPtr)ctxp->doc;
  if (docp == NULL) {
    raise_warning("Invalid XPath Document Pointer");
    return false;
  }
  xmlNodePtr nodep = NULL;
  if (!context.isNull()) {
    c_domnode *domnode = context.getTyped<c_domnode>();
    nodep = domnode->m_node;
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
  ns = xmlGetNsList(docp, nodep);
  if (ns != NULL) {
    while (ns[nsnbr] != NULL) nsnbr++;
  }
  ctxp->namespaces = ns;
  ctxp->nsNr = nsnbr;
  xpathobjp = xmlXPathEvalExpression((xmlChar*)expr.data(), ctxp);
  ctxp->node = NULL;
  if (ns != NULL) {
    xmlFree(ns);
    ctxp->namespaces = NULL;
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
    Array retval = Array::Create();
    if (xpathobjp->type == XPATH_NODESET &&
        NULL != (nodesetp = xpathobjp->nodesetval)) {
      for (i = 0; i < nodesetp->nodeNr; i++) {
        xmlNodePtr node = nodesetp->nodeTab[i];
        bool owner = false;
        if (node->type == XML_NAMESPACE_DECL) {
          xmlNsPtr curns;
          //xmlNodePtr nsparent;
          //nsparent = node->_private;
          curns = xmlNewNs(NULL, node->name, NULL);
          if (node->children) {
            curns->prefix = xmlStrdup((xmlChar*)node->children);
          }
          if (node->children) {
            node = xmlNewDocNode(docp, NULL, (xmlChar*)node->children,
                                 node->name);
          } else {
            node = xmlNewDocNode(docp, NULL, (xmlChar*)"xmlns", node->name);
          }
          owner = true;
          node->type = XML_NAMESPACE_DECL;
          //node->parent = nsparent;
          node->ns = curns;
        }
        // Ugh, the statement below creates a new object and
        // adds it to an array...
        retval.append(create_node_object(node, domxpath->m_doc, owner));
      }
    }
    c_domnodelist *nodelist = NEW(c_domnodelist)();
    nodelist->m_doc = domxpath->m_doc;
    nodelist->m_baseobjptr = retval;
    nodelist->m_nodetype = DOM_NODESET;
    ret = nodelist;
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
    ret = null;
    break;
  }
  xmlXPathFreeObject(xpathobjp);
  return ret;
}

static void node_list_unlink(xmlNodePtr node) {
  while (node != NULL) {
    if (node->type == XML_ENTITY_REF_NODE) break;
    node_list_unlink(node->children);
    switch (node->type) {
    case XML_ATTRIBUTE_DECL:
    case XML_DTD_NODE:
    case XML_DOCUMENT_TYPE_NODE:
    case XML_ENTITY_DECL:
    case XML_ATTRIBUTE_NODE:
    case XML_TEXT_NODE:
      break;
    default:
      node_list_unlink((xmlNodePtr)node->properties);
    }
    node = node->next;
  }
}

static void php_set_attribute_id(xmlAttrPtr attrp, bool is_id) {
  if (is_id == 1 && attrp->atype != XML_ATTRIBUTE_ID) {
    xmlChar *id_val = xmlNodeListGetString(attrp->doc, attrp->children, 1);
    if (id_val != NULL) {
      xmlAddID(NULL, attrp->doc, id_val, attrp);
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
  if ((tree == NULL) || (ns == NULL) || (ns->type != XML_NAMESPACE_DECL)) {
    return NULL;
  }
  /* Code taken from libxml2 (2.6.20) xmlNewReconciliedNs
   *
   * Find a close prefix which is not already in use.
   * Let's strip namespace prefixes longer than 20 chars!
   */
  if (ns->prefix == NULL) {
    snprintf((char*)prefix, sizeof(prefix), "default");
  } else {
    snprintf((char*)prefix, sizeof(prefix), "%.20s", (char *)ns->prefix);
  }
  def = xmlSearchNs(doc, tree, prefix);
  while (def != NULL) {
    if (counter > 1000) return(NULL);
    if (ns->prefix == NULL)
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

static const char * getStringData(CStrRef str) {
  if (str.isNull()) {
    return NULL;
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
  ret->content = NULL;
  ret->URI = NULL;
  ret->orig = NULL;
  ret->children = NULL;
  ret->parent = NULL;
  ret->doc = NULL;
  ret->_private = NULL;
  ret->last = NULL;
  ret->prev = NULL;
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

static void itemHashScanner(void *payload, void *data, xmlChar *name) {
  nodeIterator *priv = (nodeIterator *)data;
  if (priv->cur < priv->index) {
    priv->cur++;
  } else if (priv->node == NULL) {
    priv->node = (xmlNode *)payload;
  }
}

static xmlNode *php_dom_libxml_hash_iter(xmlHashTable *ht, int index) {
  int htsize = xmlHashSize(ht);
  if (htsize > 0 && index < htsize) {
    nodeIterator iter;
    iter.cur = 0;
    iter.index = index;
    iter.node = NULL;
    xmlHashScan(ht, itemHashScanner, &iter);
    return iter.node;
  }
  return NULL;
}

static xmlNode *php_dom_libxml_notation_iter(xmlHashTable *ht, int index) {
  int htsize = xmlHashSize(ht);
  if (htsize > 0 && index < htsize) {
    notationIterator iter;
    iter.cur = 0;
    iter.index = index;
    iter.notation = NULL;
    xmlHashScan(ht, itemHashScanner, &iter);
    xmlNotation *notep = iter.notation;
    return create_notation(notep->name, notep->PublicID, notep->SystemID);
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////

Variant dummy_getter(CObjRef) {
  raise_error("Cannot read property");
  return null;
}

void dummy_setter(CObjRef,CVarRef) {
  raise_error("Cannot write property");
}

struct PropertyAccessor {
  const char * name;
  Variant (*getter)(CObjRef);
  void (*setter)(CObjRef,CVarRef);
};

class PropertyAccessorMap : private hphp_const_char_imap<PropertyAccessor*> {
public:
  PropertyAccessorMap(PropertyAccessor* props,
                      PropertyAccessorMap *base = NULL) {
    if (base) {
      *this = *base;
    }
    for (PropertyAccessor *p = props; p->name; p++) {
      (*this)[p->name] = p;
    }
  }

  Variant (*getter(CVarRef name))(CObjRef) {
    if (name.isString()) {
      const_iterator iter = find(name.toString().data());
      if (iter != end() && iter->second->getter) {
        return iter->second->getter;
      }
    }
    return dummy_getter;
  }

  void (*setter(CVarRef name))(CObjRef,CVarRef) {
    if (name.isString()) {
      const_iterator iter = find(name.toString().data());
      if (iter != end() && iter->second->setter) {
        return iter->second->setter;
      }
    }
    return dummy_setter;
  }
};

///////////////////////////////////////////////////////////////////////////////

#define CHECK_NODE(nodep)                               \
  c_domnode *domnode = obj.getTyped<c_domnode>();       \
  xmlNodePtr nodep = domnode->m_node;                   \
  if (nodep == NULL) {                                  \
    php_dom_throw_error(INVALID_STATE_ERR, 0);          \
    return null;                                        \
  }                                                     \

#define CHECK_WRITE_NODE(nodep)                         \
  c_domnode *domnode = obj.getTyped<c_domnode>();       \
  xmlNodePtr nodep = domnode->m_node;                   \
  if (nodep == NULL) {                                  \
    php_dom_throw_error(INVALID_STATE_ERR, 0);          \
    return;                                             \
  }                                                     \

static Variant domnode_nodename_read(CObjRef obj) {
  CHECK_NODE(nodep);
  xmlNsPtr ns;
  char *str = NULL;
  xmlChar *qname = NULL;
  switch (nodep->type) {
  case XML_ATTRIBUTE_NODE:
  case XML_ELEMENT_NODE:
    ns = nodep->ns;
    if (ns != NULL && ns->prefix) {
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
    if (ns != NULL && ns->prefix) {
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
  if (qname != NULL) {
    xmlFree(qname);
  }
  return retval;
}

static Variant domnode_nodevalue_read(CObjRef obj) {
  CHECK_NODE(nodep);
  char *str = NULL;
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
    str = NULL;
    break;
  }
  if (str != NULL) {
    String retval(str, CopyString);
    xmlFree(str);
    return retval;
  } else {
    return null;
  }
}

static void domnode_nodevalue_write(CObjRef obj, CVarRef value) {
  CHECK_WRITE_NODE(nodep);
  /* Access to Element node is implemented as a convience method */
  switch (nodep->type) {
  case XML_ELEMENT_NODE:
  case XML_ATTRIBUTE_NODE:
    if (nodep->children) {
      node_list_unlink(nodep->children);
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

static Variant domnode_nodetype_read(CObjRef obj) {
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

static Variant domnode_parentnode_read(CObjRef obj) {
  CHECK_NODE(nodep);
  return create_node_object(nodep->parent, domnode->m_doc);
}

static Variant domnode_childnodes_read(CObjRef obj) {
  CHECK_NODE(nodep);
  if (!dom_node_children_valid(nodep)) {
    return null;
  }
  c_domnodelist *retval = NEW(c_domnodelist)();
  retval->m_doc = domnode->m_doc;
  retval->m_baseobj = obj;
  retval->m_nodetype = XML_ELEMENT_NODE;
  return retval;
}

static Variant domnode_firstchild_read(CObjRef obj) {
  CHECK_NODE(nodep);
  xmlNode *first = NULL;
  if (dom_node_children_valid(nodep)) {
    first = nodep->children;
  }
  return create_node_object(first, domnode->m_doc);
}

static Variant domnode_lastchild_read(CObjRef obj) {
  CHECK_NODE(nodep);
  xmlNode *last = NULL;
  if (dom_node_children_valid(nodep)) {
    last = nodep->last;
  }
  return create_node_object(last, domnode->m_doc);
}

static Variant domnode_previoussibling_read(CObjRef obj) {
  CHECK_NODE(nodep);
  return create_node_object(nodep->prev, domnode->m_doc);
}

static Variant domnode_nextsibling_read(CObjRef obj) {
  CHECK_NODE(nodep);
  return create_node_object(nodep->next, domnode->m_doc);
}

static Variant domnode_attributes_read(CObjRef obj) {
  CHECK_NODE(nodep);
  if (nodep->type == XML_ELEMENT_NODE) {
    c_domnamednodemap *nodemap = NEW(c_domnamednodemap)();
    nodemap->m_doc = domnode->m_doc;
    nodemap->m_baseobj = obj;
    nodemap->m_nodetype = XML_ATTRIBUTE_NODE;
    return nodemap;
  }
  return null;
}

static Variant domnode_ownerdocument_read(CObjRef obj) {
  CHECK_NODE(nodep);
  if (nodep->type == XML_DOCUMENT_NODE ||
      nodep->type == XML_HTML_DOCUMENT_NODE) {
    return null;
  }
  return create_node_object((xmlNodePtr)nodep->doc, domnode->m_doc);
}

static Variant domnode_namespaceuri_read(CObjRef obj) {
  CHECK_NODE(nodep);
  const char *str = NULL;
  switch (nodep->type) {
  case XML_ELEMENT_NODE:
  case XML_ATTRIBUTE_NODE:
  case XML_NAMESPACE_DECL:
    if (nodep->ns != NULL) {
      str = (const char*)nodep->ns->href;
    }
    break;
  default:
    break;
  }
  if (str) {
    return String(str, CopyString);
  }
  return null;
}

static Variant domnode_prefix_read(CObjRef obj) {
  CHECK_NODE(nodep);
  xmlNsPtr ns;
  char *str = NULL;
  switch (nodep->type) {
  case XML_ELEMENT_NODE:
  case XML_ATTRIBUTE_NODE:
  case XML_NAMESPACE_DECL:
    ns = nodep->ns;
    if (ns != NULL && ns->prefix) {
      str = (char *)ns->prefix;
    }
    break;
  default:
    break;
  }

  if (str) {
    return String(str, CopyString);
  }
  return "";
}

static void domnode_prefix_write(CObjRef obj, CVarRef value) {
  String svalue;
  xmlNode *nsnode = NULL;
  xmlNsPtr ns = NULL, curns;
  const char *strURI;
  const char *prefix;

  CHECK_WRITE_NODE(nodep);
  switch (nodep->type) {
  case XML_ELEMENT_NODE:
    nsnode = nodep;
    // fall through
  case XML_ATTRIBUTE_NODE:
    if (nsnode == NULL) {
      nsnode = nodep->parent;
      if (nsnode == NULL) {
        nsnode = xmlDocGetRootElement(nodep->doc);
      }
    }
    svalue = value.toString();
    prefix = svalue.data();
    if (nsnode && nodep->ns != NULL &&
        !xmlStrEqual(nodep->ns->prefix, (xmlChar *)prefix)) {
      strURI = (char *) nodep->ns->href;
      if (strURI == NULL ||
          (!strcmp (prefix, "xml") &&
           strcmp(strURI, (const char *)XML_XML_NAMESPACE)) ||
          (nodep->type == XML_ATTRIBUTE_NODE && !strcmp (prefix, "xmlns") &&
           strcmp (strURI, (const char *)DOM_XMLNS_NAMESPACE)) ||
          (nodep->type == XML_ATTRIBUTE_NODE &&
           !strcmp ((const char*)nodep->name, "xmlns"))) {
        ns = NULL;
      } else {
        curns = nsnode->nsDef;
        while (curns != NULL) {
          if (xmlStrEqual((xmlChar *)prefix, curns->prefix) &&
              xmlStrEqual(nodep->ns->href, curns->href)) {
            ns = curns;
            break;
          }
          curns = curns->next;
        }
        if (ns == NULL) {
          ns = xmlNewNs(nsnode, nodep->ns->href, (xmlChar *)prefix);
        }
      }

      if (ns == NULL) {
        php_dom_throw_error(NAMESPACE_ERR, domnode->m_doc->m_stricterror);
        return;
      }

      xmlSetNs(nodep, ns);
    }
    break;
  default:
    break;
  }
}

static Variant domnode_localname_read(CObjRef obj) {
  CHECK_NODE(nodep);
  if (nodep->type == XML_ELEMENT_NODE ||
      nodep->type == XML_ATTRIBUTE_NODE ||
      nodep->type == XML_NAMESPACE_DECL) {
    return String((char *)(nodep->name), CopyString);
  }
  return null;
}

static Variant domnode_baseuri_read(CObjRef obj) {
  CHECK_NODE(nodep);
  xmlChar *baseuri = xmlNodeGetBase(nodep->doc, nodep);
  if (baseuri) {
    String ret((char *)(baseuri), CopyString);
    xmlFree(baseuri);
    return ret;
  }
  return null;
}

static Variant domnode_textcontent_read(CObjRef obj) {
  CHECK_NODE(nodep);
  char *str = (char *)xmlNodeGetContent(nodep);
  if (str) {
    String ret(str, CopyString);
    xmlFree(str);
    return ret;
  }
  return "";
}

static void domnode_textcontent_write(CObjRef obj, CVarRef value) {
  // do nothing
}

static PropertyAccessor domnode_properties[] = {
  { "nodeName",        domnode_nodename_read,      NULL },
  { "nodeValue",       domnode_nodevalue_read,     domnode_nodevalue_write },
  { "nodeType",        domnode_nodetype_read,      NULL },
  { "parentNode",      domnode_parentnode_read,    NULL },
  { "childNodes",      domnode_childnodes_read,    NULL },
  { "firstChild",      domnode_firstchild_read,    NULL },
  { "lastChild",       domnode_lastchild_read,     NULL },
  { "previousSibling", domnode_previoussibling_read, NULL },
  { "nextSibling",     domnode_nextsibling_read,   NULL },
  { "attributes",      domnode_attributes_read,    NULL },
  { "ownerDocument",   domnode_ownerdocument_read, NULL },
  { "namespaceURI",    domnode_namespaceuri_read,  NULL },
  { "prefix",          domnode_prefix_read,        domnode_prefix_write },
  { "localName",       domnode_localname_read,     NULL },
  { "baseURI",         domnode_baseuri_read,       NULL },
  { "textContent",     domnode_textcontent_read,   domnode_textcontent_write },
  { NULL, NULL, NULL}
};
static PropertyAccessorMap domnode_properties_map
((PropertyAccessor*)domnode_properties);

///////////////////////////////////////////////////////////////////////////////

c_domnode::c_domnode() : m_node(NULL), m_owner(false) {
}

c_domnode::~c_domnode() {
  if (m_owner && m_node) {
    xmlUnlinkNode(m_node);
    php_libxml_node_free(m_node);
    m_node = NULL;
  }
}

void c_domnode::t___construct() {
}

Variant c_domnode::t___destruct() {
  return null;
}

Variant c_domnode::t___get(Variant name) {
  return domnode_properties_map.getter(name)(this);
}

Variant c_domnode::t___set(Variant name, Variant value) {
  domnode_properties_map.setter(name)(this, value);
  return null;
}

Variant c_domnode::t_appendchild(CObjRef newnode) {
  xmlNodePtr nodep = m_node;
  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  c_domnode *newdomnode = newnode.getTyped<c_domnode>();
  xmlNodePtr child = newdomnode->m_node;
  xmlNodePtr new_child = NULL;
  if (dom_node_is_read_only(nodep) ||
      (child->parent != NULL && dom_node_is_read_only(child->parent))) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, m_doc->m_stricterror);
    return false;
  }
  if (!dom_hierarchy(nodep, child)) {
    php_dom_throw_error(HIERARCHY_REQUEST_ERR, m_doc->m_stricterror);
    return false;
  }
  if (!(child->doc == NULL || child->doc == nodep->doc)) {
    php_dom_throw_error(WRONG_DOCUMENT_ERR, m_doc->m_stricterror);
    return false;
  }
  if (child->type == XML_DOCUMENT_FRAG_NODE && child->children == NULL) {
    raise_warning("Document Fragment is empty");
    return false;
  }
  if (child->parent != NULL){
    xmlUnlinkNode(child);
  }
  if (child->type == XML_TEXT_NODE && nodep->last != NULL &&
      nodep->last->type == XML_TEXT_NODE) {
    child->parent = nodep;
    if (child->doc == NULL) {
      xmlSetTreeDoc(child, nodep->doc);
    }
    new_child = child;
    if (nodep->children == NULL) {
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
    if (child->ns == NULL) {
      lastattr = xmlHasProp(nodep, child->name);
    } else {
      lastattr = xmlHasNsProp(nodep, child->name, child->ns->href);
    }
    if (lastattr != NULL && lastattr->type != XML_ATTRIBUTE_DECL) {
      if (lastattr != (xmlAttrPtr)child) {
        xmlUnlinkNode((xmlNodePtr)lastattr);
        php_libxml_node_free_resource((xmlNodePtr)lastattr);
      }
    }
  } else if (child->type == XML_DOCUMENT_FRAG_NODE) {
    new_child = _php_dom_insert_fragment(nodep, nodep->last, NULL, child);
  }
  if (new_child == NULL) {
    new_child = xmlAddChild(nodep, child);
    if (new_child == NULL) {
      raise_warning("Couldn't append node");
      return false;
    }
  }
  newdomnode->m_owner = false;
  dom_reconcile_ns(nodep->doc, new_child);
  return create_node_object(new_child, m_doc, false);
}

Variant c_domnode::t_clonenode(bool deep /* = false */) {
  xmlNodePtr n = m_node;
  xmlNode * node = xmlDocCopyNode(n, n->doc, deep);
  if (!node) {
    return false;
  }
  // When deep is false Element nodes still require the attributes
  // Following taken from libxml as xmlDocCopyNode doesnt do this
  if (n->type == XML_ELEMENT_NODE && !deep) {
    if (n->nsDef != NULL) {
      node->nsDef = xmlCopyNamespaceList(n->nsDef);
    }
    if (n->ns != NULL) {
      xmlNsPtr ns;
      ns = xmlSearchNs(n->doc, node, n->ns->prefix);
      if (ns == NULL) {
        ns = xmlSearchNs(n->doc, n, n->ns->prefix);
        if (ns != NULL) {
          xmlNodePtr root = node;
          while (root->parent != NULL) {
            root = root->parent;
          }
          node->ns = xmlNewNs(root, ns->href, ns->prefix);
        }
      } else {
        node->ns = ns;
      }
    }
    if (n->properties != NULL) {
      node->properties = xmlCopyPropList(node, n->properties);
    }
  }
  return create_node_object(node, m_doc, true);
}

int64 c_domnode::t_getlineno() {
  xmlNodePtr nodep = m_node;
  return xmlGetLineNo(nodep);
}

bool c_domnode::t_hasattributes() {
  xmlNodePtr nodep = m_node;
  if (nodep->type != XML_ELEMENT_NODE) {
    return false;
  }
  return nodep->properties;
}

bool c_domnode::t_haschildnodes() {
  xmlNodePtr nodep = m_node;
  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  return nodep->children;
}

Variant c_domnode::t_insertbefore(CObjRef newnode,
                                  CObjRef refnode /* = null */) {
  xmlNodePtr parentp = m_node;
  if (!dom_node_children_valid(parentp)) {
    return false;
  }
  c_domnode *domchildnode = newnode.getTyped<c_domnode>();
  xmlNodePtr child = domchildnode->m_node;
  xmlNodePtr new_child = NULL;
  int stricterror = m_doc->m_stricterror;
  if (dom_node_is_read_only(parentp) ||
    (child->parent != NULL && dom_node_is_read_only(child->parent))) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  if (!dom_hierarchy(parentp, child)) {
    php_dom_throw_error(HIERARCHY_REQUEST_ERR, stricterror);
    return false;
  }
  if (child->doc != parentp->doc && child->doc != NULL) {
    php_dom_throw_error(WRONG_DOCUMENT_ERR, stricterror);
    return false;
  }
  if (child->type == XML_DOCUMENT_FRAG_NODE && child->children == NULL) {
    raise_warning("Document Fragment is empty");
    return false;
  }
  if (!refnode.isNull()) {
    c_domnode *domrefnode = refnode.getTyped<c_domnode>();
    xmlNodePtr refp = domrefnode->m_node;
    if (refp->parent != parentp) {
      php_dom_throw_error(NOT_FOUND_ERR, stricterror);
      return false;
    }
    if (child->parent != NULL) {
      xmlUnlinkNode(child);
    }
    if (child->type == XML_TEXT_NODE && (refp->type == XML_TEXT_NODE ||
      (refp->prev != NULL && refp->prev->type == XML_TEXT_NODE))) {
      if (child->doc == NULL) {
        xmlSetTreeDoc(child, parentp->doc);
      }
      new_child = child;
      new_child->parent = refp->parent;
      new_child->next = refp;
      new_child->prev = refp->prev;
      refp->prev = new_child;
      if (new_child->prev != NULL) {
        new_child->prev->next = new_child;
      }
      if (new_child->parent != NULL) {
        if (new_child->parent->children == refp) {
          new_child->parent->children = new_child;
        }
      }
    } else if (child->type == XML_ATTRIBUTE_NODE) {
      xmlAttrPtr lastattr;
      if (child->ns == NULL) {
        lastattr = xmlHasProp(refp->parent, child->name);
      } else {
        lastattr = xmlHasNsProp(refp->parent, child->name, child->ns->href);
      }
      if (lastattr != NULL && lastattr->type != XML_ATTRIBUTE_DECL) {
        if (lastattr != (xmlAttrPtr)child) {
          xmlUnlinkNode((xmlNodePtr)lastattr);
          php_libxml_node_free_resource((xmlNodePtr) lastattr);
        } else {
          return create_node_object(child, m_doc, false);
        }
      }
    } else if (child->type == XML_DOCUMENT_FRAG_NODE) {
      new_child = _php_dom_insert_fragment(parentp, refp->prev, refp, child);
    }
    if (new_child == NULL) {
      new_child = xmlAddPrevSibling(refp, child);
    }
  } else {
    if (child->parent != NULL){
      xmlUnlinkNode(child);
    }
    if (child->type == XML_TEXT_NODE && parentp->last != NULL &&
        parentp->last->type == XML_TEXT_NODE) {
      child->parent = parentp;
      if (child->doc == NULL) {
        xmlSetTreeDoc(child, parentp->doc);
      }
      new_child = child;
      if (parentp->children == NULL) {
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
      if (child->ns == NULL)
        lastattr = xmlHasProp(parentp, child->name);
      else
        lastattr = xmlHasNsProp(parentp, child->name, child->ns->href);
      if (lastattr != NULL && lastattr->type != XML_ATTRIBUTE_DECL) {
        if (lastattr != (xmlAttrPtr)child) {
          xmlUnlinkNode((xmlNodePtr)lastattr);
          php_libxml_node_free_resource((xmlNodePtr) lastattr);
        } else {
          return create_node_object(child, m_doc, false);
        }
      }
    } else if (child->type == XML_DOCUMENT_FRAG_NODE) {
      new_child = _php_dom_insert_fragment(parentp, parentp->last, NULL,
                                           child);
    }
    if (new_child == NULL) {
      new_child = xmlAddChild(parentp, child);
    }
  }
  if (NULL == new_child) {
    raise_warning("Couldn't add newnode as the previous sibling of refnode");
    return false;
  }
  domchildnode->m_owner = false;
  dom_reconcile_ns(parentp->doc, new_child);
  return create_node_object(new_child, m_doc, false);
}

bool c_domnode::t_isdefaultnamespace(CStrRef namespaceuri) {
  xmlNodePtr nodep = m_node;
  xmlNsPtr nsptr;
  if (nodep->type == XML_DOCUMENT_NODE ||
      nodep->type == XML_HTML_DOCUMENT_NODE) {
    nodep = xmlDocGetRootElement((xmlDocPtr)nodep);
  }
  if (nodep && namespaceuri.size() > 0) {
    nsptr = xmlSearchNs(nodep->doc, nodep, NULL);
    if (nsptr && xmlStrEqual(nsptr->href, (xmlChar*)namespaceuri.data())) {
      return true;
    }
  }
  return false;
}

bool c_domnode::t_issamenode(CObjRef node) {
  c_domnode *otherdomnode = node.getTyped<c_domnode>();
  return m_node == otherdomnode->m_node;
}

bool c_domnode::t_issupported(CStrRef feature, CStrRef version) {
  return dom_has_feature(feature.data(), version.data());
}

Variant c_domnode::t_lookupnamespaceuri(CStrRef namespaceuri) {
  xmlNodePtr nodep = m_node;
  xmlNsPtr nsptr;
  if (nodep->type == XML_DOCUMENT_NODE ||
      nodep->type == XML_HTML_DOCUMENT_NODE) {
    nodep = xmlDocGetRootElement((xmlDocPtr) nodep);
    if (nodep == NULL) {
      return null;
    }
  }
  nsptr = xmlSearchNs(nodep->doc, nodep, (xmlChar*)namespaceuri.data());
  if (nsptr && nsptr->href != NULL) {
    return String((char *)nsptr->href, CopyString);
  }
  return null;
}

Variant c_domnode::t_lookupprefix(CStrRef prefix) {
  xmlNodePtr nodep = m_node;
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
      return null;
    default:
      lookupp = nodep->parent;
    }
    if (lookupp != NULL &&
        (nsptr = xmlSearchNsByHref(lookupp->doc, lookupp,
                                   (xmlChar*)prefix.data()))) {
      if (nsptr->prefix != NULL) {
        return String((char *)nsptr->prefix, CopyString);
      }
    }
  }
  return null;
}

void c_domnode::t_normalize() {
  dom_normalize(m_node);
}

Variant c_domnode::t_removechild(CObjRef node) {
  xmlNodePtr children;
  xmlNodePtr nodep = m_node;
  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  c_domnode *domnode2 = node.getTyped<c_domnode>();
  xmlNodePtr child = domnode2->m_node;
  int stricterror = m_doc->m_stricterror;
  if (dom_node_is_read_only(nodep) ||
    (child->parent != NULL && dom_node_is_read_only(child->parent))) {
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
      return create_node_object(child, m_doc, true);
    }
    children = children->next;
  }
  php_dom_throw_error(NOT_FOUND_ERR, stricterror);
  return false;
}

Variant c_domnode::t_replacechild(CObjRef newchildobj, CObjRef oldchildobj) {
  int foundoldchild = 0;
  xmlNodePtr nodep = m_node;
  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  c_domnode *domnewchildnode = newchildobj.getTyped<c_domnode>();
  xmlNodePtr newchild = domnewchildnode->m_node;
  c_domnode *domoldchildnode = oldchildobj.getTyped<c_domnode>();
  xmlNodePtr oldchild = domoldchildnode->m_node;
  xmlNodePtr children = nodep->children;
  if (!children) {
    return false;
  }
  int stricterror = m_doc->m_stricterror;
  if (dom_node_is_read_only(nodep) ||
    (newchild->parent != NULL && dom_node_is_read_only(newchild->parent))) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return false;
  }
  if (newchild->doc != nodep->doc && newchild->doc != NULL) {
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
    xmlNodePtr node;
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
      if (newchild->doc == NULL && nodep->doc != NULL) {
        xmlSetTreeDoc(newchild, nodep->doc);
      }
      node = xmlReplaceNode(oldchild, newchild);
      dom_reconcile_ns(nodep->doc, newchild);
    }
    return create_node_object(oldchild, m_doc, false);
  }

  php_dom_throw_error(NOT_FOUND_ERR, m_doc->m_stricterror);
  return false;
}

Variant c_domnode::t_c14n(bool exclusive /* = false */,
                          bool with_comments /* = false */,
                          CVarRef xpath /* = null */,
                          CVarRef ns_prefixes /* = null */) {
  return dom_canonicalization(m_node, "", exclusive, with_comments,
                              xpath, ns_prefixes, 0);
}

Variant c_domnode::t_c14nfile(CStrRef uri, bool exclusive /* = false */,
                              bool with_comments /* = false */,
                              CVarRef xpath /* = null */,
                              CVarRef ns_prefixes /* = null */) {
  return dom_canonicalization(m_node, uri, exclusive, with_comments,
                              xpath, ns_prefixes, 1);
}

Variant c_domnode::t_getnodepath() {
  xmlNodePtr n = m_node;
  char *value = (char*)xmlGetNodePath(n);
  if (value) {
    String ret(value, CopyString);
    xmlFree(value);
    return ret;
  }
  return null;
}

///////////////////////////////////////////////////////////////////////////////

#define CHECK_ATTR(attrp)                               \
  c_domattr *domattr = obj.getTyped<c_domattr>();       \
  xmlAttrPtr attrp = (xmlAttrPtr)domattr->m_node;       \
  if (attrp == NULL) {                                  \
    php_dom_throw_error(INVALID_STATE_ERR, 0);          \
    return null;                                        \
  }                                                     \

#define CHECK_WRITE_ATTR(attrp)                         \
  c_domattr *domattr = obj.getTyped<c_domattr>();       \
  xmlAttrPtr attrp = (xmlAttrPtr)domattr->m_node;       \
  if (attrp == NULL) {                                  \
    php_dom_throw_error(INVALID_STATE_ERR, 0);          \
    return;                                             \
  }                                                     \

static Variant domattr_name_read(CObjRef obj) {
  CHECK_ATTR(attrp);
  return String((char *)(attrp->name), CopyString);
}

static Variant domattr_specified_read(CObjRef obj) {
  /* TODO */
  return true;
}

static Variant domattr_value_read(CObjRef obj) {
  CHECK_ATTR(attrp);
  xmlChar *content = xmlNodeGetContent((xmlNodePtr) attrp);
  if (content) {
    String ret((const char*)content, CopyString);
    xmlFree(content);
    return ret;
  }
  return "";
}

static void domattr_value_write(CObjRef obj, CVarRef value) {
  CHECK_WRITE_ATTR(attrp);
  if (attrp->children) {
    node_list_unlink(attrp->children);
  }
  String svalue = value.toString();
  xmlNodeSetContentLen((xmlNodePtr)attrp, (xmlChar*)svalue.data(),
                       svalue.size() + 1);
}

static Variant domattr_ownerelement_read(CObjRef obj) {
  CHECK_NODE(nodep);
  return create_node_object(nodep->parent, domnode->m_doc);
}

static Variant domattr_schematypeinfo_read(CObjRef obj) {
  raise_warning("Not yet implemented");
  return null;
}

static PropertyAccessor domattr_properties[] = {
  { "name",           domattr_name_read,           NULL },
  { "specified",      domattr_specified_read,      NULL },
  { "value",          domattr_value_read,          domattr_value_write },
  { "ownerElement",   domattr_ownerelement_read,   NULL },
  { "schemaTypeInfo", domattr_schematypeinfo_read, NULL },
  { NULL, NULL, NULL}
};
static PropertyAccessorMap domattr_properties_map
((PropertyAccessor*)domattr_properties, &domnode_properties_map);

///////////////////////////////////////////////////////////////////////////////

c_domattr::c_domattr() {
}

c_domattr::~c_domattr() {
}

void c_domattr::t___construct(CStrRef name,
                              CStrRef value /* = null_string */) {
  int name_valid = xmlValidateName((xmlChar *)name.data(), 0);
  if (name_valid != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, 1);
    return;
  }
  m_node = (xmlNodePtr)xmlNewProp(NULL, (xmlChar*)name.data(),
                                  (xmlChar*)value.data());
  if (!m_node) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

Variant c_domattr::t___destruct() {
  return null;
}

Variant c_domattr::t___get(Variant name) {
  return domattr_properties_map.getter(name)(this);
}

Variant c_domattr::t___set(Variant name, Variant value) {
  domattr_properties_map.setter(name)(this, value);
  return null;
}

bool c_domattr::t_isid() {
  xmlAttrPtr attrp = (xmlAttrPtr)m_node;
  return attrp->atype == XML_ATTRIBUTE_ID;
}

///////////////////////////////////////////////////////////////////////////////

static Variant dom_characterdata_data_read(CObjRef obj) {
  CHECK_NODE(nodep);
  xmlChar *content = xmlNodeGetContent(nodep);
  if (content) {
    String ret((const char*)content, CopyString);
    xmlFree(content);
    return ret;
  }
  return "";
}

static void dom_characterdata_data_write(CObjRef obj, CVarRef value) {
  CHECK_WRITE_NODE(nodep);
  String svalue = value.toString();
  xmlNodeSetContentLen(nodep, (xmlChar*)svalue.data(), svalue.size() + 1);
}

static Variant dom_characterdata_length_read(CObjRef obj) {
  CHECK_NODE(nodep);
  int64 length = 0;
  xmlChar *content = xmlNodeGetContent(nodep);
  if (content) {
    length = xmlUTF8Strlen(content);
    xmlFree(content);
  }
  return length;
}

static PropertyAccessor domcharacterdata_properties[] = {
  { "data",   dom_characterdata_data_read,   dom_characterdata_data_write },
  { "length", dom_characterdata_length_read, NULL },
  { NULL, NULL, NULL}
};
static PropertyAccessorMap domcharacterdata_properties_map
((PropertyAccessor*)domcharacterdata_properties, &domnode_properties_map);

///////////////////////////////////////////////////////////////////////////////

c_domcharacterdata::c_domcharacterdata() {
}

c_domcharacterdata::~c_domcharacterdata() {
}

void c_domcharacterdata::t___construct() {
}

Variant c_domcharacterdata::t___destruct() {
  return null;
}

Variant c_domcharacterdata::t___get(Variant name) {
  return domcharacterdata_properties_map.getter(name)(this);
}

Variant c_domcharacterdata::t___set(Variant name, Variant value) {
  domcharacterdata_properties_map.setter(name)(this, value);
  return null;
}

bool c_domcharacterdata::t_appenddata(CStrRef arg) {
  xmlNodePtr nodep = m_node;
  // Implement logic from libxml xmlTextConcat to add suport for
  // comments and PI
  if ((nodep->content == (xmlChar *) &(nodep->properties)) ||
      ((nodep->doc != NULL) && (nodep->doc->dict != NULL) &&
       xmlDictOwns(nodep->doc->dict, nodep->content))) {
    nodep->content =
      xmlStrncatNew(nodep->content, (xmlChar*)arg.data(), arg.size());
  } else {
    nodep->content =
      xmlStrncat(nodep->content, (xmlChar*)arg.data(), arg.size());
  }
  nodep->properties = NULL;
  return true;
}

bool c_domcharacterdata::t_deletedata(int64 offset, int64 count) {
  xmlNodePtr node = m_node;
  xmlChar *cur, *substring, *second;
  cur = xmlNodeGetContent(node);
  if (cur == NULL) {
    return false;
  }
  int length = xmlUTF8Strlen(cur);
  if (offset < 0 || count < 0 || offset > length) {
    xmlFree(cur);
    php_dom_throw_error(INDEX_SIZE_ERR, m_doc->m_stricterror);
    return false;
  }
  if (offset > 0) {
    substring = xmlUTF8Strsub(cur, 0, offset);
  } else {
    substring = NULL;
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

bool c_domcharacterdata::t_insertdata(int64 offset, CStrRef data) {
  xmlNodePtr node = m_node;
  xmlChar *cur, *first, *second;
  cur = xmlNodeGetContent(node);
  if (cur == NULL) {
    return false;
  }
  int length = xmlUTF8Strlen(cur);
  if (offset < 0 || offset > length) {
    xmlFree(cur);
    php_dom_throw_error(INDEX_SIZE_ERR, m_doc->m_stricterror);
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

bool c_domcharacterdata::t_replacedata(int64 offset, int64 count,
                                       CStrRef data) {
  xmlNodePtr node = m_node;
  xmlChar *cur, *substring, *second = NULL;
  cur = xmlNodeGetContent(node);
  if (cur == NULL) {
    return false;
  }
  int length = xmlUTF8Strlen(cur);
  if (offset < 0 || count < 0 || offset > length) {
    xmlFree(cur);
    php_dom_throw_error(INDEX_SIZE_ERR, m_doc->m_stricterror);
    return false;
  }
  if (offset > 0) {
    substring = xmlUTF8Strsub(cur, 0, offset);
  } else {
    substring = NULL;
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

String c_domcharacterdata::t_substringdata(int64 offset, int64 count) {
  xmlNodePtr node = m_node;
  xmlChar *cur, *substring;
  cur = xmlNodeGetContent(node);
  if (cur == NULL) {
    return false;
  }
  int length = xmlUTF8Strlen(cur);
  if (offset < 0 || count < 0 || offset > length) {
    xmlFree(cur);
    php_dom_throw_error(INDEX_SIZE_ERR, m_doc->m_stricterror);
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
  return "";
}

///////////////////////////////////////////////////////////////////////////////

c_domcomment::c_domcomment() {
}

c_domcomment::~c_domcomment() {
}

void c_domcomment::t___construct(CStrRef value /* = null_string */) {
  m_node = xmlNewComment((xmlChar *)value.data());
  if (!m_node) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

Variant c_domcomment::t___destruct() {
  return null;
}

///////////////////////////////////////////////////////////////////////////////

static Variant dom_text_whole_text_read(CObjRef obj) {
  CHECK_NODE(node);

  /* Find starting text node */
  while (node->prev && ((node->prev->type == XML_TEXT_NODE) ||
                        (node->prev->type == XML_CDATA_SECTION_NODE))) {
    node = node->prev;
  }

  /* concatenate all adjacent text and cdata nodes */
  xmlChar *wholetext = NULL;
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
  return "";
}

static PropertyAccessor domtext_properties[] = {
  { "wholeText", dom_text_whole_text_read, NULL },
  { NULL, NULL, NULL}
};
static PropertyAccessorMap domtext_properties_map
((PropertyAccessor*)domtext_properties, &domcharacterdata_properties_map);

///////////////////////////////////////////////////////////////////////////////

c_domtext::c_domtext() {
}

c_domtext::~c_domtext() {
}

void c_domtext::t___construct(CStrRef value /* = 'null_string' */) {
  m_node = xmlNewText((xmlChar *)value.data());
  if (!m_node) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

Variant c_domtext::t___destruct() {
  return null;
}

Variant c_domtext::t___get(Variant name) {
  return domtext_properties_map.getter(name)(this);
}

Variant c_domtext::t___set(Variant name, Variant value) {
  domtext_properties_map.setter(name)(this, value);
  return null;
}

bool c_domtext::t_iswhitespaceinelementcontent() {
  return xmlIsBlankNode(m_node);
}

Variant c_domtext::t_splittext(int64 offset) {
  xmlNodePtr node = m_node;
  xmlChar *cur, *first, *second;
  xmlNodePtr nnode;
  if (node->type != XML_TEXT_NODE) {
    return false;
  }
  cur = xmlNodeGetContent(node);
  if (cur == NULL) {
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
  if (nnode == NULL) {
    return false;
  }
  if (node->parent != NULL) {
    nnode->type = XML_ELEMENT_NODE;
    xmlAddNextSibling(node, nnode);
    nnode->type = XML_TEXT_NODE;
  }
  c_domtext *ret = NEW(c_domtext)();
  ret->m_doc = m_doc;
  ret->m_node = nnode;
  ret->m_owner = true;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

c_domcdatasection::c_domcdatasection() {
}

c_domcdatasection::~c_domcdatasection() {
}

void c_domcdatasection::t___construct(CStrRef value) {
  m_node = xmlNewCDataBlock(NULL, (xmlChar *)value.data(), value.size());
  if (!m_node) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

Variant c_domcdatasection::t___destruct() {
  return null;
}

///////////////////////////////////////////////////////////////////////////////

#define CHECK_DOC(docp)                                         \
  c_domdocument *domdoc = obj.getTyped<c_domdocument>();        \
  xmlDocPtr docp = (xmlDocPtr)domdoc->m_node;                   \
  if (docp == NULL) {                                           \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                  \
    return null;                                                \
  }                                                             \

#define CHECK_WRITE_DOC(docp)                                   \
  c_domdocument *domdoc = obj.getTyped<c_domdocument>();        \
  xmlDocPtr docp = (xmlDocPtr)domdoc->m_node;                   \
  if (docp == NULL) {                                           \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                  \
    return;                                                     \
  }                                                             \

static Variant dom_document_doctype_read(CObjRef obj) {
  CHECK_DOC(docp);
  return create_node_object((xmlNodePtr)xmlGetIntSubset(docp), domdoc);
}

static Variant dom_document_implementation_read(CObjRef obj) {
  return NEW(c_domimplementation)();
}

static Variant dom_document_document_element_read(CObjRef obj) {
  CHECK_DOC(docp);
  return create_node_object(xmlDocGetRootElement(docp), domdoc);
}

static Variant dom_document_encoding_read(CObjRef obj) {
  CHECK_DOC(docp);
  char *encoding = (char *) docp->encoding;
  if (encoding) {
    return String(encoding, CopyString);
  }
  return null;
}

static void dom_document_encoding_write(CObjRef obj, CVarRef value) {
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

static Variant dom_document_standalone_read(CObjRef obj) {
  CHECK_DOC(docp);
  return (bool)docp->standalone;
}

static void dom_document_standalone_write(CObjRef obj, CVarRef value) {
  CHECK_WRITE_DOC(docp);
  int64 standalone = value.toInt64();
  if (standalone > 0) {
    docp->standalone = 1;
  } else if (standalone < 0) {
    docp->standalone = -1;
  } else {
    docp->standalone = 0;
  }
}

static Variant dom_document_version_read(CObjRef obj) {
  CHECK_DOC(docp);
  char *version = (char *) docp->version;
  if (version) {
    return String(version, CopyString);
  }
  return null;
}

static void dom_document_version_write(CObjRef obj, CVarRef value) {
  CHECK_WRITE_DOC(docp);
  if (docp->version != NULL) {
    xmlFree((xmlChar *)docp->version);
  }
  String svalue = value.toString();
  docp->version = xmlStrdup((const xmlChar *)svalue.data());
}

#define DOCPROP_READ_WRITE(member, name)                                \
  static Variant dom_document_ ##name## _read(CObjRef obj) {            \
    c_domdocument *domdoc = obj.getTyped<c_domdocument>();              \
    return domdoc->m_ ## member;                                        \
  }                                                                     \
  static void dom_document_ ##name## _write(CObjRef obj,                \
                                            CVarRef value) {            \
    c_domdocument *domdoc = obj.getTyped<c_domdocument>();              \
    domdoc->m_ ## member = value.toBoolean();                           \
  }                                                                     \

DOCPROP_READ_WRITE(stricterror,        strict_error_checking );
DOCPROP_READ_WRITE(formatoutput,       format_output         );
DOCPROP_READ_WRITE(validateonparse,    validate_on_parse     );
DOCPROP_READ_WRITE(resolveexternals,   resolve_externals     );
DOCPROP_READ_WRITE(preservewhitespace, preserve_whitespace   );
DOCPROP_READ_WRITE(recover,            recover               );
DOCPROP_READ_WRITE(substituteentities, substitue_entities    );

static Variant dom_document_document_uri_read(CObjRef obj) {
  CHECK_DOC(docp);
  char *url = (char *)docp->URL;
  if (url) {
    return String(url, CopyString);
  }
  return null;
}

static void dom_document_document_uri_write(CObjRef obj, CVarRef value) {
  CHECK_WRITE_DOC(docp);
  if (docp->URL != NULL) {
    xmlFree((xmlChar *) docp->URL);
  }
  String svalue = value.toString();
  docp->URL = xmlStrdup((const xmlChar *)svalue.data());
}

static Variant dom_document_config_read(CObjRef obj) {
  return null;
}

/* }}} */

static PropertyAccessor domdocument_properties[] = {
  { "doctype",             dom_document_doctype_read,          NULL },
  { "implementation",      dom_document_implementation_read,   NULL },
  { "documentElement",     dom_document_document_element_read, NULL },
  { "actualEncoding",      dom_document_encoding_read,         NULL },
  { "encoding",            dom_document_encoding_read,
    dom_document_encoding_write },
  { "xmlEncoding",         dom_document_encoding_read,         NULL },
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
  { "config",              dom_document_config_read,           NULL },
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
  { NULL, NULL, NULL}
};
static PropertyAccessorMap domdocument_properties_map
((PropertyAccessor*)domdocument_properties, &domnode_properties_map);

///////////////////////////////////////////////////////////////////////////////

c_domdocument::c_domdocument() :
  m_formatoutput(false),
  m_validateonparse(false),
  m_resolveexternals(false),
  m_preservewhitespace(true),
  m_substituteentities(false),
  m_stricterror(true),
  m_recover(false) {
}

c_domdocument::~c_domdocument() {
  if (m_owner && m_node) {
    xmlDocPtr doc = (xmlDocPtr)m_node;
    if (doc->URL) {
      xmlFree((void*)doc->URL);
      doc->URL = NULL;
    }
    xmlFreeDoc(doc);
    m_node = NULL; // so ~c_domnode() won't do anything
  }
}

void c_domdocument::t___construct(CStrRef version /* = null_string */,
                                  CStrRef encoding /* = null_string */) {
  xmlDoc *docp = xmlNewDoc((xmlChar*)version.data());
  if (!docp) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
    return;
  }
  if (encoding.size() > 0) {
    docp->encoding = (const xmlChar*)xmlStrdup((xmlChar*)encoding.data());
  }
  m_node = (xmlNodePtr)docp;
  m_owner = true;
}

Variant c_domdocument::t___destruct() {
  return null;
}

Variant c_domdocument::t___get(Variant name) {
  return domdocument_properties_map.getter(name)(this);
}

Variant c_domdocument::t___set(Variant name, Variant value) {
  domdocument_properties_map.setter(name)(this, value);
  return null;
}

Variant c_domdocument::t_createattribute(CStrRef name) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  if (xmlValidateName((xmlChar*)name.data(), 0) != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, m_doc->m_stricterror);
    return false;
  }
  xmlAttrPtr node = xmlNewDocProp(docp, (xmlChar*)name.data(), NULL);
  if (!node) {
    return false;
  }
  c_domattr *ret = NEW(c_domattr)();
  ret->m_doc = this;
  ret->m_node = (xmlNodePtr)node;
  ret->m_owner = true;
  return ret;
}

Variant c_domdocument::t_createattributens(CStrRef namespaceuri,
                                           CStrRef qualifiedname) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNodePtr nodep = NULL;
  xmlNsPtr nsptr;
  char *localname = NULL, *prefix = NULL;
  int errorcode;
  xmlNodePtr root = xmlDocGetRootElement(docp);
  if (root != NULL) {
    errorcode = dom_check_qname((char*)qualifiedname.data(), &localname,
                                &prefix, namespaceuri.size(),
                                qualifiedname.size());
    if (errorcode == 0) {
      if (xmlValidateName((xmlChar*)localname, 0) == 0) {
        nodep = (xmlNodePtr)xmlNewDocProp(docp, (xmlChar*)localname, NULL);
        if (nodep != NULL && namespaceuri.size() > 0) {
          nsptr = xmlSearchNsByHref(nodep->doc, root,
                                    (xmlChar*)namespaceuri.data());
          if (nsptr == NULL) {
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
  if (prefix != NULL) {
    xmlFree(prefix);
  }
  if (errorcode != 0) {
    if (nodep != NULL) {
      xmlFreeProp((xmlAttrPtr) nodep);
    }
    php_dom_throw_error((dom_exception_code)errorcode, m_doc->m_stricterror);
    return false;
  }
  if (nodep == NULL) {
    return false;
  }
  c_domattr *ret = NEW(c_domattr)();
  ret->m_doc = this;
  ret->m_node = nodep;
  ret->m_owner = true;
  return ret;
}

Variant c_domdocument::t_createcdatasection(CStrRef data) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNode *node = xmlNewCDataBlock(docp, (xmlChar*)data.data(), data.size());
  if (!node) {
    return false;
  }
  c_domcdatasection *ret = NEW(c_domcdatasection)();
  ret->m_doc = this;
  ret->m_node = node;
  ret->m_owner = true;
  return ret;
}

Variant c_domdocument::t_createcomment(CStrRef data) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNode *node = xmlNewCDataBlock(docp, (xmlChar*)data.data(), data.size());
  if (!node) {
    return false;
  }
  c_domcomment *ret = NEW(c_domcomment)();
  ret->m_doc = this;
  ret->m_node = node;
  ret->m_owner = true;
  return ret;
}

Variant c_domdocument::t_createdocumentfragment() {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNode *node = xmlNewDocFragment(docp);
  if (!node) {
    return false;
  }
  c_domdocumentfragment *ret = NEW(c_domdocumentfragment)();
  ret->m_doc = this;
  ret->m_node = node;
  ret->m_owner = true;
  return ret;
}

Variant c_domdocument::t_createelement(CStrRef name,
                                       CStrRef value /* = null_string */) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  if (xmlValidateName((xmlChar*)name.data(), 0) != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, m_doc->m_stricterror);
    return false;
  }
  xmlNode *node = xmlNewDocNode(docp, NULL, (xmlChar*)name.data(),
                                (xmlChar*)getStringData(value));
  if (!node) {
    return false;
  }
  c_domelement *ret = NEW(c_domelement)();
  ret->m_doc = this;
  ret->m_node = node;
  ret->m_owner = true;
  return ret;
}

Variant c_domdocument::t_createelementns(CStrRef namespaceuri,
                                         CStrRef qualifiedname,
                                         CStrRef value /* = null_string */) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNodePtr nodep = NULL;
  xmlNsPtr nsptr = NULL;
  char *localname = NULL, *prefix = NULL;
  int errorcode = dom_check_qname((char*)qualifiedname.data(), &localname,
                                  &prefix, namespaceuri.size(),
                                  qualifiedname.size());
  if (errorcode == 0) {
    if (xmlValidateName((xmlChar*)localname, 0) == 0) {
      nodep = xmlNewDocNode(docp, NULL, (xmlChar*)localname,
                            (xmlChar*)getStringData(value));
      if (nodep != NULL && !namespaceuri.isNull()) {
        nsptr = xmlSearchNsByHref(nodep->doc, nodep,
                                  (xmlChar*)namespaceuri.data());
        if (nsptr == NULL) {
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
  if (prefix != NULL) {
    xmlFree(prefix);
  }
  if (errorcode != 0) {
    if (nodep != NULL) {
      xmlFreeNode(nodep);
    }
    php_dom_throw_error((dom_exception_code)errorcode, m_doc->m_stricterror);
    return false;
  }
  if (nodep == NULL) {
    return false;
  }
  nodep->ns = nsptr;
  c_domelement *ret = NEW(c_domelement)();
  ret->m_doc = this;
  ret->m_node = nodep;
  ret->m_owner = true;
  return ret;
}

Variant c_domdocument::t_createentityreference(CStrRef name) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  if (xmlValidateName((xmlChar*)name.data(), 0) != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, m_doc->m_stricterror);
    return false;
  }
  xmlNode *node = xmlNewReference(docp, (xmlChar*)name.data());
  if (!node) {
    return false;
  }
  c_domentity *ret = NEW(c_domentity)();
  ret->m_doc = this;
  ret->m_node = node;
  ret->m_owner = true;
  return ret;
}

Variant c_domdocument::t_createprocessinginstruction(CStrRef target,
                                                     CStrRef data
                                                     /* = null_string */) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  if (xmlValidateName((xmlChar*)target.data(), 0) != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, m_doc->m_stricterror);
    return false;
  }
  xmlNode *node = xmlNewPI((xmlChar*)target.data(),
                           (xmlChar*)getStringData(data));
  if (!node) {
    return false;
  }
  node->doc = docp;
  c_domprocessinginstruction *ret = NEW(c_domprocessinginstruction)();
  ret->m_doc = this;
  ret->m_node = node;
  ret->m_owner = true;
  return ret;
}

Variant c_domdocument::t_createtextnode(CStrRef data) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNode *node = xmlNewDocText(docp, (xmlChar*)data.data());
  if (!node) {
    return false;
  }
  c_domtext *ret = NEW(c_domtext)();
  ret->m_doc = this;
  ret->m_node = node;
  ret->m_owner = true;
  return ret;
}

Variant c_domdocument::t_getelementbyid(CStrRef elementid) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlAttrPtr attrp = xmlGetID(docp, (xmlChar*)elementid.data());
  if (attrp && attrp->parent) {
    c_domtext *ret = NEW(c_domtext)();
    ret->m_doc = this;
    ret->m_node = attrp->parent;
    return ret;
  }
  return null;
}

Variant c_domdocument::t_getelementsbytagname(CStrRef name) {
  c_domnodelist *ret = NEW(c_domnodelist)();
  ret->m_doc = this;
  ret->m_baseobj = this;
  ret->m_nodetype = 0;
  ret->m_local = name;
  return ret;
}

Variant c_domdocument::t_getelementsbytagnamens(CStrRef namespaceuri,
                                                CStrRef localname) {
  c_domnodelist *ret = NEW(c_domnodelist)();
  ret->m_doc = this;
  ret->m_baseobj = this;
  ret->m_nodetype = 0;
  ret->m_local = localname;
  ret->m_ns = namespaceuri;
  return ret;
}

Variant c_domdocument::t_importnode(CObjRef importednode,
                                    bool deep /* = false */) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  c_domnode *domnode = importednode.getTyped<c_domnode>();
  xmlNodePtr nodep = domnode->m_node;
  xmlNodePtr retnodep;
  long recursive = deep ? 1 : 0;
  if (nodep->type == XML_HTML_DOCUMENT_NODE ||
      nodep->type == XML_DOCUMENT_NODE ||
      nodep->type == XML_DOCUMENT_TYPE_NODE) {
    raise_warning("Cannot import: Node Type Not Supported");
    return false;
  }
  bool owner = false;
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
    owner = true;
  }
  return create_node_object(retnodep, m_doc, owner);
}

Variant c_domdocument::t_load(CStrRef filename, int64 options /* = 0 */) {
  String translated = File::TranslatePath(filename);
  if (translated.empty()) {
    raise_warning("Unable to read file: %s", filename.data());
    return false;
  }
  return dom_parse_document(this, translated, options, DOM_LOAD_FILE);
}

Variant c_domdocument::t_loadhtml(CStrRef source) {
  return dom_load_html(this, source, DOM_LOAD_STRING);
}

Variant c_domdocument::t_loadhtmlfile(CStrRef filename) {
  String translated = File::TranslatePath(filename);
  if (translated.empty()) {
    raise_warning("Unable to read file: %s", filename.data());
    return false;
  }
  return dom_load_html(this, translated, DOM_LOAD_FILE);
}

Variant c_domdocument::t_loadxml(CStrRef source, int64 options /* = 0 */) {
  return dom_parse_document(this, source, options, DOM_LOAD_STRING);
}

void c_domdocument::t_normalizedocument() {
  dom_normalize(m_node);
}

bool c_domdocument::t_registernodeclass(CStrRef baseclass,
                                        CStrRef extendedclass) {
  if (!f_class_exists(baseclass)) {
    raise_error("Class %s does not exist", baseclass.data());
    return false;
  }
  if (!f_is_subclass_of(baseclass, "DOMNode")) {
    raise_error("Class %s is not derived from DOMNode.", baseclass.data());
    return false;
  }
  if (!f_class_exists(extendedclass)) {
    raise_error("Class %s does not exist", extendedclass.data());
    return false;
  }
  if (!f_is_subclass_of(extendedclass, baseclass)) {
    raise_error("Class %s is not derived from %s.", extendedclass.data(),
                baseclass.data());
    return false;
  }
  m_classmap.set(StringUtil::ToLower(baseclass), extendedclass);
  return true;
}

bool c_domdocument::t_relaxngvalidate(CStrRef filename) {
  return _dom_document_relaxNG_validate(this, filename, DOM_LOAD_FILE);
}

bool c_domdocument::t_relaxngvalidatesource(CStrRef source) {
  return _dom_document_relaxNG_validate(this, source, DOM_LOAD_STRING);
}

Variant c_domdocument::t_save(CStrRef file, int64 options /* = 0 */) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  int bytes, format = 0, saveempty = 0;

  String translated = File::TranslatePath(file);
  if (translated.empty()) {
    raise_warning("Invalid Filename");
    return false;
  }

  /* encoding handled by property on doc */
  format = m_formatoutput;
  if (options & LIBXML_SAVE_NOEMPTYTAG) {
    saveempty = xmlSaveNoEmptyTags;
    xmlSaveNoEmptyTags = 1;
  }
  bytes = xmlSaveFormatFileEnc(translated.data(), docp, NULL, format);
  if (options & LIBXML_SAVE_NOEMPTYTAG) {
    xmlSaveNoEmptyTags = saveempty;
  }
  if (bytes == -1) {
    return false;
  }
  return bytes;
}

Variant c_domdocument::t_savehtml() {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlChar *mem;
  int size;
  htmlDocDumpMemory(docp, &mem, &size);
  if (!size) {
    if (mem) xmlFree(mem);
    return false;
  }
  String ret = String((char*)mem, size, CopyString);
  xmlFree(mem);
  return ret;
}

Variant c_domdocument::t_savehtmlfile(CStrRef file) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  int bytes, format = 0;

  String translated = File::TranslatePath(file);
  if (translated.empty()) {
    raise_warning("Invalid Filename");
    return false;
  }
  /* encoding handled by property on doc */
  format = m_formatoutput;
  bytes = htmlSaveFileFormat(translated.data(), docp, NULL, format);
  if (bytes == -1) {
    return false;
  }
  return bytes;
}

Variant c_domdocument::t_savexml(CObjRef node /* = null_object */,
                                 int64 options /* = 0 */) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlBufferPtr buf;
  xmlChar *mem;
  int size, format = 0, saveempty = 0;

  format = m_formatoutput;

  if (!node.isNull()) {
    c_domnode *domnode = node.getTyped<c_domnode>();
    xmlNodePtr node = domnode->m_node;
    /* Dump contents of Node */
    if (node->doc != docp) {
      php_dom_throw_error(WRONG_DOCUMENT_ERR, m_doc->m_stricterror);
      return false;
    }
    buf = xmlBufferCreate();
    if (!buf) {
      raise_warning("Could not fetch buffer");
      return false;
    }
    if (options & LIBXML_SAVE_NOEMPTYTAG) {
      saveempty = xmlSaveNoEmptyTags;
      xmlSaveNoEmptyTags = 1;
    }
    xmlNodeDump(buf, docp, node, 0, format);
    if (options & LIBXML_SAVE_NOEMPTYTAG) {
      xmlSaveNoEmptyTags = saveempty;
    }
    mem = (xmlChar*)xmlBufferContent(buf);
    if (!mem) {
      xmlBufferFree(buf);
      return false;
    }
    String ret = String((char*)mem, CopyString);
    xmlBufferFree(buf);
    return ret;
  } else {
    if (options & LIBXML_SAVE_NOEMPTYTAG) {
      saveempty = xmlSaveNoEmptyTags;
      xmlSaveNoEmptyTags = 1;
    }
    // Encoding is handled from the encoding property set on the document
    xmlDocDumpFormatMemory(docp, &mem, &size, format);
    if (options & LIBXML_SAVE_NOEMPTYTAG) {
      xmlSaveNoEmptyTags = saveempty;
    }
    if (!size) {
      return false;
    }
    String ret = String((char*)mem, size, CopyString);
    xmlFree(mem);
    return ret;
  }
}

bool c_domdocument::t_schemavalidate(CStrRef filename) {
  return _dom_document_schema_validate(this, filename, DOM_LOAD_FILE);
}

bool c_domdocument::t_schemavalidatesource(CStrRef source) {
  return _dom_document_schema_validate(this, source, DOM_LOAD_STRING);
}

bool c_domdocument::t_validate() {
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlValidCtxt *cvp;
  if (docp->intSubset == NULL) {
    raise_notice("No DTD given in XML-Document");
  }
  cvp = xmlNewValidCtxt();
  cvp->userData = NULL;
  cvp->error    = (xmlValidityErrorFunc) php_libxml_ctx_error;
  cvp->warning  = (xmlValidityErrorFunc) php_libxml_ctx_error;
  bool ret = xmlValidateDocument(cvp, docp);
  xmlFreeValidCtxt(cvp);
  return ret;
}

Variant c_domdocument::t_xinclude(int64 options /* = 0 */) {
  xmlDocPtr docp = (xmlDocPtr)m_node;
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

c_domdocumentfragment::c_domdocumentfragment() {
}

c_domdocumentfragment::~c_domdocumentfragment() {
}

void c_domdocumentfragment::t___construct() {
  m_node = xmlNewDocFragment(NULL);
  if (!m_node) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

Variant c_domdocumentfragment::t___destruct() {
  return null;
}

bool c_domdocumentfragment::t_appendxml(CStrRef data) {
  xmlNodePtr nodep = m_node;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, m_doc->m_stricterror);
    return false;
  }
  if (!data.empty()) {
    xmlNodePtr lst;
    if (xmlParseBalancedChunkMemory(nodep->doc, NULL, NULL, 0,
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

#define CHECK_DOCTYPE(dtdptr)                                           \
  c_domdocumenttype *domdoctype = obj.getTyped<c_domdocumenttype>();    \
  xmlDtdPtr dtdptr = (xmlDtdPtr)domdoctype->m_node;                     \
  if (dtdptr == NULL) {                                                 \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                          \
    return null;                                                        \
  }                                                                     \

static Variant dom_documenttype_name_read(CObjRef obj) {
  CHECK_DOCTYPE(dtdptr);
  return String((char *)(dtdptr->name), CopyString);
}

static Variant dom_documenttype_entities_read(CObjRef obj) {
  CHECK_DOCTYPE(doctypep);
  c_domnamednodemap *ret = NEW(c_domnamednodemap)();
  ret->m_doc = domdoctype->m_doc;
  ret->m_baseobj = obj;
  ret->m_nodetype = XML_ENTITY_NODE;
  ret->m_ht = (xmlHashTable *) doctypep->entities;
  return ret;
}

static Variant dom_documenttype_notations_read(CObjRef obj) {
  CHECK_DOCTYPE(doctypep);
  c_domnamednodemap *ret = NEW(c_domnamednodemap)();
  ret->m_doc = domdoctype->m_doc;
  ret->m_baseobj = obj;
  ret->m_nodetype = XML_NOTATION_NODE;
  ret->m_ht = (xmlHashTable *) doctypep->notations;
  return ret;
}

static Variant dom_documenttype_public_id_read(CObjRef obj) {
  CHECK_DOCTYPE(dtdptr);
  if (dtdptr->ExternalID) {
    return String((char *)(dtdptr->ExternalID), CopyString);
  }
  return "";
}

static Variant dom_documenttype_system_id_read(CObjRef obj) {
  CHECK_DOCTYPE(dtdptr);
  if (dtdptr->SystemID) {
    return String((char *)(dtdptr->SystemID), CopyString);
  }
  return "";
}

static Variant dom_documenttype_internal_subset_read(CObjRef obj) {
  CHECK_DOCTYPE(dtdptr);

  xmlDtd *intsubset;
  xmlOutputBuffer *buff = NULL;
  xmlChar *strintsubset;
  if (dtdptr->doc != NULL && ((intsubset = dtdptr->doc->intSubset) != NULL)) {
    buff = xmlAllocOutputBuffer(NULL);
    if (buff != NULL) {
      xmlNodeDumpOutput (buff, NULL, (xmlNodePtr) intsubset, 0, 0, NULL);
      xmlOutputBufferFlush(buff);
      strintsubset = xmlStrndup(buff->buffer->content, buff->buffer->use);
      (void)xmlOutputBufferClose(buff);
      return String((char *)strintsubset, CopyString);
    }
  }
  return "";
}

static PropertyAccessor domdocumenttype_properties[] = {
  { "name",           dom_documenttype_name_read,            NULL },
  { "entities",       dom_documenttype_entities_read,        NULL },
  { "notations",      dom_documenttype_notations_read,       NULL },
  { "publicId",       dom_documenttype_public_id_read,       NULL },
  { "systemId",       dom_documenttype_system_id_read,       NULL },
  { "internalSubset", dom_documenttype_internal_subset_read, NULL },
  { NULL, NULL, NULL}
};
static PropertyAccessorMap domdocumenttype_properties_map
((PropertyAccessor*)domdocumenttype_properties, &domnode_properties_map);

///////////////////////////////////////////////////////////////////////////////

c_domdocumenttype::c_domdocumenttype() {
}

c_domdocumenttype::~c_domdocumenttype() {
}

void c_domdocumenttype::t___construct() {
}

Variant c_domdocumenttype::t___destruct() {
  return null;
}

Variant c_domdocumenttype::t___get(Variant name) {
  return domdocumenttype_properties_map.getter(name)(this);
}

Variant c_domdocumenttype::t___set(Variant name, Variant value) {
  domdocumenttype_properties_map.setter(name)(this, value);
  return null;
}

///////////////////////////////////////////////////////////////////////////////

static Variant dom_element_tag_name_read(CObjRef obj) {
  CHECK_NODE(nodep);
  xmlChar *qname;
  xmlNsPtr ns = nodep->ns;
  if (ns != NULL && ns->prefix) {
    qname = xmlStrdup(ns->prefix);
    qname = xmlStrcat(qname, (xmlChar *)":");
    qname = xmlStrcat(qname, nodep->name);
    String ret((char *)qname, CopyString);
    xmlFree(qname);
    return ret;
  }
  return String((char *)nodep->name, CopyString);
}

static Variant dom_element_schema_type_info_read(CObjRef obj) {
  return null;
}

static PropertyAccessor domelement_properties[] = {
  { "tagName",        dom_element_tag_name_read,         NULL},
  { "schemaTypeInfo", dom_element_schema_type_info_read, NULL},
  { NULL, NULL, NULL}
};
static PropertyAccessorMap domelement_properties_map
((PropertyAccessor*)domelement_properties, &domnode_properties_map);

///////////////////////////////////////////////////////////////////////////////

c_domelement::c_domelement() {
}

c_domelement::~c_domelement() {
}

void c_domelement::t___construct(CStrRef name,
                                 CStrRef value /* = null_string */,
                                 CStrRef namespaceuri /* = null_string */) {
  xmlNodePtr nodep = NULL;
  char *localname = NULL, *prefix = NULL;
  int errorcode = 0;
  xmlNsPtr nsptr = NULL;

  int name_valid = xmlValidateName((xmlChar *) name.data(), 0);
  if (name_valid != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, 1);
    return;
  }

  /* Namespace logic is seperate and only when uri passed in to insure
     no BC breakage */
  if (!namespaceuri.empty()) {
    errorcode = dom_check_qname(name.data(), &localname, &prefix,
                                namespaceuri.size(), name.size());
    if (errorcode == 0) {
      nodep = xmlNewNode (NULL, (xmlChar *)localname);
      if (nodep != NULL && !namespaceuri.empty()) {
        nsptr = dom_get_ns(nodep, namespaceuri.data(), &errorcode, prefix);
        xmlSetNs(nodep, nsptr);
      }
    }
    xmlFree(localname);
    if (prefix != NULL) {
      xmlFree(prefix);
    }
    if (errorcode != 0) {
      if (nodep != NULL) {
        xmlFreeNode(nodep);
      }
      php_dom_throw_error((dom_exception_code)errorcode, 1);
      return;
    }
  } else {
    /* If you don't pass a namespace uri, then you can't set a prefix */
    localname = (char*)xmlSplitQName2((xmlChar *)name.data(),
                                      (xmlChar **)&prefix);
    if (prefix != NULL) {
      xmlFree(localname);
      xmlFree(prefix);
      php_dom_throw_error(NAMESPACE_ERR, 1);
      return;
    }
    nodep = xmlNewNode(NULL, (xmlChar *) name.data());
  }

  if (!nodep) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
    return;
  }

  if (!value.empty()) {
    xmlNodeSetContentLen(nodep, (xmlChar *)value.data(), value.size());
  }
  m_node = nodep;
}

Variant c_domelement::t___destruct() {
  return null;
}

Variant c_domelement::t___get(Variant name) {
  return domelement_properties_map.getter(name)(this);
}

Variant c_domelement::t___set(Variant name, Variant value) {
  domelement_properties_map.setter(name)(this, value);
  return null;
}

String c_domelement::t_getattribute(CStrRef name) {
  xmlNodePtr nodep = m_node;
  xmlChar *value = NULL;
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
  return "";
}

Variant c_domelement::t_getattributenode(CStrRef name) {
  xmlNodePtr nodep = m_node;
  xmlNodePtr attrp;
  attrp = dom_get_dom1_attribute(nodep, (xmlChar*)name.data());
  if (attrp == NULL) {
    return false;
  }
  bool owner = false;
  if (attrp->type == XML_NAMESPACE_DECL) {
    xmlNsPtr curns;
    //xmlNodePtr nsparent;
    //nsparent = attrp->_private;
    curns = xmlNewNs(NULL, attrp->name, NULL);
    if (attrp->children) {
      curns->prefix = xmlStrdup((xmlChar*)attrp->children);
    }
    if (attrp->children) {
      attrp = xmlNewDocNode(nodep->doc, NULL, (xmlChar *) attrp->children,
                            attrp->name);
    } else {
      attrp = xmlNewDocNode(nodep->doc, NULL, (xmlChar *)"xmlns", attrp->name);
    }
    attrp->type = XML_NAMESPACE_DECL;
    //attrp->parent = nsparent;
    attrp->ns = curns;
    owner = true;
  }
  c_domnode *ret = NEW(c_domattr)();
  ret->m_doc = m_doc;
  ret->m_node = (xmlNodePtr)attrp;
  ret->m_owner = owner;
  return ret;
}

Object c_domelement::t_getattributenodens(CStrRef namespaceuri,
                                          CStrRef localname) {
  xmlNodePtr elemp = m_node;
  xmlAttrPtr attrp;
  attrp = xmlHasNsProp(elemp, (xmlChar*)localname.data(),
                       (xmlChar*)namespaceuri.data());
  if (attrp == NULL) {
    return null_object;
  }
  c_domnode *ret = NEW(c_domattr)();
  ret->m_doc = m_doc;
  ret->m_node = (xmlNodePtr)attrp;
  return ret;
}

String c_domelement::t_getattributens(CStrRef namespaceuri,
                                      CStrRef localname) {
  xmlNodePtr elemp = m_node;
  xmlNsPtr nsptr;
  xmlChar *strattr;
  strattr = xmlGetNsProp(elemp, (xmlChar*)localname.data(),
                         (xmlChar*)namespaceuri.data());
  if (strattr != NULL) {
    String ret((char*)strattr, CopyString);
    xmlFree(strattr);
    return ret;
  } else {
    if (xmlStrEqual((xmlChar*)namespaceuri.data(),
                    (xmlChar*)DOM_XMLNS_NAMESPACE)) {
      nsptr = dom_get_nsdecl(elemp, (xmlChar*)localname.data());
      if (nsptr != NULL) {
        return String((char*)nsptr->href, CopyString);
      }
    }
  }
  return "";
}

Object c_domelement::t_getelementsbytagname(CStrRef name) {
  c_domnodelist *ret = NEW(c_domnodelist)();
  ret->m_doc = m_doc;
  ret->m_baseobj = this;
  ret->m_nodetype = 0;
  ret->m_local = name;
  return ret;
}

Object c_domelement::t_getelementsbytagnamens(CStrRef namespaceuri,
                                              CStrRef localname) {
  c_domnodelist *ret = NEW(c_domnodelist)();
  ret->m_doc = m_doc;
  ret->m_baseobj = this;
  ret->m_nodetype = 0;
  ret->m_local = localname;
  ret->m_ns = namespaceuri;
  return ret;
}

bool c_domelement::t_hasattribute(CStrRef name) {
  xmlNodePtr nodep = m_node;
  return dom_get_dom1_attribute(nodep, (xmlChar*)name.data()) != NULL;
}

bool c_domelement::t_hasattributens(CStrRef namespaceuri, CStrRef localname) {
  xmlNodePtr elemp = m_node;
  xmlNs *nsp;
  xmlChar *value = xmlGetNsProp(elemp, (xmlChar*)localname.data(),
                                (xmlChar*)namespaceuri.data());
  if (value != NULL) {
    xmlFree(value);
    return true;
  } else {
    if (xmlStrEqual((xmlChar*)namespaceuri.data(),
                    (xmlChar*)DOM_XMLNS_NAMESPACE)) {
      nsp = dom_get_nsdecl(elemp, (xmlChar*)localname.data());
      if (nsp != NULL) {
        return true;
      }
    }
  }
  return false;
}

bool c_domelement::t_removeattribute(CStrRef name) {
  xmlNodePtr nodep = m_node;
  xmlNodePtr attrp;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, m_doc->m_stricterror);
    return false;
  }
  attrp = dom_get_dom1_attribute(nodep, (xmlChar*)name.data());
  if (attrp == NULL) {
    return false;
  }
  switch (attrp->type) {
  case XML_ATTRIBUTE_NODE:
    node_list_unlink(attrp->children);
    xmlUnlinkNode(attrp);
    xmlFreeProp((xmlAttrPtr)attrp);
    break;
  case XML_NAMESPACE_DECL:
    return false;
  default:
    break;
  }
  return true;
}

Variant c_domelement::t_removeattributenode(CObjRef oldattr) {
  xmlNodePtr nodep = m_node;
  c_domattr *attr = oldattr.getTyped<c_domattr>();
  xmlAttrPtr attrp = (xmlAttrPtr)attr->m_node;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, m_doc->m_stricterror);
    return false;
  }
  if (attrp->type != XML_ATTRIBUTE_NODE || attrp->parent != nodep) {
    php_dom_throw_error(NOT_FOUND_ERR, m_doc->m_stricterror);
    return false;
  }
  xmlUnlinkNode((xmlNodePtr)attrp);
  c_domattr *ret = NEW(c_domattr)();
  ret->m_doc = m_doc;
  ret->m_node = (xmlNodePtr)attrp;
  ret->m_owner = true;
  return ret;
}

Variant c_domelement::t_removeattributens(CStrRef namespaceuri,
                                          CStrRef localname) {
  xmlNodePtr nodep = m_node;
  xmlAttr *attrp;
  xmlNsPtr nsptr;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, m_doc->m_stricterror);
    return null;
  }
  attrp = xmlHasNsProp(nodep, (xmlChar*)localname.data(),
                       (xmlChar*)namespaceuri.data());
  nsptr = dom_get_nsdecl(nodep, (xmlChar*)localname.data());
  if (nsptr != NULL) {
    if (xmlStrEqual((xmlChar*)namespaceuri.data(), nsptr->href)) {
      if (nsptr->href != NULL) {
        xmlFree((char*)nsptr->href);
        nsptr->href = NULL;
      }
      if (nsptr->prefix != NULL) {
        xmlFree((char*)nsptr->prefix);
        nsptr->prefix = NULL;
      }
    } else {
      return null;
    }
  }
  if (attrp && attrp->type != XML_ATTRIBUTE_DECL) {
    node_list_unlink(attrp->children);
    xmlUnlinkNode((xmlNodePtr)attrp);
    xmlFreeProp(attrp);
  }
  return null;
}

Variant c_domelement::t_setattribute(CStrRef name, CStrRef value) {
  xmlNodePtr nodep = m_node;
  xmlNodePtr attr = NULL;
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
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, m_doc->m_stricterror);
    return false;
  }
  attr = dom_get_dom1_attribute(nodep, (xmlChar*)name.data());
  if (attr != NULL) {
    switch (attr->type) {
    case XML_ATTRIBUTE_NODE:
      node_list_unlink(attr->children);
      break;
    case XML_NAMESPACE_DECL:
      return false;
    default:
      break;
    }
  }
  if (xmlStrEqual((xmlChar*)name.data(), (xmlChar*)"xmlns")) {
    if (xmlNewNs(nodep, (xmlChar*)value.data(), NULL)) {
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
  c_domattr *ret = NEW(c_domattr)();
  ret->m_doc = m_doc;
  ret->m_node = (xmlNodePtr)attr;
  return ret;
}

Variant c_domelement::t_setattributenode(CObjRef newattr) {
  xmlNodePtr nodep = m_node;
  c_domattr *domattr = newattr.getTyped<c_domattr>();
  xmlAttrPtr attrp = (xmlAttrPtr)domattr->m_node;
  xmlAttr *existattrp = NULL;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, m_doc->m_stricterror);
    return false;
  }
  if (attrp->type != XML_ATTRIBUTE_NODE) {
    raise_warning("Attribute node is required");
    return false;
  }
  if (!(attrp->doc == NULL || attrp->doc == nodep->doc)) {
    php_dom_throw_error(WRONG_DOCUMENT_ERR, m_doc->m_stricterror);
    return false;
  }
  existattrp = xmlHasProp(nodep, attrp->name);
  if (existattrp != NULL && existattrp->type != XML_ATTRIBUTE_DECL) {
    xmlUnlinkNode((xmlNodePtr)existattrp);
  }
  if (attrp->parent != NULL) {
    xmlUnlinkNode((xmlNodePtr)attrp);
  }
  xmlAddChild(nodep, (xmlNodePtr)attrp);
  /* Returns old property if removed otherwise NULL */
  if (existattrp != NULL) {
    c_domattr *ret = NEW(c_domattr)();
    ret->m_doc = m_doc;
    ret->m_node = (xmlNodePtr)existattrp;
    return ret;
  }
  return null;
}

Variant c_domelement::t_setattributenodens(CObjRef newattr) {
  xmlNs *nsp;
  xmlAttr *existattrp = NULL;
  xmlNodePtr nodep = m_node;
  c_domattr *domattr = newattr.getTyped<c_domattr>();
  xmlAttrPtr attrp = (xmlAttrPtr)domattr->m_node;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, m_doc->m_stricterror);
    return false;
  }
  if (attrp->type != XML_ATTRIBUTE_NODE) {
    raise_warning("Attribute node is required");
    return false;
  }
  if (!(attrp->doc == NULL || attrp->doc == nodep->doc)) {
    php_dom_throw_error(WRONG_DOCUMENT_ERR, m_doc->m_stricterror);
    return false;
  }
  nsp = attrp->ns;
  if (nsp != NULL) {
    existattrp = xmlHasNsProp(nodep, nsp->href, attrp->name);
  } else {
    existattrp = xmlHasProp(nodep, attrp->name);
  }
  if (existattrp != NULL && existattrp->type != XML_ATTRIBUTE_DECL) {
    xmlUnlinkNode((xmlNodePtr)existattrp);
  }
  if (attrp->parent != NULL) {
    xmlUnlinkNode((xmlNodePtr) attrp);
  }
  xmlAddChild(nodep, (xmlNodePtr) attrp);
  /* Returns old property if removed otherwise NULL */
  if (existattrp != NULL) {
    c_domattr *ret = NEW(c_domattr)();
    ret->m_doc = m_doc;
    ret->m_node = (xmlNodePtr)existattrp;
    return ret;
  }
  return null;
}

Variant c_domelement::t_setattributens(CStrRef namespaceuri, CStrRef name,
                                       CStrRef value) {
  xmlNodePtr elemp = m_node;
  xmlNsPtr nsptr;
  xmlNode *nodep;
  xmlAttr *attr;
  char *localname = NULL, *prefix = NULL;
  int errorcode = 0, is_xmlns = 0, name_valid;
  if (name.empty()) {
    raise_warning("Attribute Name is required");
    return false;
  }
  int stricterror = m_doc->m_stricterror;
  if (dom_node_is_read_only(elemp)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror);
    return null;
  }
  errorcode = dom_check_qname((char*)name.data(), &localname, &prefix,
                              namespaceuri.size(), name.size());
  if (errorcode == 0) {
    if (namespaceuri.size() > 0) {
      nodep = (xmlNodePtr)xmlHasNsProp(elemp, (xmlChar*)localname,
                                       (xmlChar*)namespaceuri.data());
      if (nodep != NULL && nodep->type != XML_ATTRIBUTE_DECL) {
        node_list_unlink(nodep->children);
      }
      if (xmlStrEqual((xmlChar*)prefix, (xmlChar*)"xmlns") &&
          xmlStrEqual((xmlChar*)namespaceuri.data(),
                      (xmlChar*)DOM_XMLNS_NAMESPACE)) {
        is_xmlns = 1;
        nsptr = dom_get_nsdecl(elemp, (xmlChar*)localname);
      } else {
        nsptr = xmlSearchNsByHref(elemp->doc, elemp,
                                  (xmlChar*)namespaceuri.data());
        if (nsptr && nsptr->prefix == NULL) {
          xmlNsPtr tmpnsptr;
          tmpnsptr = nsptr->next;
          while (tmpnsptr) {
            if ((tmpnsptr->prefix != NULL) && (tmpnsptr->href != NULL) &&
              (xmlStrEqual(tmpnsptr->href, (xmlChar*)namespaceuri.data()))) {
              nsptr = tmpnsptr;
              break;
            }
            tmpnsptr = tmpnsptr->next;
          }
          if (tmpnsptr == NULL) {
            nsptr = _dom_new_reconNs(elemp->doc, elemp, nsptr);
          }
        }
      }
      if (nsptr == NULL) {
        if (prefix == NULL) {
          errorcode = NAMESPACE_ERR;
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
        attr = xmlSetNsProp(elemp, nsptr, (xmlChar*)localname,
                            (xmlChar*)value.data());
      }
    } else {
      name_valid = xmlValidateName((xmlChar*)localname, 0);
      if (name_valid != 0) {
        errorcode = INVALID_CHARACTER_ERR;
        stricterror = 1;
      } else {
        attr = xmlHasProp(elemp, (xmlChar*)localname);
        if (attr != NULL && attr->type != XML_ATTRIBUTE_DECL) {
          node_list_unlink(attr->children);
        }
        attr = xmlSetProp(elemp, (xmlChar*)localname, (xmlChar*)value.data());
      }
    }
  }
  xmlFree(localname);
  if (prefix != NULL) {
    xmlFree(prefix);
  }
  if (errorcode != 0) {
    php_dom_throw_error((dom_exception_code)errorcode, stricterror);
  }
  return null;
}

Variant c_domelement::t_setidattribute(CStrRef name, bool isid) {
  xmlNodePtr nodep = m_node;
  xmlAttrPtr attrp;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, m_doc->m_stricterror);
    return null;
  }
  attrp = xmlHasNsProp(nodep, (xmlChar*)name.data(), NULL);
  if (attrp == NULL || attrp->type == XML_ATTRIBUTE_DECL) {
    php_dom_throw_error(NOT_FOUND_ERR, m_doc->m_stricterror);
  } else {
    php_set_attribute_id(attrp, isid);
  }
  return null;
}

Variant c_domelement::t_setidattributenode(CObjRef idattr, bool isid) {
  xmlNodePtr nodep = m_node;
  c_domattr *domattr = idattr.getTyped<c_domattr>();
  xmlAttrPtr attrp = (xmlAttrPtr)domattr->m_node;
  if (dom_node_is_read_only(nodep)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, m_doc->m_stricterror);
    return null;
  }
  if (attrp->parent != nodep) {
    php_dom_throw_error(NOT_FOUND_ERR, m_doc->m_stricterror);
  } else {
    php_set_attribute_id(attrp, isid);
  }
  return null;
}

Variant c_domelement::t_setidattributens(CStrRef namespaceuri,
                                         CStrRef localname, bool isid) {
  xmlNodePtr elemp = m_node;
  xmlAttrPtr attrp;
  if (dom_node_is_read_only(elemp)) {
    php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, m_doc->m_stricterror);
    return null;
  }
  attrp = xmlHasNsProp(elemp, (xmlChar*)localname.data(),
                       (xmlChar*)namespaceuri.data());
  if (attrp == NULL || attrp->type == XML_ATTRIBUTE_DECL) {
    php_dom_throw_error(NOT_FOUND_ERR, m_doc->m_stricterror);
  } else {
    php_set_attribute_id(attrp, isid);
  }
  return null;
}

///////////////////////////////////////////////////////////////////////////////

#define CHECK_ENTITY(nodep)                             \
  c_domentity *domentity = obj.getTyped<c_domentity>(); \
  xmlEntity *nodep = (xmlEntity*)domentity->m_node;     \
  if (nodep == NULL) {                                  \
    php_dom_throw_error(INVALID_STATE_ERR, 0);          \
    return null;                                        \
  }                                                     \

static Variant dom_entity_public_id_read(CObjRef obj) {
  CHECK_ENTITY(nodep);
  if (nodep->etype != XML_EXTERNAL_GENERAL_UNPARSED_ENTITY) {
    return null;
  }
  return String((char *)(nodep->ExternalID), CopyString);
}

static Variant dom_entity_system_id_read(CObjRef obj) {
  CHECK_ENTITY(nodep);
  if (nodep->etype != XML_EXTERNAL_GENERAL_UNPARSED_ENTITY) {
    return null;
  }
  return String((char *)(nodep->SystemID), CopyString);
}

static Variant dom_entity_notation_name_read(CObjRef obj) {
  CHECK_ENTITY(nodep);
  if (nodep->etype != XML_EXTERNAL_GENERAL_UNPARSED_ENTITY) {
    return null;
  }
  char *content = (char*)xmlNodeGetContent((xmlNodePtr) nodep);
  String ret(content, CopyString);
  xmlFree(content);
  return ret;
}

static Variant dom_entity_actual_encoding_read(CObjRef obj) {
  return null;
}

static void dom_entity_actual_encoding_write(CObjRef obj, CVarRef value) {
  // do nothing
}

static Variant dom_entity_encoding_read(CObjRef obj) {
  return null;
}

static void dom_entity_encoding_write(CObjRef obj, CVarRef value) {
  // do nothing
}

static Variant dom_entity_version_read(CObjRef obj) {
  return null;
}

static void dom_entity_version_write(CObjRef obj, CVarRef value) {
  // do nothing
}

static PropertyAccessor domentity_properties[] = {
 { "publicId",       dom_entity_public_id_read,       NULL },
 { "systemId",       dom_entity_system_id_read,       NULL },
 { "notationName",   dom_entity_notation_name_read,   NULL },
 { "actualEncoding", dom_entity_actual_encoding_read,
   dom_entity_actual_encoding_write },
 { "encoding",       dom_entity_encoding_read,
   dom_entity_encoding_write },
 { "version",        dom_entity_version_read,
   dom_entity_version_write },
 { NULL, NULL, NULL}
};
static PropertyAccessorMap domentity_properties_map
((PropertyAccessor*)domentity_properties, &domnode_properties_map);

///////////////////////////////////////////////////////////////////////////////

c_domentity::c_domentity() {
}

c_domentity::~c_domentity() {
}

void c_domentity::t___construct() {
}

Variant c_domentity::t___destruct() {
  return null;
}

Variant c_domentity::t___get(Variant name) {
  return domentity_properties_map.getter(name)(this);
}

Variant c_domentity::t___set(Variant name, Variant value) {
  domentity_properties_map.setter(name)(this, value);
  return null;
}

///////////////////////////////////////////////////////////////////////////////

c_domentityreference::c_domentityreference() {
}

c_domentityreference::~c_domentityreference() {
}

void c_domentityreference::t___construct(CStrRef name) {
  int name_valid = xmlValidateName((xmlChar *)name.data(), 0);
  if (name_valid != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, 1);
    return;
  }

  m_node = xmlNewReference(NULL, (xmlChar*)name.data());
  if (!m_node) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

Variant c_domentityreference::t___destruct() {
  return null;
}

///////////////////////////////////////////////////////////////////////////////

#define CHECK_NOTATION(nodep)                                 \
  c_domnotation *domnotation = obj.getTyped<c_domnotation>(); \
  xmlEntity *nodep = (xmlEntity*)domnotation->m_node;         \
  if (nodep == NULL) {                                        \
    php_dom_throw_error(INVALID_STATE_ERR, 0);                \
    return null;                                              \
  }                                                           \

static Variant dom_notation_public_id_read(CObjRef obj) {
  CHECK_NOTATION(nodep);
  if (nodep->ExternalID) {
    return String((char *)(nodep->ExternalID), CopyString);
  }
  return "";
}

static Variant dom_notation_system_id_read(CObjRef obj) {
  CHECK_NOTATION(nodep);
  if (nodep->SystemID) {
    return String((char *)(nodep->SystemID), CopyString);
  }
  return "";
}

static PropertyAccessor domnotation_properties[] = {
 { "publicId",   dom_notation_public_id_read, NULL },
 { "systemId",   dom_notation_system_id_read, NULL },
 { "nodeName",   domnode_nodename_read,       NULL },
 { "nodeValue",  domnode_nodevalue_read,      domnode_nodevalue_write },
 { "attributes", domnode_attributes_read,     NULL },
 { NULL, NULL, NULL}
};
static PropertyAccessorMap domnotation_properties_map
((PropertyAccessor*)domnotation_properties);

///////////////////////////////////////////////////////////////////////////////

c_domnotation::c_domnotation() {
}

c_domnotation::~c_domnotation() {
}

void c_domnotation::t___construct() {
}

Variant c_domnotation::t___destruct() {
  return null;
}

Variant c_domnotation::t___get(Variant name) {
  return domnotation_properties_map.getter(name)(this);
}

Variant c_domnotation::t___set(Variant name, Variant value) {
  domnotation_properties_map.setter(name)(this, value);
  return null;
}

///////////////////////////////////////////////////////////////////////////////

static Variant dom_processinginstruction_target_read(CObjRef obj) {
  CHECK_NODE(nodep);
  return String((char *)(nodep->name), CopyString);
}

static Variant dom_processinginstruction_data_read(CObjRef obj) {
  CHECK_NODE(nodep);
  xmlChar *content = xmlNodeGetContent(nodep);
  if (content) {
    String ret((char*)content, CopyString);
    xmlFree(content);
    return ret;
  }
  return "";
}

static void dom_processinginstruction_data_write(CObjRef obj, CVarRef value) {
  CHECK_WRITE_NODE(nodep);
  String svalue = value.toString();
  xmlNodeSetContentLen(nodep, (xmlChar*)svalue.data(), svalue.size() + 1);
}

static PropertyAccessor domprocessinginstruction_properties[] = {
  { "target", dom_processinginstruction_target_read, NULL },
  { "data",   dom_processinginstruction_data_read,
    dom_processinginstruction_data_write },
  { NULL, NULL, NULL}
};
static PropertyAccessorMap domprocessinginstruction_properties_map
((PropertyAccessor*)domprocessinginstruction_properties,
 &domnode_properties_map);

///////////////////////////////////////////////////////////////////////////////

c_domprocessinginstruction::c_domprocessinginstruction() {
}

c_domprocessinginstruction::~c_domprocessinginstruction() {
}

void c_domprocessinginstruction::t___construct(CStrRef name,
                                               CStrRef value
                                               /*= null_string*/) {
  int name_valid = xmlValidateName((xmlChar *)name.data(), 0);
  if (name_valid != 0) {
    php_dom_throw_error(INVALID_CHARACTER_ERR, 1);
    return;
  }

  m_node = xmlNewPI((xmlChar *)name.data(), (xmlChar *)value.data());
  if (!m_node) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
  }
}

Variant c_domprocessinginstruction::t___destruct() {
  return null;
}

Variant c_domprocessinginstruction::t___get(Variant name) {
  return domprocessinginstruction_properties_map.getter(name)(this);
}

Variant c_domprocessinginstruction::t___set(Variant name, Variant value) {
  domprocessinginstruction_properties_map.setter(name)(this, value);
  return null;
}

///////////////////////////////////////////////////////////////////////////////

static Variant dom_namednodemap_length_read(CObjRef obj) {
  c_domnamednodemap *objmap = obj.getTyped<c_domnamednodemap>();

  int count = 0;
  if (objmap->m_nodetype == XML_NOTATION_NODE ||
      objmap->m_nodetype == XML_ENTITY_NODE) {
    if (objmap->m_ht) {
      count = xmlHashSize(objmap->m_ht);
    }
  } else {
    xmlNodePtr nodep = objmap->m_baseobj.getTyped<c_domnode>()->m_node;
    if (nodep) {
      xmlAttrPtr curnode = nodep->properties;
      if (curnode) {
        count++;
        while (curnode->next != NULL) {
          count++;
          curnode = curnode->next;
        }
      }
    }
  }
  return count;
}

static PropertyAccessor domnamednodemap_properties[] = {
  { "length", dom_namednodemap_length_read, NULL },
  { NULL, NULL, NULL}
};
static PropertyAccessorMap domnamednodemap_properties_map
((PropertyAccessor*)domnamednodemap_properties);

///////////////////////////////////////////////////////////////////////////////

c_domnamednodemap::c_domnamednodemap() {
}

c_domnamednodemap::~c_domnamednodemap() {
}

void c_domnamednodemap::t___construct() {
}

Variant c_domnamednodemap::t___destruct() {
  return null;
}

Variant c_domnamednodemap::t_getnameditem(CStrRef name) {
  xmlNodePtr itemnode = NULL;
  bool owner = false;
  if (m_nodetype == XML_NOTATION_NODE || m_nodetype == XML_ENTITY_NODE) {
    if (m_ht) {
      if (m_nodetype == XML_ENTITY_NODE) {
        itemnode = (xmlNodePtr)xmlHashLookup(m_ht, (xmlChar*)name.data());
      } else {
        xmlNotation *notep =
          (xmlNotation *)xmlHashLookup(m_ht,
                                       (xmlChar*)name.data());
        if (notep) {
          itemnode = create_notation(notep->name, notep->PublicID,
                                     notep->SystemID);
          owner = true;
        }
      }
    }
  } else {
    xmlNodePtr nodep = m_baseobj.getTyped<c_domnode>()->m_node;
    if (nodep) {
      itemnode = (xmlNodePtr)xmlHasProp(nodep, (xmlChar*)name.data());
    }
  }

  if (itemnode) {
    Variant ret = php_dom_create_object(itemnode, m_doc, owner);
    if (ret.isNull()) {
      raise_warning("Cannot create required DOM object");
      return false;
    }
    return ret;
  }
  return null;
}

Variant c_domnamednodemap::t_getnameditemns(CStrRef namespaceuri,
                                            CStrRef localname) {
  xmlNodePtr itemnode = NULL;
  bool owner = false;
  if (m_nodetype == XML_NOTATION_NODE || m_nodetype == XML_ENTITY_NODE) {
    if (m_ht) {
      if (m_nodetype == XML_ENTITY_NODE) {
        itemnode = (xmlNodePtr)xmlHashLookup(m_ht,
                                             (xmlChar*)localname.data());
      } else {
        xmlNotation *notep =
          (xmlNotation *)xmlHashLookup(m_ht, (xmlChar*)localname.data());
        if (notep) {
          itemnode = create_notation(notep->name, notep->PublicID,
                                     notep->SystemID);
          owner = true;
        }
      }
    }
  } else {
    xmlNodePtr nodep = m_baseobj.getTyped<c_domnode>()->m_node;
    if (nodep) {
      itemnode = (xmlNodePtr)xmlHasNsProp(nodep, (xmlChar*)localname.data(),
                                          (xmlChar*)namespaceuri.data());
    }
  }

  if (itemnode) {
    Variant ret = php_dom_create_object(itemnode, m_doc, owner);
    if (ret.isNull()) {
      raise_warning("Cannot create required DOM object");
      return false;
    }
    return ret;
  }
  return null;
}

Variant c_domnamednodemap::t_item(int64 index) {
  if (index >= 0) {
    xmlNodePtr itemnode = NULL;
    bool owner = false;
    if (m_nodetype == XML_NOTATION_NODE || m_nodetype == XML_ENTITY_NODE) {
      if (m_ht) {
        if (m_nodetype == XML_ENTITY_NODE) {
          itemnode = php_dom_libxml_hash_iter(m_ht, index);
        } else {
          itemnode = php_dom_libxml_notation_iter(m_ht, index);
          owner = true;
        }
      }
    } else {
      xmlNodePtr nodep = m_baseobj.getTyped<c_domnode>()->m_node;
      if (nodep) {
        xmlNodePtr curnode = (xmlNodePtr)nodep->properties;
        int count = 0;
        while (count < index && curnode != NULL) {
          count++;
          curnode = (xmlNodePtr)curnode->next;
        }
        itemnode = curnode;
      }
    }

    if (itemnode) {
      Variant ret = php_dom_create_object(itemnode, m_doc, owner);
      if (ret.isNull()) {
        raise_warning("Cannot create required DOM object");
        return false;
      }
      return ret;
    }
  }

  return null;
}

Variant c_domnamednodemap::t___get(Variant name) {
  return domnamednodemap_properties_map.getter(name)(this);
}

Variant c_domnamednodemap::t___set(Variant name, Variant value) {
  domnamednodemap_properties_map.setter(name)(this, value);
  return null;
}

Variant c_domnamednodemap::t_getiterator() {
  c_domnodeiterator *iter = NEW(c_domnodeiterator)();
  iter->reset_iterator(this);
  return Object(iter);
}

///////////////////////////////////////////////////////////////////////////////

static Variant dom_nodelist_length_read(CObjRef obj) {
  c_domnodelist *objmap = obj.getTyped<c_domnodelist>();

  int count = 0;
  if (objmap->m_ht) {
    count = xmlHashSize(objmap->m_ht);
  } else {
    if (objmap->m_nodetype == DOM_NODESET) {
      count = objmap->m_baseobjptr.size();
    } else {
      xmlNodePtr nodep = objmap->m_baseobj.getTyped<c_domnode>()->m_node;
      if (nodep) {
        if (objmap->m_nodetype == XML_ATTRIBUTE_NODE ||
            objmap->m_nodetype == XML_ELEMENT_NODE) {
          xmlNodePtr curnode = nodep->children;
          if (curnode) {
            count++;
            while (curnode->next != NULL) {
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

static PropertyAccessor domnodelist_properties[] = {
  { "length", dom_nodelist_length_read, NULL },
  { NULL, NULL, NULL}
};
static PropertyAccessorMap domnodelist_properties_map
((PropertyAccessor*)domnodelist_properties);

///////////////////////////////////////////////////////////////////////////////

c_domnodelist::c_domnodelist() {
}

c_domnodelist::~c_domnodelist() {
}

void c_domnodelist::t___construct() {
}

Variant c_domnodelist::t___destruct() {
  return null;
}

Variant c_domnodelist::t___get(Variant name) {
  return domnodelist_properties_map.getter(name)(this);
}

Variant c_domnodelist::t___set(Variant name, Variant value) {
  domnodelist_properties_map.setter(name)(this, value);
  return null;
}

Variant c_domnodelist::t_item(int64 index) {
  xmlNodePtr itemnode = NULL;
  xmlNodePtr nodep, curnode;
  int count = 0;
  bool owner = false;
  if (index >= 0) {
    if (m_ht) {
      if (m_nodetype == XML_ENTITY_NODE) {
        itemnode = php_dom_libxml_hash_iter(m_ht, index);
      } else {
        itemnode = php_dom_libxml_notation_iter(m_ht, index);
        owner = true;
      }
    } else {
      if (m_nodetype == DOM_NODESET) {
        if (m_baseobjptr.exists(index)) {
          return m_baseobjptr[index];
        }
      } else if (!m_baseobj.isNull()) {
        nodep = m_baseobj.getTyped<c_domnode>()->m_node;
        if (nodep) {
          if (m_nodetype == XML_ATTRIBUTE_NODE ||
              m_nodetype == XML_ELEMENT_NODE) {
            curnode = nodep->children;
            while (count < index && curnode != NULL) {
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
            itemnode = dom_get_elements_by_tag_name_ns_raw
              (nodep, m_ns.data(), m_local.data(), &count, index);
          }
        }
      }
    }
    if (itemnode) {
      return create_node_object(itemnode, m_doc, owner);
    }
  }
  return null_object;
}

Variant c_domnodelist::t_getiterator() {
  c_domnodeiterator *iter = NEW(c_domnodeiterator)();
  iter->reset_iterator(this);
  return Object(iter);
}

///////////////////////////////////////////////////////////////////////////////

c_domexception::c_domexception() {
}

c_domexception::~c_domexception() {
}

void c_domexception::t___construct(CStrRef message /* = "" */,
                                   int64 code /* = 0 */) {
  c_exception::t___construct(message, code);
}

Variant c_domexception::t___destruct() {
  return null;
}

///////////////////////////////////////////////////////////////////////////////

c_domimplementation::c_domimplementation() {
}

c_domimplementation::~c_domimplementation() {
}

void c_domimplementation::t___construct() {
}

Variant c_domimplementation::t___destruct() {
  return null;
}

Variant c_domimplementation::t_createdocument
(CStrRef namespaceuri /* = null_string */,
 CStrRef qualifiedname /* = null_string */,
 CObjRef doctypeobj /* = null_object */) {
  xmlDoc *docp;
  xmlNode *nodep;
  xmlNsPtr nsptr = NULL;
  int errorcode = 0;
  char *prefix = NULL, *localname = NULL;
  xmlDtdPtr doctype = NULL;
  if (!doctypeobj.isNull()) {
    c_domdocumenttype *domdoctype = doctypeobj.getTyped<c_domdocumenttype>();
    doctype = (xmlDtdPtr)domdoctype->m_node;
    if (doctype->type == XML_DOCUMENT_TYPE_NODE) {
      raise_warning("Invalid DocumentType object");
      return false;
    }
    if (doctype->doc != NULL) {
      php_dom_throw_error(WRONG_DOCUMENT_ERR, 1);
      return false;
    }
  }
  if (qualifiedname.size() > 0) {
    errorcode = dom_check_qname((char*)qualifiedname.data(), (char**)&localname, (char**)&prefix, 1, qualifiedname.size());
    if (errorcode == 0 && namespaceuri.size() > 0 && ((nsptr = xmlNewNs(NULL, (xmlChar*)namespaceuri.data(), (xmlChar*)prefix)) == NULL)) {
      errorcode = NAMESPACE_ERR;
    }
  }
  if (prefix != NULL) {
    xmlFree(prefix);
  }
  if (errorcode != 0) {
    if (localname != NULL) {
      xmlFree(localname);
    }
    php_dom_throw_error((dom_exception_code)errorcode, 1);
    return false;
  }
  /* currently letting libxml2 set the version string */
  docp = xmlNewDoc(NULL);
  if (!docp) {
    if (localname != NULL) {
      xmlFree(localname);
    }
    return false;
  }
  if (doctype != NULL) {
    docp->intSubset = doctype;
    doctype->parent = docp;
    doctype->doc = docp;
    docp->children = (xmlNodePtr)doctype;
    docp->last = (xmlNodePtr)doctype;
  }
  if (localname != NULL) {
    nodep = xmlNewDocNode(docp, nsptr, (xmlChar*)localname, NULL);
    if (!nodep) {
      if (doctype != NULL) {
        docp->intSubset = NULL;
        doctype->parent = NULL;
        doctype->doc = NULL;
        docp->children = NULL;
        docp->last = NULL;
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
  c_domdocument *ret = NEW(c_domdocument)();
  ret->m_node = (xmlNodePtr)docp;
  ret->m_owner = true;
  return ret;
}

Variant c_domimplementation::t_createdocumenttype
(CStrRef qualifiedname /* = null_string */,
 CStrRef publicid /* = null_string */,
 CStrRef systemid /* = null_string */) {
  xmlDtd *doctype;
  xmlChar *pch1 = NULL, *pch2 = NULL, *localname = NULL;
  xmlURIPtr uri;
  if (qualifiedname.empty()) {
    raise_warning("qualifiedName is required");
    return false;
  }
  if (publicid.size() > 0) {
    pch1 = (xmlChar*)publicid.data();
  }
  if (systemid.size() > 0) {
    pch2 = (xmlChar*)systemid.data();
  }
  uri = xmlParseURI((char*)qualifiedname.data());
  if (uri != NULL && uri->opaque != NULL) {
    localname = xmlStrdup((xmlChar*)uri->opaque);
    if (xmlStrchr(localname, (xmlChar)':') != NULL) {
      php_dom_throw_error(NAMESPACE_ERR, 1);
      xmlFreeURI(uri);
      xmlFree(localname);
      return false;
    }
  } else {
    localname = xmlStrdup((xmlChar*)qualifiedname.data());
  }
  if (uri) {
    xmlFreeURI(uri);
  }
  doctype = xmlCreateIntSubset(NULL, localname, pch1, pch2);
  xmlFree(localname);
  if (doctype == NULL) {
    raise_warning("Unable to create DocumentType");
    return false;
  }
  return create_node_object((xmlNodePtr)doctype, NULL, true);
}

bool c_domimplementation::t_hasfeature(CStrRef feature, CStrRef version) {
  return dom_has_feature(feature.data(), version.data());
}

///////////////////////////////////////////////////////////////////////////////

static Variant dom_xpath_document_read(CObjRef obj) {
  xmlDoc *docp = NULL;
  c_domxpath *xpath = obj.getTyped<c_domxpath>();
  xmlXPathContextPtr ctx = (xmlXPathContextPtr)xpath->m_node;
  if (ctx) {
    docp = (xmlDocPtr)ctx->doc;
  }
  return create_node_object((xmlNodePtr)docp, xpath->m_doc);
}

static PropertyAccessor domxpath_properties[] = {
  { "document", dom_xpath_document_read, NULL },
  { NULL, NULL, NULL}
};
static PropertyAccessorMap domxpath_properties_map
((PropertyAccessor*)domxpath_properties);

///////////////////////////////////////////////////////////////////////////////

static void dom_xpath_ext_function_php(xmlXPathParserContextPtr ctxt,
                                       int nargs, int type) {
  int error = 0;
  c_domxpath *intern = (c_domxpath*) ctxt->context->userData;
  if (intern == NULL) {
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

  Array args;
  /* Reverse order to pop values off ctxt stack */
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
        arg = Array::Create();
        if (obj->nodesetval && obj->nodesetval->nodeNr > 0) {
          for (int j = 0; j < obj->nodesetval->nodeNr; j++) {
            xmlNodePtr node = obj->nodesetval->nodeTab[j];
            /* not sure, if we need this... it's copied from xpath.c */
            if (node->type == XML_NAMESPACE_DECL) {
              xmlNodePtr nsparent = (xmlNodePtr)node->_private;
              xmlNsPtr curns = xmlNewNs(NULL, node->name, NULL);
              if (node->children) {
                curns->prefix = xmlStrdup((xmlChar *)node->children);
              }
              if (node->children) {
                node = xmlNewDocNode(node->doc, NULL,
                                     (xmlChar *)node->children, node->name);
              } else {
                node = xmlNewDocNode(node->doc, NULL,
                                     (xmlChar *)"xmlns", node->name);
              }
              node->type = XML_NAMESPACE_DECL;
              node->parent = nsparent;
              node->ns = curns;
            }
            arg.append(create_node_object(node, intern->m_doc));
          }
        }
      }
      break;
    default:
      arg = String((char *)xmlXPathCastToString(obj), CopyString);
    }
    xmlXPathFreeObject(obj);
    args.set(i, arg);
  }

  obj = valuePop(ctxt);
  if (obj->stringval == NULL) {
    xmlXPathFreeObject(obj);
    raise_warning("Handler name must be a string");
    return;
  }
  String handler((char*)obj->stringval, CopyString);
  xmlXPathFreeObject(obj);

  if (!f_is_callable(handler)) {
    raise_warning("Unable to call handler %s()", handler.data());
  } else if (intern->m_registerPhpFunctions == 2 &&
             !intern->m_registered_phpfunctions.exists(handler)) {
    /* Push an empty string, so that we at least have an xslt result... */
    valuePush(ctxt, xmlXPathNewString((xmlChar *)""));
    raise_warning("Not allowed to call handler '%s()'.", handler.data());
  } else {
    Variant retval = f_call_user_func_array(handler, args);
    if (retval.instanceof("DOMNode")) {
      if (intern->m_node_list.empty()) {
        intern->m_node_list = Array::Create();
      }
      intern->m_node_list.append(retval);
      xmlNode *nodep = retval.toObject().getTyped<c_domnode>()->m_node;
      valuePush(ctxt, xmlXPathNewNodeSet(nodep));
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

c_domxpath::c_domxpath() : m_node(NULL), m_registerPhpFunctions(0) {
}

c_domxpath::~c_domxpath() {
  if (m_node) {
    xmlXPathFreeContext((xmlXPathContextPtr)m_node);
    m_node = NULL;
  }
}

void c_domxpath::t___construct(CVarRef doc) {
  m_doc = doc.toObject().getTyped<c_domdocument>();
  xmlDocPtr docp = (xmlDocPtr)m_doc->m_node;
  xmlXPathContextPtr ctx = xmlXPathNewContext(docp);
  if (ctx == NULL) {
    php_dom_throw_error(INVALID_STATE_ERR, 1);
    return;
  }

  xmlXPathRegisterFuncNS(ctx, (const xmlChar *) "functionString",
                         (const xmlChar *) "http://php.net/xpath",
                         dom_xpath_ext_function_string_php);
  xmlXPathRegisterFuncNS(ctx, (const xmlChar *) "function",
                         (const xmlChar *) "http://php.net/xpath",
                         dom_xpath_ext_function_object_php);
  m_node = (xmlNodePtr)ctx;
  ctx->userData = this;
}

Variant c_domxpath::t___destruct() {
  return null;
}

Variant c_domxpath::t___get(Variant name) {
  return domxpath_properties_map.getter(name)(this);
}

Variant c_domxpath::t___set(Variant name, Variant value) {
  domxpath_properties_map.setter(name)(this, value);
  return null;
}

Variant c_domxpath::t_evaluate(CStrRef expr,
                               CObjRef context /* = null_object */) {
  return php_xpath_eval(this, expr, context, PHP_DOM_XPATH_EVALUATE);
}

Variant c_domxpath::t_query(CStrRef expr,
                            CObjRef context /* = null_object */) {
  return php_xpath_eval(this, expr, context, PHP_DOM_XPATH_QUERY);
}

bool c_domxpath::t_registernamespace(CStrRef prefix, CStrRef uri) {
  xmlXPathContextPtr ctxp = (xmlXPathContextPtr)m_node;
  if (ctxp == NULL) {
    raise_warning("Invalid XPath Context");
    return false;
  }
  return xmlXPathRegisterNs(ctxp, (xmlChar*)prefix.data(),
                            (xmlChar*)uri.data()) == 0;
}

Variant c_domxpath::t_registerphpfunctions(CVarRef funcs /* = null */) {
  if (funcs.isArray()) {
    Array arr = funcs.toArray();
    for (ArrayIter iter(arr); iter; ++iter) {
      m_registered_phpfunctions.set(iter.second(), "1");
    }
    m_registerPhpFunctions = 2;
    return true;
  }
  if (funcs.isString()) {
    m_registered_phpfunctions.set(funcs, "1");
    m_registerPhpFunctions = 2;
  } else {
    m_registerPhpFunctions = 1;
  }
  return null;
}

///////////////////////////////////////////////////////////////////////////////

c_domnodeiterator::c_domnodeiterator()
  : m_objmap(NULL), m_iter(NULL), m_index(-1) {
}

c_domnodeiterator::~c_domnodeiterator() {
  delete m_iter;
}

void c_domnodeiterator::reset_iterator(dom_iterable *objmap) {
  m_objmap = objmap;

  xmlNodePtr curnode = NULL;
  bool owner = false;
  if (m_objmap->m_nodetype != XML_ENTITY_NODE &&
      m_objmap->m_nodetype != XML_NOTATION_NODE) {
    if (m_objmap->m_nodetype == DOM_NODESET) {
      m_iter = new ArrayIter(m_objmap->m_baseobjptr);
    } else {
      xmlNodePtr nodep = m_objmap->m_baseobj.getTyped<c_domnode>()->m_node;
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
        ++m_index;
      }
    }
  } else {
    if (m_objmap->m_nodetype == XML_ENTITY_NODE) {
      curnode = php_dom_libxml_hash_iter(m_objmap->m_ht, 0);
    } else {
      curnode = php_dom_libxml_notation_iter(m_objmap->m_ht, 0);
      owner = true;
    }
    m_index = 1;
  }
 err:
  if (curnode) {
    sp_domdocument doc = m_objmap->m_baseobj.getTyped<c_domnode>()->m_doc;
    m_curobj = create_node_object(curnode, doc, owner);
  } else {
    m_curobj.reset();
  }
}

void c_domnodeiterator::t___construct() {
}

Variant c_domnodeiterator::t___destruct() {
  return null;
}

Variant c_domnodeiterator::t_current() {
  if (m_iter) {
    return m_iter->second();
  }
  return m_curobj;
}

Variant c_domnodeiterator::t_key() {
  if (m_iter) {
    return m_iter->first();
  }
  xmlNodePtr curnode = m_curobj.getTyped<c_domnode>()->m_node;
  return String((const char *)curnode->name, CopyString);
}

Variant c_domnodeiterator::t_next() {
  if (m_iter) {
    m_iter->next();
    return null;
  }

  xmlNodePtr curnode = NULL;
  bool owner = false;
  if (m_objmap->m_nodetype != XML_ENTITY_NODE &&
      m_objmap->m_nodetype != XML_NOTATION_NODE) {
    curnode = m_curobj.getTyped<c_domnode>()->m_node;
    if (m_objmap->m_nodetype == XML_ATTRIBUTE_NODE ||
        m_objmap->m_nodetype == XML_ELEMENT_NODE) {
      curnode = curnode->next;
    } else {
      /* Nav the tree evey time as this is LIVE */
      xmlNodePtr basenode =
        m_objmap->m_baseobj.getTyped<c_domnode>()->m_node;
      if (basenode && (basenode->type == XML_DOCUMENT_NODE ||
                       basenode->type == XML_HTML_DOCUMENT_NODE)) {
        basenode = xmlDocGetRootElement((xmlDoc *) basenode);
      } else if (basenode) {
        basenode = basenode->children;
      } else {
        goto err;
      }
      int previndex = 0;
      curnode = dom_get_elements_by_tag_name_ns_raw
        (basenode, m_objmap->m_ns.data(), m_objmap->m_local.data(),
         &previndex, m_index);
      ++m_index;
    }
  } else {
    if (m_objmap->m_nodetype == XML_ENTITY_NODE) {
      curnode = php_dom_libxml_hash_iter(m_objmap->m_ht, m_index);
    } else {
      curnode = php_dom_libxml_notation_iter(m_objmap->m_ht, m_index);
      owner = true;
    }
    ++m_index;
  }
err:
  if (curnode) {
    sp_domdocument doc = m_objmap->m_baseobj.getTyped<c_domnode>()->m_doc;
    m_curobj = create_node_object(curnode, doc, owner);
  } else {
    m_curobj.reset();
  }
  return null;
}

Variant c_domnodeiterator::t_rewind() {
  delete m_iter;
  m_iter = NULL;
  m_index = -1;
  reset_iterator(m_objmap);
  return null;
}

Variant c_domnodeiterator::t_valid() {
  if (m_iter) {
    return !m_iter->end();
  }
  return !m_curobj.isNull();
}

///////////////////////////////////////////////////////////////////////////////
// function-style wrappers

#define DOM_GET_OBJ(name)                                       \
  c_dom ##name *pobj = NULL;                                    \
  if (obj.isObject()) {                                         \
    pobj = obj.toObject().getTyped<c_dom ##name>(true, true);   \
    if (pobj == NULL) {                                         \
      raise_warning("Expecting dom " #name " object");          \
      return null;                                              \
    }                                                           \
  }                                                             \

Variant f_dom_document_create_element(CVarRef obj, CStrRef name,
                                      CStrRef value /* = null_string */) {
  DOM_GET_OBJ(document);
  return pobj->t_createelement(name, value);
}

Variant f_dom_document_create_document_fragment(CVarRef obj) {
  DOM_GET_OBJ(document);
  return pobj->t_createdocumentfragment();
}

Variant f_dom_document_create_text_node(CVarRef obj, CStrRef data) {
  DOM_GET_OBJ(document);
  return pobj->t_createtextnode(data);
}

Variant f_dom_document_create_comment(CVarRef obj, CStrRef data) {
  DOM_GET_OBJ(document);
  return pobj->t_createcomment(data);
}

Variant f_dom_document_create_cdatasection(CVarRef obj, CStrRef data) {
  DOM_GET_OBJ(document);
  return pobj->t_createcdatasection(data);
}

Variant f_dom_document_create_processing_instruction(CVarRef obj, CStrRef target, CStrRef data /* = null_string */) {
  DOM_GET_OBJ(document);
  return pobj->t_createprocessinginstruction(target, data);
}

Variant f_dom_document_create_attribute(CVarRef obj, CStrRef name) {
  DOM_GET_OBJ(document);
  return pobj->t_createattribute(name);
}

Variant f_dom_document_create_entity_reference(CVarRef obj, CStrRef name) {
  DOM_GET_OBJ(document);
  return pobj->t_createentityreference(name);
}

Variant f_dom_document_get_elements_by_tag_name(CVarRef obj, CStrRef name) {
  DOM_GET_OBJ(document);
  return pobj->t_getelementsbytagname(name);
}

Variant f_dom_document_import_node(CVarRef obj, CObjRef importednode,
                                   bool deep /* = false */) {
  DOM_GET_OBJ(document);
  return pobj->t_importnode(importednode, deep);
}

Variant f_dom_document_create_element_ns(CVarRef obj, CStrRef namespaceuri,
                                         CStrRef qualifiedname,
                                         CStrRef value /* = null_string */) {
  DOM_GET_OBJ(document);
  return pobj->t_createelementns(namespaceuri, qualifiedname, value);
}

Variant f_dom_document_create_attribute_ns(CVarRef obj, CStrRef namespaceuri,
                                           CStrRef qualifiedname) {
  DOM_GET_OBJ(document);
  return pobj->t_createattributens(namespaceuri, qualifiedname);
}

Variant f_dom_document_get_elements_by_tag_name_ns(CVarRef obj, CStrRef namespaceuri, CStrRef localname) {
  DOM_GET_OBJ(document);
  return pobj->t_getelementsbytagnamens(namespaceuri, localname);
}

Variant f_dom_document_get_element_by_id(CVarRef obj, CStrRef elementid) {
  DOM_GET_OBJ(document);
  return pobj->t_getelementbyid(elementid);
}

Variant f_dom_document_normalize_document(CVarRef obj) {
  DOM_GET_OBJ(document);
  pobj->t_normalizedocument();
  return null;
}

Variant f_dom_document_save(CVarRef obj, CStrRef file,
                            int64 options /* = 0 */) {
  DOM_GET_OBJ(document);
  return pobj->t_save(file, options);
}

Variant f_dom_document_savexml(CVarRef obj, CObjRef node /* = null_object */,
                               int64 options /* = 0 */) {
  DOM_GET_OBJ(document);
  return pobj->t_savexml(node, options);
}

Variant f_dom_document_validate(CVarRef obj) {
  DOM_GET_OBJ(document);
  return pobj->t_validate();
}

Variant f_dom_document_xinclude(CVarRef obj, int64 options /* = 0 */) {
  DOM_GET_OBJ(document);
  return pobj->t_xinclude(options);
}

Variant f_dom_document_save_html(CVarRef obj) {
  DOM_GET_OBJ(document);
  return pobj->t_savehtml();
}

Variant f_dom_document_save_html_file(CVarRef obj, CStrRef file) {
  DOM_GET_OBJ(document);
  return pobj->t_savehtmlfile(file);
}

Variant f_dom_document_schema_validate_file(CVarRef obj, CStrRef filename) {
  DOM_GET_OBJ(document);
  return pobj->t_schemavalidate(filename);
}

Variant f_dom_document_schema_validate_xml(CVarRef obj, CStrRef source) {
  DOM_GET_OBJ(document);
  return pobj->t_schemavalidatesource(source);
}

Variant f_dom_document_relaxng_validate_file(CVarRef obj, CStrRef filename) {
  DOM_GET_OBJ(document);
  return pobj->t_relaxngvalidate(filename);
}

Variant f_dom_document_relaxng_validate_xml(CVarRef obj, CStrRef source) {
  DOM_GET_OBJ(document);
  return pobj->t_relaxngvalidatesource(source);
}

Variant f_dom_node_insert_before(CVarRef obj, CObjRef newnode,
                                 CObjRef refnode /* = null */) {
  DOM_GET_OBJ(node);
  return pobj->t_insertbefore(newnode, refnode);
}

Variant f_dom_node_replace_child(CVarRef obj, CObjRef newchildobj,
                                 CObjRef oldchildobj) {
  DOM_GET_OBJ(node);
  return pobj->t_replacechild(newchildobj, oldchildobj);
}

Variant f_dom_node_remove_child(CVarRef obj, CObjRef node) {
  DOM_GET_OBJ(node);
  return pobj->t_removechild(node);
}

Variant f_dom_node_append_child(CVarRef obj, CObjRef newnode) {
  DOM_GET_OBJ(node);
  return pobj->t_appendchild(newnode);
}

Variant f_dom_node_has_child_nodes(CVarRef obj) {
  DOM_GET_OBJ(node);
  return pobj->t_haschildnodes();
}

Variant f_dom_node_clone_node(CVarRef obj, bool deep /* = false */) {
  DOM_GET_OBJ(node);
  return pobj->t_clonenode(deep);
}

Variant f_dom_node_normalize(CVarRef obj) {
  DOM_GET_OBJ(node);
  pobj->t_normalize();
  return null;
}

Variant f_dom_node_is_supported(CVarRef obj, CStrRef feature,
                                CStrRef version) {
  DOM_GET_OBJ(node);
  return pobj->t_issupported(feature, version);
}

Variant f_dom_node_has_attributes(CVarRef obj) {
  DOM_GET_OBJ(node);
  return pobj->t_hasattributes();
}

Variant f_dom_node_is_same_node(CVarRef obj, CObjRef node) {
  DOM_GET_OBJ(node);
  return pobj->t_issamenode(node);
}

Variant f_dom_node_lookup_prefix(CVarRef obj, CStrRef prefix) {
  DOM_GET_OBJ(node);
  return pobj->t_lookupprefix(prefix);
}

Variant f_dom_node_is_default_namespace(CVarRef obj, CStrRef namespaceuri) {
  DOM_GET_OBJ(node);
  return pobj->t_isdefaultnamespace(namespaceuri);
}

Variant f_dom_node_lookup_namespace_uri(CVarRef obj, CStrRef namespaceuri) {
  DOM_GET_OBJ(node);
  return pobj->t_lookupnamespaceuri(namespaceuri);
}

Variant f_dom_nodelist_item(CVarRef obj, int64 index) {
  DOM_GET_OBJ(nodelist);
  return pobj->t_item(index);
}

Variant f_dom_namednodemap_get_named_item(CVarRef obj, CStrRef name) {
  DOM_GET_OBJ(namednodemap);
  return pobj->t_getnameditem(name);
}

Variant f_dom_namednodemap_item(CVarRef obj, int64 index) {
  DOM_GET_OBJ(namednodemap);
  return pobj->t_item(index);
}

Variant f_dom_namednodemap_get_named_item_ns(CVarRef obj, CStrRef namespaceuri,
                                             CStrRef localname) {
  DOM_GET_OBJ(namednodemap);
  return pobj->t_getnameditemns(namespaceuri, localname);
}

Variant f_dom_characterdata_substring_data(CVarRef obj, int64 offset,
                                           int64 count) {
  DOM_GET_OBJ(characterdata);
  return pobj->t_substringdata(offset, count);
}

Variant f_dom_characterdata_append_data(CVarRef obj, CStrRef arg) {
  DOM_GET_OBJ(characterdata);
  return pobj->t_appenddata(arg);
}

Variant f_dom_characterdata_insert_data(CVarRef obj, int64 offset,
                                        CStrRef data) {
  DOM_GET_OBJ(characterdata);
  return pobj->t_insertdata(offset, data);
}

Variant f_dom_characterdata_delete_data(CVarRef obj, int64 offset,
                                        int64 count) {
  DOM_GET_OBJ(characterdata);
  return pobj->t_deletedata(offset, count);
}

Variant f_dom_characterdata_replace_data(CVarRef obj, int64 offset,
                                         int64 count, CStrRef data) {
  DOM_GET_OBJ(characterdata);
  return pobj->t_replacedata(offset, count, data);
}

Variant f_dom_attr_is_id(CVarRef obj) {
  DOM_GET_OBJ(attr);
  return pobj->t_isid();
}

Variant f_dom_element_get_attribute(CVarRef obj, CStrRef name) {
  DOM_GET_OBJ(element);
  return pobj->t_getattribute(name);
}

Variant f_dom_element_set_attribute(CVarRef obj, CStrRef name, CStrRef value) {
  DOM_GET_OBJ(element);
  return pobj->t_setattribute(name, value);
}

Variant f_dom_element_remove_attribute(CVarRef obj, CStrRef name) {
  DOM_GET_OBJ(element);
  return pobj->t_removeattribute(name);
}

Variant f_dom_element_get_attribute_node(CVarRef obj, CStrRef name) {
  DOM_GET_OBJ(element);
  return pobj->t_getattributenode(name);
}

Variant f_dom_element_set_attribute_node(CVarRef obj, CObjRef newattr) {
  DOM_GET_OBJ(element);
  return pobj->t_setattributenode(newattr);
}

Variant f_dom_element_remove_attribute_node(CVarRef obj, CObjRef oldattr) {
  DOM_GET_OBJ(element);
  return pobj->t_removeattributenode(oldattr);
}

Variant f_dom_element_get_elements_by_tag_name(CVarRef obj, CStrRef name) {
  DOM_GET_OBJ(element);
  return pobj->t_getelementsbytagname(name);
}

Variant f_dom_element_get_attribute_ns(CVarRef obj, CStrRef namespaceuri,
                                       CStrRef localname) {
  DOM_GET_OBJ(element);
  return pobj->t_getattributens(namespaceuri, localname);
}

Variant f_dom_element_set_attribute_ns(CVarRef obj, CStrRef namespaceuri,
                                       CStrRef name, CStrRef value) {
  DOM_GET_OBJ(element);
  return pobj->t_setattributens(namespaceuri, name, value);
}

Variant f_dom_element_remove_attribute_ns(CVarRef obj, CStrRef namespaceuri,
                                          CStrRef localname) {
  DOM_GET_OBJ(element);
  return pobj->t_removeattributens(namespaceuri, localname);
}

Variant f_dom_element_get_attribute_node_ns(CVarRef obj, CStrRef namespaceuri,
                                            CStrRef localname) {
  DOM_GET_OBJ(element);
  return pobj->t_getattributenodens(namespaceuri, localname);
}

Variant f_dom_element_set_attribute_node_ns(CVarRef obj, CObjRef newattr) {
  DOM_GET_OBJ(element);
  return pobj->t_setattributenodens(newattr);
}

Variant f_dom_element_get_elements_by_tag_name_ns(CVarRef obj,
                                                  CStrRef namespaceuri,
                                                  CStrRef localname) {
  DOM_GET_OBJ(element);
  return pobj->t_getelementsbytagnamens(namespaceuri, localname);
}

Variant f_dom_element_has_attribute(CVarRef obj, CStrRef name) {
  DOM_GET_OBJ(element);
  return pobj->t_hasattribute(name);
}

Variant f_dom_element_has_attribute_ns(CVarRef obj, CStrRef namespaceuri,
                                       CStrRef localname) {
  DOM_GET_OBJ(element);
  return pobj->t_hasattributens(namespaceuri, localname);
}

Variant f_dom_element_set_id_attribute(CVarRef obj, CStrRef name, bool isid) {
  DOM_GET_OBJ(element);
  return pobj->t_setidattribute(name, isid);
}

Variant f_dom_element_set_id_attribute_ns(CVarRef obj, CStrRef namespaceuri,
                                          CStrRef localname, bool isid) {
  DOM_GET_OBJ(element);
  return pobj->t_setidattributens(namespaceuri, localname, isid);
}

Variant f_dom_element_set_id_attribute_node(CVarRef obj, CObjRef idattr,
                                            bool isid) {
  DOM_GET_OBJ(element);
  return pobj->t_setidattributenode(idattr, isid);
}

Variant f_dom_text_split_text(CVarRef obj, int64 offset) {
  DOM_GET_OBJ(text);
  return pobj->t_splittext(offset);
}

Variant f_dom_text_is_whitespace_in_element_content(CVarRef obj) {
  DOM_GET_OBJ(text);
  return pobj->t_iswhitespaceinelementcontent();
}

Variant f_dom_xpath_register_ns(CVarRef obj, CStrRef prefix, CStrRef uri) {
  DOM_GET_OBJ(xpath);
  return pobj->t_registernamespace(prefix, uri);
}

Variant f_dom_xpath_query(CVarRef obj, CStrRef expr,
                          CObjRef context /* = null_object */) {
  DOM_GET_OBJ(xpath);
  return pobj->t_query(expr, context);
}

Variant f_dom_xpath_evaluate(CVarRef obj, CStrRef expr,
                             CObjRef context /* = null_object */) {
  DOM_GET_OBJ(xpath);
  return pobj->t_evaluate(expr, context);
}

Variant f_dom_xpath_register_php_functions(CVarRef obj,
                                           CVarRef funcs /* = null */) {
  DOM_GET_OBJ(xpath);
  return pobj->t_registerphpfunctions(funcs);
}

///////////////////////////////////////////////////////////////////////////////
}
