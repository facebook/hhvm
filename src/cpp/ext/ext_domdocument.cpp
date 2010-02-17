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

#include <cpp/ext/ext_domdocument.h>
#include <cpp/ext/ext_file.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// domexception errors
typedef enum {
// PHP_ERR is non-spec code for PHP errors:
  PHP_ERR                        = 0,
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
// Introduced in DOM Level 2:
  INVALID_STATE_ERR              = 11,
// Introduced in DOM Level 2:
  SYNTAX_ERR                     = 12,
// Introduced in DOM Level 2:
  INVALID_MODIFICATION_ERR       = 13,
// Introduced in DOM Level 2:
  NAMESPACE_ERR                  = 14,
// Introduced in DOM Level 2:
  INVALID_ACCESS_ERR             = 15,
// Introduced in DOM Level 3:
  VALIDATION_ERR                 = 16
} dom_exception_code;

#define DOM_XMLNS_NAMESPACE \
    (const xmlChar *) "http://www.w3.org/2000/xmlns/"

#define DOM_LOAD_STRING 0
#define DOM_LOAD_FILE 1

#define LIBXML_SAVE_NOEMPTYTAG 1<<2

#define PHP_DOM_XPATH_QUERY 0
#define PHP_DOM_XPATH_EVALUATE 1

#define VCWD_REALPATH(path, real_path) f_realpath(path, real_path)

int dom_has_feature(char *feature, char *version)
{
  int retval = 0;
  if (!(strcmp (version, "1.0") && strcmp (version,"2.0") && strcmp(version, ""))) {
    if ((!strcasecmp(feature, "Core") && !strcmp (version, "1.0")) || !strcasecmp(feature, "XML"))
      retval = 1;
  }
  return retval;
}

void dom_normalize(xmlNodePtr nodep)
{
  xmlNodePtr child, nextp, newnextp;
  xmlAttrPtr attr;
  xmlChar *strContent;
  child = nodep->children;
  while(child != NULL) {
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
        dom_normalize (child);
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
    child = child->next;
  }
}

static Variant dom_canonicalization(xmlNodePtr nodep, CStrRef file, bool exclusive, bool with_comments, CVarRef xpath_array, CVarRef ns_prefixes, int mode)
{
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
      xpathobjp = xmlXPathEvalExpression((xmlChar*)"(.//. | .//@* | .//namespace::*)", ctxp);
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
    if (!arr.exists("query"))
      return false;
    Variant tmp = arr.rvalAt("query");
    if(tmp.isString())
      return false;
    xquery = tmp.toString();
    ctxp = xmlXPathNewContext(docp);
    ctxp->node = nodep;
    if (arr.exists("namespaces")) {
      Variant tmp = arr.rvalAt("namespaces");
      if(tmp.isArray()) {
        for (ArrayIter it = tmp.toArray().begin(); !it.end(); it.next()) {
          Variant prefix = it.first();
          Variant tmpns = it.second();
          if (prefix.isString() || tmpns.isString()) {
            xmlXPathRegisterNs(ctxp, (xmlChar*)prefix.toString().data(), (xmlChar*)tmpns.toString().data());
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
      inclusive_ns_prefixes = (xmlChar**)malloc((ns_prefixes.toArray().size()+1) * sizeof(xmlChar *));
      for (ArrayIter it = ns_prefixes.toArray().begin(); !it.end(); it.next()) {
        Variant tmpns = it.second();
        if (tmpns.isString()) {
          inclusive_ns_prefixes[nscount++] = (xmlChar*)tmpns.toString().data();
        }
      }
      inclusive_ns_prefixes[nscount] = NULL;
    } else {
      //php_error_docref(NULL TSRMLS_CC, E_NOTICE,
      //  "Inclusive namespace prefixes only allowed in exlcusive mode.");
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

bool dom_node_children_valid(xmlNodePtr node) {
  switch (node->type) {
    case XML_DOCUMENT_TYPE_NODE:
    case XML_DTD_NODE:
    case XML_PI_NODE:
    case XML_COMMENT_NODE:
    case XML_TEXT_NODE:
    case XML_CDATA_SECTION_NODE:
    case XML_NOTATION_NODE:
      return false;
      break;
    default:
      return true;
  }
}

bool dom_node_is_read_only(xmlNodePtr node) {
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
      break;
    default:
      if (node->doc == NULL) {
        return true;
      } else {
        return false;
      }
  }
}

void dom_set_old_ns(xmlDoc *doc, xmlNs *ns) {
  xmlNs *cur;
  if (doc == NULL)
    return;
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

static void dom_reconcile_ns(xmlDocPtr doc, xmlNodePtr nodep)
{
  xmlNsPtr nsptr, nsdftptr, curns, prevns = NULL;
  if (nodep->type == XML_ELEMENT_NODE) {
    // Following if block primarily used for inserting nodes created via createElementNS
    if (nodep->nsDef != NULL) {
      curns = nodep->nsDef;
      while (curns) {
        nsdftptr = curns->next;
        if (curns->href != NULL) {
          if((nsptr = xmlSearchNsByHref(doc, nodep->parent, curns->href)) &&
            (curns->prefix == NULL || xmlStrEqual(nsptr->prefix, curns->prefix))) {
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

bool dom_hierarchy(xmlNodePtr parent, xmlNodePtr child)
{
  xmlNodePtr nodep;
  if (parent == NULL || child == NULL || child->doc != parent->doc) {
    return true;
  }
  nodep = parent;
  while (nodep) {
    if (nodep == child) {
      return false;
    }
    nodep = nodep->parent;
  }
  return true;
}

static xmlNodePtr _php_dom_insert_fragment(xmlNodePtr nodep, xmlNodePtr prevsib, xmlNodePtr nextsib, xmlNodePtr fragment)
{
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
        if (node->_private != NULL) {
          //childobj = node->_private;
          //childobj->document = intern->document;
          //php_libxml_increment_doc_ref((php_libxml_node_object *)childobj, NULL TSRMLS_CC);
        }
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

int dom_check_qname(char *qname, char **localname, char **prefix, int uri_len, int name_len) {
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

xmlNsPtr dom_get_ns(xmlNodePtr nodep, char *uri, int *errorcode, char *prefix) {
  xmlNsPtr nsptr = NULL;
  *errorcode = 0;
  if (! ((prefix && !strcmp (prefix, "xml") && strcmp(uri, (char *)XML_XML_NAMESPACE)) ||
       (prefix && !strcmp (prefix, "xmlns") && strcmp(uri, (char *)DOM_XMLNS_NAMESPACE)) ||
       (prefix && !strcmp(uri, (char *)DOM_XMLNS_NAMESPACE) && strcmp (prefix, "xmlns")))) {
    nsptr = xmlNewNs(nodep, (xmlChar *)uri, (xmlChar *)prefix);
  }
  if (nsptr == NULL) {
    *errorcode = NAMESPACE_ERR;
  }
  return nsptr;
}

char *_dom_get_valid_file_path(char *source, char *resolved_path, int resolved_path_len)
{
  xmlURI *uri;
  xmlChar *escsource;
  char *file_dest;
  int isFileUri = 0;

  uri = xmlCreateURI();
  escsource = xmlURIEscapeStr((xmlChar*)source, (xmlChar*)":");
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

  file_dest = source;

  /*
  if ((uri->scheme == NULL || isFileUri)) {
    // XXX possible buffer overflow if VCWD_REALPATH does not know size of resolved_path
    if (!VCWD_REALPATH(source, resolved_path)) && !expand_filepath(source, resolved_path)) {
      xmlFreeURI(uri);
      return NULL;
    }
    file_dest = resolved_path;
  }
  */

  xmlFreeURI(uri);

  return file_dest;
}

static xmlDocPtr dom_document_parser(int mode, char *source, int options)
{
  xmlDocPtr ret;
  xmlParserCtxtPtr ctxt = NULL;
  //dom_doc_propsptr doc_props;
  //dom_object *intern;
  //php_libxml_ref_obj *document = NULL;
  //int validate, recover, resolve_externals, keep_blanks, substitute_ent;
  int resolved_path_len;
  //int old_error_reporting = 0;
  char *directory=NULL, resolved_path[4096];

  /*if (id != NULL) {
    intern = (dom_object *)zend_object_store_get_object(id TSRMLS_CC);
    document = intern->document;
  }*/

  /*
  doc_props = dom_get_doc_props(document);
  validate = doc_props->validateonparse;
  resolve_externals = doc_props->resolveexternals;
  keep_blanks = doc_props->preservewhitespace;
  substitute_ent = doc_props->substituteentities;
  recover = doc_props->recover;
  */

  /*if (document == NULL) {
    efree(doc_props);
  }*/

  xmlInitParser();

  if (mode == DOM_LOAD_FILE) {
    char *file_dest = _dom_get_valid_file_path(source, resolved_path, 4096);
    if (file_dest) {
      ctxt = xmlCreateFileParserCtxt(file_dest);
    }
  } else {
    ctxt = xmlCreateDocParserCtxt((xmlChar*)source);
  }

  if (ctxt == NULL) {
    return(NULL);
  }

  /* If loading from memory, we need to set the base directory for the document */
  if (mode != DOM_LOAD_FILE) {
#if HAVE_GETCWD
    directory = VCWD_GETCWD(resolved_path, 4096);
#elif HAVE_GETWD
    directory = VCWD_GETWD(resolved_path);
#endif
    if (directory) {
      if(ctxt->directory != NULL) {
        xmlFree((char *) ctxt->directory);
      }
      resolved_path_len = strlen(resolved_path);
      if (resolved_path[resolved_path_len - 1] != '/') {
        resolved_path[resolved_path_len] = '/';
        resolved_path[++resolved_path_len] = '\0';
      }
      ctxt->directory = (char*)xmlCanonicPath((const xmlChar*)resolved_path);
    }
  }

  /*
  ctxt->vctxt.error = php_libxml_ctx_error;
  ctxt->vctxt.warning = php_libxml_ctx_warning;

  if (ctxt->sax != NULL) {
    ctxt->sax->error = php_libxml_ctx_error;
    ctxt->sax->warning = php_libxml_ctx_warning;
  }
  */

  /*
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
  */

  xmlCtxtUseOptions(ctxt, options);

  /*ctxt->recovery = recover;
  if (recover) {
    old_error_reporting = EG(error_reporting);
    EG(error_reporting) = old_error_reporting | E_WARNING;
  }*/

  xmlParseDocument(ctxt);

  //if (ctxt->wellFormed || recover) {
    ret = ctxt->myDoc;
    /*if (ctxt->recovery) {
      EG(error_reporting) = old_error_reporting;
    }*/
    /* If loading from memory, set the base reference uri for the document */
    if (ret && ret->URL == NULL && ctxt->directory != NULL) {
      ret->URL = xmlStrdup((xmlChar*)ctxt->directory);
    }
  /*
  } else {
    ret = NULL;
    xmlFreeDoc(ctxt->myDoc);
    ctxt->myDoc = NULL;
  }
  */

  xmlFreeParserCtxt(ctxt);

  return(ret);
}

static Variant dom_parse_document(c_domdocument * domdoc, CStrRef source, int options, int mode) {
  //xmlDocPtr docp = (xmlDocPtr)domdoc->m_node;
  xmlDoc *newdoc;
  //dom_doc_propsptr doc_prop;
  //int refcount;
  if (source.size() == 0) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty string supplied as input");
    return false;
  }
  newdoc = dom_document_parser(mode, (char*)source.data(), options);
  if (!newdoc)
    return false;

  domdoc->m_node = (xmlNodePtr)newdoc;
  return true;

  // Hmm...
  /*
  if (id != NULL) {
      docp = (xmlDocPtr) dom_object_get_node(intern);
      doc_prop = NULL;
      if (docp != NULL) {
        php_libxml_decrement_node_ptr((php_libxml_node_object *) intern TSRMLS_CC);
        doc_prop = intern->document->doc_props;
        intern->document->doc_props = NULL;
        refcount = php_libxml_decrement_doc_ref((php_libxml_node_object *)intern TSRMLS_CC);
        if (refcount != 0) {
          docp->_private = NULL;
        }
      }
      intern->document = NULL;
      if (php_libxml_increment_doc_ref((php_libxml_node_object *)intern, newdoc TSRMLS_CC) == -1) {
        return false;
      }
      intern->document->doc_props = doc_prop;
    }
    php_libxml_increment_node_ptr((php_libxml_node_object *)intern, (xmlNodePtr)newdoc, (void *)intern TSRMLS_CC);
    return true;
  } else {
    SmartObject<c_domdocument> ret = NEW(c_domdocument)();
    ret->m_node = (xmlNodePtr)newdoc;
    return ret;
  }
  */
}

static Variant dom_load_html(c_domdocument * domdoc, CStrRef source, int mode)
{
  //SmartObject<c_domdocument> domdoc = obj.getTyped<c_domdocument>();
  //xmlDocPtr docp = (xmlDocPtr)domdoc->m_node;
  xmlDoc *newdoc;
  //int refcount;
  htmlParserCtxtPtr ctxt;
  if (source.size() == 0) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Empty string supplied as input");
    return false;
  }
  if (mode == DOM_LOAD_FILE) {
    ctxt = htmlCreateFileParserCtxt(source.data(), NULL);
  } else {
    //source_len = xmlStrlen(source);
    ctxt = htmlCreateMemoryParserCtxt(source.data(), source.size());
  }
  if (!ctxt) {
    return false;
  }
  /*
  ctxt->vctxt.error = php_libxml_ctx_error;
  ctxt->vctxt.warning = php_libxml_ctx_warning;
  if (ctxt->sax != NULL) {
    ctxt->sax->error = php_libxml_ctx_error;
    ctxt->sax->warning = php_libxml_ctx_warning;
  }
  */
  htmlParseDocument(ctxt);
  newdoc = ctxt->myDoc;
  htmlFreeParserCtxt(ctxt);
  if (!newdoc)
    return false;
  /*
  if (id != NULL && instanceof_function(Z_OBJCE_P(id), dom_document_class_entry TSRMLS_CC)) {
    intern = (dom_object *)zend_object_store_get_object(id TSRMLS_CC);
    if (intern != NULL) {
      docp = (xmlDocPtr) dom_object_get_node(intern);
      doc_prop = NULL;
      if (docp != NULL) {
        php_libxml_decrement_node_ptr((php_libxml_node_object *) intern TSRMLS_CC);
        doc_prop = intern->document->doc_props;
        intern->document->doc_props = NULL;
        //refcount = php_libxml_decrement_doc_ref((php_libxml_node_object *)intern TSRMLS_CC);
        //if (refcount != 0) {
        //  docp->_private = NULL;
        //}
      }
      intern->document = NULL;
      //if (php_libxml_increment_doc_ref((php_libxml_node_object *)intern, newdoc TSRMLS_CC) == -1) {
      //  return false;
      //}
      intern->document->doc_props = doc_prop;
    }
    //php_libxml_increment_node_ptr((php_libxml_node_object *)intern, (xmlNodePtr)newdoc, (void *)intern TSRMLS_CC);
    return true;
  } else {*/
    SmartObject<c_domdocument> ret = NEW(c_domdocument)();
    ret->m_node = (xmlNodePtr)newdoc;
    return ret;
  /*}*/
}

bool _dom_document_relaxNG_validate(c_domdocument * domdoc, CStrRef source, int type) {
  char *valid_file = NULL;
  xmlRelaxNGParserCtxtPtr parser;
  xmlRelaxNGPtr           sptr;
  xmlRelaxNGValidCtxtPtr  vptr;
  int                     is_valid;
  char resolved_path[4096 + 1];

  if (source.size() == 0) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid Schema source");
    return false;
  }

  xmlDocPtr docp = (xmlDocPtr)domdoc->m_node;

  switch (type) {
  case DOM_LOAD_FILE:
    valid_file = _dom_get_valid_file_path((char*)source.data(), resolved_path, 4096);
    if (!valid_file) {
      //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid RelaxNG file source");
      return false;
    }
    parser = xmlRelaxNGNewParserCtxt(valid_file);
    break;
  case DOM_LOAD_STRING:
    parser = xmlRelaxNGNewMemParserCtxt(source.data(), source.size());
    /* If loading from memory, we need to set the base directory for the document·
       but it is not apparent how to do that for schema's */
    break;
  default:
    return false;
  }

  /*xmlRelaxNGSetParserErrors(parser,
    (xmlRelaxNGValidityErrorFunc) php_libxml_error_handler,
    (xmlRelaxNGValidityWarningFunc) php_libxml_error_handler,
    parser);*/
  sptr = xmlRelaxNGParse(parser);
  xmlRelaxNGFreeParserCtxt(parser);
  if (!sptr) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid RelaxNG");
    return false;
  }

  //docp = (xmlDocPtr) dom_object_get_node(intern);

  vptr = xmlRelaxNGNewValidCtxt(sptr);
  if (!vptr) {
    xmlRelaxNGFree(sptr);
    //php_error(E_ERROR, "Invalid RelaxNG Validation Context");
    return false;
  }

  //xmlRelaxNGSetValidErrors(vptr, php_libxml_error_handler, php_libxml_error_handler, vptr);
  is_valid = xmlRelaxNGValidateDoc(vptr, docp);
  xmlRelaxNGFree(sptr);
  xmlRelaxNGFreeValidCtxt(vptr);

  if (is_valid == 0) {
    return true;
  } else {
    return false;
  }
}

bool _dom_document_schema_validate(c_domdocument * domdoc, CStrRef source, int type)
{
  xmlDocPtr docp = (xmlDocPtr)domdoc->m_node;
  char *valid_file = NULL;
  xmlSchemaParserCtxtPtr  parser;
  xmlSchemaPtr            sptr;
  xmlSchemaValidCtxtPtr   vptr;
  int                     is_valid;
  char resolved_path[4096 + 1];

  if (source.size() == 0) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid Schema source");
    return false;
  }

  switch (type) {
  case DOM_LOAD_FILE:
    valid_file = _dom_get_valid_file_path((char*)source.data(), resolved_path, 4096);
    if (!valid_file) {
      //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid Schema file source");
      return false;
    }
    parser = xmlSchemaNewParserCtxt(valid_file);
    break;
  case DOM_LOAD_STRING:
    parser = xmlSchemaNewMemParserCtxt(source.data(), source.size());
    /* If loading from memory, we need to set the base directory for the document·
       but it is not apparent how to do that for schema's */
    break;
  default:
    return false;
  }

  /*xmlSchemaSetParserErrors(parser,
    (xmlSchemaValidityErrorFunc) php_libxml_error_handler,
    (xmlSchemaValidityWarningFunc) php_libxml_error_handler,
    parser);*/
  sptr = xmlSchemaParse(parser);
  xmlSchemaFreeParserCtxt(parser);
  if (!sptr) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid Schema");
    return false;
  }

  vptr = xmlSchemaNewValidCtxt(sptr);
  if (!vptr) {
    xmlSchemaFree(sptr);
    //php_error(E_ERROR, "Invalid Schema Validation Context");
    return false;
  }

  /*xmlSchemaSetValidErrors(vptr, php_libxml_error_handler, php_libxml_error_handler, vptr);*/
  is_valid = xmlSchemaValidateDoc(vptr, docp);
  xmlSchemaFree(sptr);
  xmlSchemaFreeValidCtxt(vptr);

  if (is_valid == 0) {
    return true;
  } else {
    return false;
  }
}

static xmlNodePtr php_dom_free_xinclude_node(xmlNodePtr cur)
{
  xmlNodePtr xincnode;
  xincnode = cur;
  cur = cur->next;
  xmlUnlinkNode(xincnode);
  //php_libxml_node_free_resource(xincnode);
  return cur;
}

static void php_dom_remove_xinclude_nodes(xmlNodePtr cur)
{
  while(cur) {
    if (cur->type == XML_XINCLUDE_START) {
      cur = php_dom_free_xinclude_node(cur);
      /* XML_XINCLUDE_END node will be a sibling of XML_XINCLUDE_START */
      while(cur && cur->type != XML_XINCLUDE_END) {
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

static void php_dom_xmlSetTreeDoc(xmlNodePtr tree, xmlDocPtr doc)
{
  xmlAttrPtr prop;
  xmlNodePtr cur;
  if (tree) {
    if(tree->type == XML_ELEMENT_NODE) {
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

static xmlNodePtr dom_get_dom1_attribute(xmlNodePtr elem, xmlChar *name)
{
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

xmlNsPtr dom_get_nsdecl(xmlNode *node, xmlChar *localName) {
  xmlNsPtr cur;
  xmlNs *ret = NULL;
  if (node == NULL)
    return NULL;
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

static Variant php_xpath_eval(c_domxpath * domxpath, CStrRef expr, CObjRef context, int type) /* {{{ */
{
  xmlXPathObjectPtr xpathobjp;
  int nsnbr = 0, xpath_type;
  xmlDoc *docp = NULL;
  xmlNsPtr *ns;
  xmlXPathContextPtr ctxp = (xmlXPathContextPtr)domxpath->m_node;
  if (ctxp == NULL) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid XPath Context");
    return false;
  }
  docp = (xmlDocPtr)ctxp->doc;
  if (docp == NULL) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid XPath Document Pointer");
    return false;
  }
  xmlNodePtr nodep = NULL;
  if (!context.isNull()) {
    SmartObject<c_domxpath> domxpath = context.getTyped<c_domxpath>();
    nodep = domxpath->m_node;
  }
  if (!nodep) {
    nodep = xmlDocGetRootElement(docp);
  }
  if (nodep && docp != nodep->doc) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Node From Wrong Document");
    return false;
  }
  ctxp->node = nodep;
  /* Register namespaces in the node */
  ns = xmlGetNsList(docp, nodep);
  if (ns != NULL) {
    while (ns[nsnbr] != NULL)
      nsnbr++;
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
      if (xpathobjp->type == XPATH_NODESET && NULL != (nodesetp = xpathobjp->nodesetval)) {
        for (i = 0; i < nodesetp->nodeNr; i++) {
          xmlNodePtr node = nodesetp->nodeTab[i];
          if (node->type == XML_NAMESPACE_DECL) {
            xmlNsPtr curns;
            //xmlNodePtr nsparent;
            //nsparent = node->_private;
            curns = xmlNewNs(NULL, node->name, NULL);
            if (node->children) {
              curns->prefix = xmlStrdup((xmlChar*)node->children);
            }
            if (node->children) {
              node = xmlNewDocNode(docp, NULL, (xmlChar*)node->children, node->name);
            } else {
              node = xmlNewDocNode(docp, NULL, (xmlChar*)"xmlns", node->name);
            }
            node->type = XML_NAMESPACE_DECL;
            //node->parent = nsparent;
            node->ns = curns;
          }
          // Ugh, the statement below creates a new object and adds it to an array...
          SmartObject<c_domnode> child = NEW(c_domnode)();
          child->m_node = node;
          retval.append(child);
        }
      }
      // TODO: Return a nodelist object instead of an array..
      ret = retval;
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

void node_list_unlink(xmlNodePtr node)
{
  //dom_object *wrapper;
  while (node != NULL) {
    //wrapper = php_dom_object_get_data(node);
    //if (wrapper != NULL) {
    //  xmlUnlinkNode(node);
    //} else {
      if (node->type == XML_ENTITY_REF_NODE)
        break;
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
    //}
    node = node->next;
  }
}

static void php_set_attribute_id(xmlAttrPtr attrp, bool is_id)
{
  if (is_id == 1 && attrp->atype != XML_ATTRIBUTE_ID) {
    xmlChar *id_val;
    id_val = xmlNodeListGetString(attrp->doc, attrp->children, 1);
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

static xmlNsPtr _dom_new_reconNs(xmlDocPtr doc, xmlNodePtr tree, xmlNsPtr ns)
{
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
  if (ns->prefix == NULL)
    snprintf((char*)prefix, sizeof(prefix), "default");
  else
    snprintf((char*)prefix, sizeof(prefix), "%.20s", (char *)ns->prefix);
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


///////////////////////////////////////////////////////////////////////////////


c_domnode::c_domnode() {
}

c_domnode::~c_domnode() {
}

void c_domnode::t___construct() {
  throw NotImplementedException(__func__);
}

Variant c_domnode::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

Variant c_domnode::t_appendchild(CObjRef newnode) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  SmartObject<c_domnode> newdomnode = newnode.getTyped<c_domnode>();
  xmlNodePtr child = newdomnode->m_node;
  xmlNodePtr new_child = NULL;
  if (dom_node_is_read_only(nodep) ||
    (child->parent != NULL && dom_node_is_read_only(child->parent))) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror TSRMLS_CC);
    return false;
  }
  if (!dom_hierarchy(nodep, child)) {
    //php_dom_throw_error(HIERARCHY_REQUEST_ERR, stricterror TSRMLS_CC);
    return false;
  }
  if (!(child->doc == NULL || child->doc == nodep->doc)) {
    //php_dom_throw_error(WRONG_DOCUMENT_ERR, stricterror TSRMLS_CC);
    return false;
  }
  if (child->type == XML_DOCUMENT_FRAG_NODE && child->children == NULL) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Document Fragment is empty");
    return false;
  }
  if (child->doc == NULL && nodep->doc != NULL) {
    //childobj->document = intern->document;
    //php_libxml_increment_doc_ref((php_libxml_node_object *)childobj, NULL TSRMLS_CC);
  }
  if (child->parent != NULL){
    xmlUnlinkNode(child);
  }
  if (child->type == XML_TEXT_NODE && nodep->last != NULL && nodep->last->type == XML_TEXT_NODE) {
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
    if (child->ns == NULL)
      lastattr = xmlHasProp(nodep, child->name);
    else
      lastattr = xmlHasNsProp(nodep, child->name, child->ns->href);
    if (lastattr != NULL && lastattr->type != XML_ATTRIBUTE_DECL) {
      if (lastattr != (xmlAttrPtr)child) {
        xmlUnlinkNode((xmlNodePtr)lastattr);
        //php_libxml_node_free_resource((xmlNodePtr)lastattr);
      }
    }
  } else if (child->type == XML_DOCUMENT_FRAG_NODE) {
    new_child = _php_dom_insert_fragment(nodep, nodep->last, NULL, child);
  }
  if (new_child == NULL) {
    new_child = xmlAddChild(nodep, child);
    if (new_child == NULL) {
      //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Couldn't append node");
      return false;
    }
  }
  dom_reconcile_ns(nodep->doc, new_child);
  SmartObject<c_domnode> ret = NEW(c_domnode)();
  ret->m_node = new_child;
  return ret;
}

Variant c_domnode::t_clonenode(bool deep /* = false */) {
  throw NotImplementedException(__func__);
  xmlNodePtr n = m_node;
  xmlNode * node = xmlDocCopyNode(n, n->doc, deep);
  if (!node)
    return false;
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
  SmartObject<c_domnode> ret = NEW(c_domnode)();
  ret->m_node = node;
  return ret;
}

int64 c_domnode::t_getlineno() {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  return xmlGetLineNo(nodep);
}

bool c_domnode::t_hasattributes() {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  if (nodep->type != XML_ELEMENT_NODE)
    return false;
  if (nodep->properties) {
    return true;
  } else {
    return false;
  }
}

bool c_domnode::t_haschildnodes() {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  if (nodep->children) {
    return true;
  } else {
    return false;
  }
}

Variant c_domnode::t_insertbefore(CObjRef newnode, CObjRef refnode /* = null */) {
  throw NotImplementedException(__func__);
  xmlNodePtr parentp = m_node;
  if (!dom_node_children_valid(parentp)) {
    return false;
  }
  SmartObject<c_domnode> domchildnode = newnode.getTyped<c_domnode>();
  xmlNodePtr child = domchildnode->m_node;
  xmlNodePtr new_child = NULL;
  //stricterror = dom_get_strict_error(intern->document);
  if (dom_node_is_read_only(parentp) ||
    (child->parent != NULL && dom_node_is_read_only(child->parent))) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror TSRMLS_CC);
    return false;
  }
  if (!dom_hierarchy(parentp, child)) {
    //php_dom_throw_error(HIERARCHY_REQUEST_ERR, stricterror TSRMLS_CC);
    return false;
  }
  if (child->doc != parentp->doc && child->doc != NULL) {
    //php_dom_throw_error(WRONG_DOCUMENT_ERR, stricterror TSRMLS_CC);
    return false;
  }
  if (child->type == XML_DOCUMENT_FRAG_NODE && child->children == NULL) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Document Fragment is empty");
    return false;
  }
  if (child->doc == NULL && parentp->doc != NULL) {
    //childobj->document = intern->document;
    //php_libxml_increment_doc_ref((php_libxml_node_object *)childobj, NULL TSRMLS_CC);
  }
  if (!refnode.isNull()) {
    SmartObject<c_domnode> domrefnode = refnode.getTyped<c_domnode>();
    xmlNodePtr refp = domrefnode->m_node;
    if (refp->parent != parentp) {
      //php_dom_throw_error(NOT_FOUND_ERR, stricterror TSRMLS_CC);
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
      if (child->ns == NULL)
        lastattr = xmlHasProp(refp->parent, child->name);
      else
        lastattr = xmlHasNsProp(refp->parent, child->name, child->ns->href);
      if (lastattr != NULL && lastattr->type != XML_ATTRIBUTE_DECL) {
        if (lastattr != (xmlAttrPtr)child) {
          xmlUnlinkNode((xmlNodePtr)lastattr);
          //php_libxml_node_free_resource((xmlNodePtr) lastattr TSRMLS_CC);
        } else {
          SmartObject<c_domnode> ret = NEW(c_domnode)();
          ret->m_node = child;
          return ret;
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
    if (child->type == XML_TEXT_NODE && parentp->last != NULL && parentp->last->type == XML_TEXT_NODE) {
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
          //php_libxml_node_free_resource((xmlNodePtr) lastattr TSRMLS_CC);
        } else {
          SmartObject<c_domnode> ret = NEW(c_domnode)();
          ret->m_node = child;
          return ret;
        }
      }
    } else if (child->type == XML_DOCUMENT_FRAG_NODE) {
      new_child = _php_dom_insert_fragment(parentp, parentp->last, NULL, child);
    }
    if (new_child == NULL) {
      new_child = xmlAddChild(parentp, child);
    }
  }
  if (NULL == new_child) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Couldn't add newnode as the previous sibling of refnode");
    return false;
  }
  dom_reconcile_ns(parentp->doc, new_child);
  SmartObject<c_domnode> ret = NEW(c_domnode)();
  ret->m_node = new_child;
  return ret;
}

bool c_domnode::t_isdefaultnamespace(CStrRef namespaceuri) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  xmlNsPtr nsptr;
  if (nodep->type == XML_DOCUMENT_NODE || nodep->type == XML_HTML_DOCUMENT_NODE) {
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
  throw NotImplementedException(__func__);
  SmartObject<c_domnode> otherdomnode = node.getTyped<c_domnode>();
  if (m_node == otherdomnode->m_node)
    return true;
  return false;
}

bool c_domnode::t_issupported(CStrRef feature, CStrRef version) {
  throw NotImplementedException(__func__);
  // dom_has_feature makes a bunch of calls to strcmp, we should
  // look into making it use string_strcmp instead
  if (dom_has_feature((char*)feature.data(), (char*)version.data()))
    return true;
  return false;
}

Variant c_domnode::t_lookupnamespaceuri(CStrRef namespaceuri) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  xmlNsPtr nsptr;
  if (nodep->type == XML_DOCUMENT_NODE || nodep->type == XML_HTML_DOCUMENT_NODE) {
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
  throw NotImplementedException(__func__);
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
        break;
      default:
        lookupp = nodep->parent;
    }
    if (lookupp != NULL && (nsptr = xmlSearchNsByHref(lookupp->doc, lookupp, (xmlChar*)prefix.data()))) {
      if (nsptr->prefix != NULL) {
        return String((char *)nsptr->prefix, CopyString);
      }
    }
  }
  return null;
}

void c_domnode::t_normalize() {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  dom_normalize(nodep);
}

Variant c_domnode::t_removechild(CObjRef node) {
  throw NotImplementedException(__func__);
  xmlNodePtr children;
  xmlNodePtr nodep = m_node;
  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  SmartObject<c_domnode> domnode2 = node.getTyped<c_domnode>();
  xmlNodePtr child = domnode2->m_node;
  //stricterror = dom_get_strict_error(intern->document);
  if (dom_node_is_read_only(nodep) ||
    (child->parent != NULL && dom_node_is_read_only(child->parent))) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror TSRMLS_CC);
    return false;
  }
  children = nodep->children;
  if (!children) {
    //php_dom_throw_error(NOT_FOUND_ERR, stricterror TSRMLS_CC);
    return false;
  }
  while (children) {
    if (children == child) {
      xmlUnlinkNode(child);
      SmartObject<c_domnode> ret = NEW(c_domnode)();
      ret->m_node = child;
      return ret;
    }
    children = children->next;
  }
  //php_dom_throw_error(NOT_FOUND_ERR, stricterror TSRMLS_CC);
  return false;
}

Variant c_domnode::t_replacechild(CObjRef newchildobj, CObjRef oldchildobj) {
  throw NotImplementedException(__func__);
  int foundoldchild = 0;
  xmlNodePtr nodep = m_node;
  if (!dom_node_children_valid(nodep)) {
    return false;
  }
  SmartObject<c_domnode> domnewchildnode = newchildobj.getTyped<c_domnode>();
  xmlNodePtr newchild = domnewchildnode->m_node;
  SmartObject<c_domnode> domoldchildnode = oldchildobj.getTyped<c_domnode>();
  xmlNodePtr oldchild = domoldchildnode->m_node;
  xmlNodePtr children = nodep->children;
  if (!children) {
    return false;
  }
  //stricterror = dom_get_strict_error(intern->document);
  if (dom_node_is_read_only(nodep) ||
    (newchild->parent != NULL && dom_node_is_read_only(newchild->parent))) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror TSRMLS_CC);
    return false;
  }
  if (newchild->doc != nodep->doc && newchild->doc != NULL) {
    //php_dom_throw_error(WRONG_DOCUMENT_ERR, stricterror TSRMLS_CC);
    return false;
  }
  if (!dom_hierarchy(nodep, newchild)) {
    //php_dom_throw_error(HIERARCHY_REQUEST_ERR, stricterror TSRMLS_CC);
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
        //newchildobj->document = intern->document;
        //php_libxml_increment_doc_ref((php_libxml_node_object *)newchildobj, NULL TSRMLS_CC);
      }
      node = xmlReplaceNode(oldchild, newchild);
      dom_reconcile_ns(nodep->doc, newchild);
    }
    SmartObject<c_domnode> ret = NEW(c_domnode)();
    ret->m_node = oldchild;
    return ret;
  } else {
    //php_dom_throw_error(NOT_FOUND_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
}

Variant c_domnode::t_c14n(bool exclusive /* = false */, bool with_comments /* = false */, CVarRef xpath /* = null */, CVarRef ns_prefixes /* = null */) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  return dom_canonicalization(nodep, "", exclusive, with_comments, xpath, ns_prefixes, 0);
}

Variant c_domnode::t_c14nfile(CStrRef uri, bool exclusive /* = false */, bool with_comments /* = false */, CVarRef xpath /* = null */, CVarRef ns_prefixes /* = null */) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  return dom_canonicalization(nodep, uri, exclusive, with_comments, xpath, ns_prefixes, 1);
}

Variant c_domnode::t_getnodepath() {
  throw NotImplementedException(__func__);
  xmlNodePtr n = m_node;
  char *value = (char*)xmlGetNodePath(n);
  if (value == NULL) {
    return null;
  } else {
    String ret = String(value, CopyString);
    xmlFree(value);
    return ret;
  }
}

c_domattr::c_domattr() {
}

c_domattr::~c_domattr() {
}

void c_domattr::t___construct(CVarRef name, CVarRef value) {
  throw NotImplementedException(__func__);
}

Variant c_domattr::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

bool c_domattr::t_isid() {
  throw NotImplementedException(__func__);
  xmlAttrPtr attrp = (xmlAttrPtr)m_node;
  if (attrp->atype == XML_ATTRIBUTE_ID) {
    return true;
  } else {
    return false;
  }
}

c_domcharacterdata::c_domcharacterdata() {
}

c_domcharacterdata::~c_domcharacterdata() {
}

void c_domcharacterdata::t___construct(CVarRef value) {
  throw NotImplementedException(__func__);
}

Variant c_domcharacterdata::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

bool c_domcharacterdata::t_appenddata(CStrRef arg) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  // Implement logic from libxml xmlTextConcat to add suport for comments and PI
  if ((nodep->content == (xmlChar *) &(nodep->properties)) ||
      ((nodep->doc != NULL) && (nodep->doc->dict != NULL) &&
      xmlDictOwns(nodep->doc->dict, nodep->content))) {
    nodep->content = xmlStrncatNew(nodep->content, (xmlChar*)arg.data(), arg.size());
  } else {
    nodep->content = xmlStrncat(nodep->content, (xmlChar*)arg.data(), arg.size());
  }
  nodep->properties = NULL;
  return true;
}

bool c_domcharacterdata::t_deletedata(int64 offset, int64 count) {
  throw NotImplementedException(__func__);
  xmlNodePtr node = m_node;
  xmlChar    *cur, *substring, *second;
  int         length;
  cur = xmlNodeGetContent(node);
  if (cur == NULL) {
    return false;
  }
  length = xmlUTF8Strlen(cur);
  if (offset < 0 || count < 0 || offset > length) {
    xmlFree(cur);
    //php_dom_throw_error(INDEX_SIZE_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
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
  throw NotImplementedException(__func__);
  xmlNodePtr node = m_node;
  xmlChar  *cur, *first, *second;
  int      length;
  cur = xmlNodeGetContent(node);
  if (cur == NULL) {
    return false;
  }
  length = xmlUTF8Strlen(cur);
  if (offset < 0 || offset > length) {
    xmlFree(cur);
    //php_dom_throw_error(INDEX_SIZE_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
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

bool c_domcharacterdata::t_replacedata(int64 offset, int64 count, CStrRef data) {
  throw NotImplementedException(__func__);
  xmlNodePtr node = m_node;
  xmlChar *cur, *substring, *second = NULL;
  int         length;
  cur = xmlNodeGetContent(node);
  if (cur == NULL) {
    return false;
  }
  length = xmlUTF8Strlen(cur);
  if (offset < 0 || count < 0 || offset > length) {
    xmlFree(cur);
    //php_dom_throw_error(INDEX_SIZE_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
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
  throw NotImplementedException(__func__);
  xmlNodePtr node = m_node;
  xmlChar    *cur;
  xmlChar    *substring;
  int         length;
  cur = xmlNodeGetContent(node);
  if (cur == NULL) {
    return false;
  }
  length = xmlUTF8Strlen(cur);
  if (offset < 0 || count < 0 || offset > length) {
    xmlFree(cur);
    //php_dom_throw_error(INDEX_SIZE_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  if ((offset + count) > length) {
    count = length - offset;
  }
  substring = xmlUTF8Strsub(cur, offset, count);
  xmlFree(cur);
  String ret;
  if (substring) {
    ret = String((char*)substring, CopyString);
    xmlFree(substring);
  } else {
    ret = String("", CopyString);
  }
  return ret;
}

c_domcomment::c_domcomment() {
}

c_domcomment::~c_domcomment() {
}

void c_domcomment::t___construct(CVarRef value) {
  throw NotImplementedException(__func__);
}

Variant c_domcomment::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

c_domtext::c_domtext() {
}

c_domtext::~c_domtext() {
}

void c_domtext::t___construct(CVarRef value) {
  throw NotImplementedException(__func__);
}

Variant c_domtext::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

bool c_domtext::t_iswhitespaceinelementcontent() {
  throw NotImplementedException(__func__);
  xmlNodePtr node = m_node;
  if (xmlIsBlankNode(node)) {
    return true;
  } else {
    return false;
  }
}

Variant c_domtext::t_splittext(int64 offset) {
  throw NotImplementedException(__func__);
  xmlNodePtr node = m_node;
  xmlChar    *cur;
  xmlChar    *first;
  xmlChar    *second;
  xmlNodePtr  nnode;
  int         length;
  if (node->type != XML_TEXT_NODE) {
    return false;
  }
  cur = xmlNodeGetContent(node);
  if (cur == NULL) {
    return false;
  }
  length = xmlUTF8Strlen(cur);
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
  SmartObject<c_domtext> ret = NEW(c_domtext)();
  ret->m_node = nnode;
  return ret;
}

c_domcdatasection::c_domcdatasection() {
}

c_domcdatasection::~c_domcdatasection() {
}

void c_domcdatasection::t___construct(CVarRef value) {
  throw NotImplementedException(__func__);
}

Variant c_domcdatasection::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

c_domdocument::c_domdocument() {
}

c_domdocument::~c_domdocument() {
}

void c_domdocument::t___construct(CStrRef version, CStrRef encoding) {
  throw NotImplementedException(__func__);
  xmlDoc *docp = xmlNewDoc((xmlChar*)version.data());
  if (!docp) {
    //php_dom_throw_error(INVALID_STATE_ERR, 1 TSRMLS_CC);
    //RETURN_FALSE;
    return;
  }
  if (encoding.size() > 0) {
    docp->encoding = (const xmlChar*)xmlStrdup((xmlChar*)encoding.data());
  }
  m_node = (xmlNodePtr)docp;
}

Variant c_domdocument::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

Variant c_domdocument::t_createattribute(CStrRef name) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlAttrPtr node;
  if (xmlValidateName((xmlChar*)name.data(), 0) != 0) {
    //php_dom_throw_error(INVALID_CHARACTER_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  node = xmlNewDocProp(docp, (xmlChar*)name.data(), NULL);
  if (!node) {
    return false;
  }
  SmartObject<c_domattr> ret = NEW(c_domattr)();
  ret->m_node = (xmlNodePtr)node;
  return ret;
}

Variant c_domdocument::t_createattributens(CStrRef namespaceuri, CStrRef qualifiedname) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNodePtr nodep = NULL, root;
  xmlNsPtr nsptr;
  char *localname = NULL, *prefix = NULL;
  int errorcode;
  root = xmlDocGetRootElement(docp);
  if (root != NULL) {
    errorcode = dom_check_qname((char*)qualifiedname.data(), &localname, &prefix, namespaceuri.size(), qualifiedname.size());
    if (errorcode == 0) {
      if (xmlValidateName((xmlChar*)localname, 0) == 0) {
        nodep = (xmlNodePtr)xmlNewDocProp(docp, (xmlChar*)localname, NULL);
        if (nodep != NULL && namespaceuri.size() > 0) {
          nsptr = xmlSearchNsByHref(nodep->doc, root, (xmlChar*)namespaceuri.data());
          if (nsptr == NULL) {
            nsptr = dom_get_ns(root, (char*)namespaceuri.data(), &errorcode, prefix);
          }
          xmlSetNs(nodep, nsptr);
        }
      } else {
        errorcode = INVALID_CHARACTER_ERR;
      }
    }
  } else {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Document Missing Root Element");
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
    //php_dom_throw_error(errorcode, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  if (nodep == NULL) {
    return false;
  }
  SmartObject<c_domattr> ret = NEW(c_domattr)();
  ret->m_node = nodep;
  return ret;
}

Variant c_domdocument::t_createcdatasection(CStrRef data) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNode *node;
  node = xmlNewCDataBlock(docp, (xmlChar*)data.data(), data.size());
  if (!node) {
    return false;
  }
  SmartObject<c_domcdatasection> ret = NEW(c_domcharacterdata)();
  ret->m_node = node;
  return ret;
}

Variant c_domdocument::t_createcomment(CStrRef data) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNode *node;
  node = xmlNewCDataBlock(docp, (xmlChar*)data.data(), data.size());
  if (!node) {
    return false;
  }
  SmartObject<c_domcomment> ret = NEW(c_domcomment)();
  ret->m_node = node;
  return ret;
}

Variant c_domdocument::t_createdocumentfragment() {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNode *node;
  node = xmlNewDocFragment(docp);
  if (!node) {
    return false;
  }
  SmartObject<c_domdocumentfragment> ret = NEW(c_domdocumentfragment)();
  ret->m_node = node;
  return ret;
}

Variant c_domdocument::t_createelement(CStrRef name, CStrRef value /* = null_string */) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNode *node;
  if (xmlValidateName((xmlChar*)name.data(), 0) != 0) {
    //php_dom_throw_error(INVALID_CHARACTER_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  node = xmlNewDocNode(docp, NULL, (xmlChar*)name.data(), (xmlChar*)value.data());
  if (!node) {
    return false;
  }
  SmartObject<c_domelement> ret = NEW(c_domelement)();
  ret->m_node = node;
  return ret;
}

Variant c_domdocument::t_createelementns(CStrRef namespaceuri, CStrRef qualifiedname, CStrRef value /* = null_string */) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNodePtr nodep = NULL;
  xmlNsPtr nsptr = NULL;
  char *localname = NULL, *prefix = NULL;
  int errorcode;
  errorcode = dom_check_qname((char*)qualifiedname.data(), &localname, &prefix, namespaceuri.size(), qualifiedname.size());
  if (errorcode == 0) {
    if (xmlValidateName((xmlChar*)localname, 0) == 0) {
      nodep = xmlNewDocNode(docp, NULL, (xmlChar*)localname, (xmlChar*)value.data());
      if (nodep != NULL && !namespaceuri.isNull()) {
        nsptr = xmlSearchNsByHref(nodep->doc, nodep, (xmlChar*)namespaceuri.data());
        if (nsptr == NULL) {
          nsptr = dom_get_ns(nodep, (char*)namespaceuri.data(), &errorcode, prefix);
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
    //php_dom_throw_error(errorcode, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  if (nodep == NULL) {
    return false;
  }
  nodep->ns = nsptr;
  SmartObject<c_domelement> ret = NEW(c_domelement)();
  ret->m_node = nodep;
  return ret;
}

Variant c_domdocument::t_createentityreference(CStrRef name) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNode *node;
  if (xmlValidateName((xmlChar*)name.data(), 0) != 0) {
    //php_dom_throw_error(INVALID_CHARACTER_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  node = xmlNewReference(docp, (xmlChar*)name.data());
  if (!node) {
    return false;
  }
  SmartObject<c_domentity> ret = NEW(c_domentity)();
  ret->m_node = node;
  return ret;
}

Variant c_domdocument::t_createprocessinginstruction(CStrRef target, CStrRef data /* = null_string */) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNode *node;
  if (xmlValidateName((xmlChar*)target.data(), 0) != 0) {
    //php_dom_throw_error(INVALID_CHARACTER_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  node = xmlNewPI((xmlChar*)target.data(), (xmlChar*)data.data());
  if (!node) {
    return false;
  }
  node->doc = docp;
  SmartObject<c_domprocessinginstruction> ret = NEW(c_domprocessinginstruction)();
  ret->m_node = node;
  return ret;
}

Variant c_domdocument::t_createtextnode(CStrRef data) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNode *node;
  node = xmlNewDocText(docp, (xmlChar*)data.data());
  if (!node) {
    return false;
  }
  SmartObject<c_domtext> ret = NEW(c_domtext)();
  ret->m_node = node;
  return ret;
}

Variant c_domdocument::t_getelementbyid(CStrRef elementid) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlAttrPtr attrp;
  attrp = xmlGetID(docp, (xmlChar*)elementid.data());
  if (attrp && attrp->parent) {
    SmartObject<c_domtext> ret = NEW(c_domtext)();
    ret->m_node = attrp->parent;
    return ret;
  }
  return null;
}

Variant c_domdocument::t_getelementsbytagname(CStrRef name) {
  throw NotImplementedException(__func__);
}

Variant c_domdocument::t_getelementsbytagnamens(CStrRef namespaceuri, CStrRef localname) {
  throw NotImplementedException(__func__);
}

Variant c_domdocument::t_importnode(CObjRef importednode, bool deep /* = false */) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  SmartObject<c_domnode> domnode = importednode.getTyped<c_domnode>();
  xmlNodePtr nodep = domnode->m_node;
  xmlNodePtr retnodep;
  long recursive = deep ? 1 : 0;
  if (nodep->type == XML_HTML_DOCUMENT_NODE || nodep->type == XML_DOCUMENT_NODE
    || nodep->type == XML_DOCUMENT_TYPE_NODE) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot import: Node Type Not Supported");
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
  SmartObject<c_domnode> ret = NEW(c_domnode)();
  ret->m_node = retnodep;
  return ret;
}

Variant c_domdocument::t_load(CStrRef filename, int64 options /* = 0 */) {
  throw NotImplementedException(__func__);
  return dom_parse_document(this, filename, options, DOM_LOAD_FILE);
}

Variant c_domdocument::t_loadhtml(CStrRef source) {
  throw NotImplementedException(__func__);
  return dom_load_html(this, source, DOM_LOAD_STRING);
}

Variant c_domdocument::t_loadhtmlfile(CStrRef filename) {
  throw NotImplementedException(__func__);
  return dom_load_html(this, filename, DOM_LOAD_FILE);
}

Variant c_domdocument::t_loadxml(CStrRef source, int64 options /* = 0 */) {
  throw NotImplementedException(__func__);
  return dom_parse_document(this, source, options, DOM_LOAD_STRING);
}

void c_domdocument::t_normalizedocument() {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  dom_normalize((xmlNodePtr)docp);
}

bool c_domdocument::t_registernodeclass(CStrRef baseclass, CStrRef extendedclass) {
  throw NotImplementedException(__func__);
  return false;
}

bool c_domdocument::t_relaxngvalidate(CStrRef filename) {
  throw NotImplementedException(__func__);
  return _dom_document_relaxNG_validate(this, filename, DOM_LOAD_FILE);
}

bool c_domdocument::t_relaxngvalidatesource(CStrRef source) {
  throw NotImplementedException(__func__);
  return _dom_document_relaxNG_validate(this, source, DOM_LOAD_STRING);
}

Variant c_domdocument::t_save(CStrRef file, int64 options /* = 0 */) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  int bytes, format = 0, saveempty = 0;
  //dom_doc_propsptr doc_props;

  if (file.size() == 0) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid Filename");
    return false;
  }

  /* encoding handled by property on doc */
  //doc_props = dom_get_doc_props(intern->document);
  //format = doc_props->formatoutput;
  if (options & LIBXML_SAVE_NOEMPTYTAG) {
    saveempty = xmlSaveNoEmptyTags;
    xmlSaveNoEmptyTags = 1;
  }
  bytes = xmlSaveFormatFileEnc(file, docp, NULL, format);
  if (options & LIBXML_SAVE_NOEMPTYTAG) {
    xmlSaveNoEmptyTags = saveempty;
  }
  if (bytes == -1) {
    return false;
  }
  return bytes;
}

Variant c_domdocument::t_savehtml() {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlChar *mem;
  int size;
  htmlDocDumpMemory(docp, &mem, &size);
  if (!size) {
    if (mem)
      xmlFree(mem);
    return false;
  }
  String ret = String((char*)mem, size, CopyString);
  xmlFree(mem);
  return ret;
}

Variant c_domdocument::t_savehtmlfile(CStrRef file) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  int bytes, format = 0;
  //dom_doc_propsptr doc_props;
  if (file.size() == 0) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid Filename");
    return false;
  }
  /* encoding handled by property on doc */
  //doc_props = dom_get_doc_props(intern->document);
  //format = doc_props->formatoutput;
  bytes = htmlSaveFileFormat(file.data(), docp, NULL, format);
  if (bytes == -1) {
    return false;
  }
  return bytes;
}

Variant c_domdocument::t_savexml(CObjRef node /* = null_object */, int64 options /* = 0 */) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlBufferPtr buf;
  xmlChar *mem;
  //dom_doc_propsptr doc_props;
  int size, format = 0, saveempty = 0;

  //doc_props = dom_get_doc_props(intern->document);
  //format = doc_props->formatoutput;

  if (!node.isNull()) {
    SmartObject<c_domnode> domnode = node.getTyped<c_domnode>();
    xmlNodePtr node = domnode->m_node;
    /* Dump contents of Node */
    if (node->doc != docp) {
      //php_dom_throw_error(WRONG_DOCUMENT_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
      return false;
    }
    buf = xmlBufferCreate();
    if (!buf) {
      //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not fetch buffer");
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
  throw NotImplementedException(__func__);
  return _dom_document_schema_validate(this, filename, DOM_LOAD_FILE);
}

bool c_domdocument::t_schemavalidatesource(CStrRef source) {
  throw NotImplementedException(__func__);
  return _dom_document_schema_validate(this, source, DOM_LOAD_STRING);
}

bool c_domdocument::t_validate() {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlValidCtxt *cvp;
  if (docp->intSubset == NULL) {
    //php_error_docref(NULL TSRMLS_CC, E_NOTICE, "No DTD given in XML-Document");
  }
  cvp = xmlNewValidCtxt();
  cvp->userData = NULL;
  //cvp->error    = (xmlValidityErrorFunc) php_libxml_error_handler;
  //cvp->warning  = (xmlValidityErrorFunc) php_libxml_error_handler;
  bool ret;
  if (xmlValidateDocument(cvp, docp)) {
    ret = true;
  } else {
    ret = false;
  }
  xmlFreeValidCtxt(cvp);
  return ret;
}

Variant c_domdocument::t_xinclude(int64 options /* = 0 */) {
  throw NotImplementedException(__func__);
  xmlDocPtr docp = (xmlDocPtr)m_node;
  xmlNodePtr root;
  int err;
  err = xmlXIncludeProcessFlags(docp, options);
  /* XML_XINCLUDE_START and XML_XINCLUDE_END nodes need to be removed as these
  are added via xmlXIncludeProcess to mark beginning and ending of xincluded document·
  but are not wanted in resulting document - must be done even if err as it could fail after
  having processed some xincludes */
  root = (xmlNodePtr) docp->children;
  while(root && root->type != XML_ELEMENT_NODE && root->type != XML_XINCLUDE_START) {
    root = root->next;
  }
  if (root) {
    php_dom_remove_xinclude_nodes(root);
  }
  if (err) {
    return err;
  } else {
    return false;
  }
}

c_domdocumentfragment::c_domdocumentfragment() {
}

c_domdocumentfragment::~c_domdocumentfragment() {
}

void c_domdocumentfragment::t___construct() {
  throw NotImplementedException(__func__);
}

Variant c_domdocumentfragment::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

c_domdocumenttype::c_domdocumenttype() {
}

c_domdocumenttype::~c_domdocumenttype() {
}

void c_domdocumenttype::t___construct() {
  throw NotImplementedException(__func__);
}

Variant c_domdocumenttype::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

bool c_domdocumentfragment::t_appendxml(CStrRef data) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  int err;
  xmlNodePtr lst;
  if (dom_node_is_read_only(nodep)) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  if (data) {
    err = xmlParseBalancedChunkMemory(nodep->doc, NULL, NULL, 0, (xmlChar*)data.data(), &lst);
    if (err != 0) {
      return false;
    }
    /* Following needed due to bug in libxml2 <= 2.6.14
    ifdef after next libxml release as bug is fixed in their cvs */
    php_dom_xmlSetTreeDoc(lst, nodep->doc);
    /* End stupid hack */
    xmlAddChildList(nodep,lst);
  }
  return true;
}

c_domelement::c_domelement() {
}

c_domelement::~c_domelement() {
}

void c_domelement::t___construct(CVarRef name, CVarRef value, CVarRef namespaceuri) {
  throw NotImplementedException(__func__);
}

Variant c_domelement::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

String c_domelement::t_getattribute(CStrRef name) {
  throw NotImplementedException(__func__);
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
  if (value == NULL) {
    return String("");
  } else {
    String ret = String((char*)value, CopyString);
    xmlFree(value);
    return ret;
  }
}

Variant c_domelement::t_getattributenode(CStrRef name) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  xmlNodePtr attrp;
  attrp = dom_get_dom1_attribute(nodep, (xmlChar*)name.data());
  if (attrp == NULL) {
    return false;
  }
  if (attrp->type == XML_NAMESPACE_DECL) {
    xmlNsPtr curns;
    //xmlNodePtr nsparent;
    //nsparent = attrp->_private;
    curns = xmlNewNs(NULL, attrp->name, NULL);
    if (attrp->children) {
      curns->prefix = xmlStrdup((xmlChar*)attrp->children);
    }
    if (attrp->children) {
      attrp = xmlNewDocNode(nodep->doc, NULL, (xmlChar *) attrp->children, attrp->name);
    } else {
      attrp = xmlNewDocNode(nodep->doc, NULL, (xmlChar *)"xmlns", attrp->name);
    }
    attrp->type = XML_NAMESPACE_DECL;
    //attrp->parent = nsparent;
    attrp->ns = curns;
  }
  SmartObject<c_domnode> ret = NEW(c_domattr)();
  ret->m_node = (xmlNodePtr)attrp;
  return ret;
}

Object c_domelement::t_getattributenodens(CStrRef namespaceuri, CStrRef localname) {
  throw NotImplementedException(__func__);
  xmlNodePtr elemp = m_node;
  xmlAttrPtr attrp;
  attrp = xmlHasNsProp(elemp, (xmlChar*)localname.data(), (xmlChar*)namespaceuri.data());
  if (attrp == NULL) {
    return null_object;
  }
  SmartObject<c_domnode> ret = NEW(c_domattr)();
  ret->m_node = (xmlNodePtr)attrp;
  return ret;
}

String c_domelement::t_getattributens(CStrRef namespaceuri, CStrRef localname) {
  throw NotImplementedException(__func__);
  xmlNodePtr elemp = m_node;
  xmlNsPtr nsptr;
  xmlChar *strattr;
  strattr = xmlGetNsProp(elemp, (xmlChar*)localname.data(), (xmlChar*)namespaceuri.data());
  String ret = String("");
  if (strattr != NULL) {
    ret = String((char*)strattr, CopyString);
    xmlFree(strattr);
  } else {
    if (xmlStrEqual((xmlChar*)namespaceuri.data(), (xmlChar*)DOM_XMLNS_NAMESPACE)) {
      nsptr = dom_get_nsdecl(elemp, (xmlChar*)localname.data());
      if (nsptr != NULL) {
        ret = String((char*)nsptr->href, CopyString);
      }
    }
  }
  return ret;
}

Object c_domelement::t_getelementsbytagname(CStrRef name) {
  throw NotImplementedException(__func__);
}

Object c_domelement::t_getelementsbytagnamens(CStrRef namespaceuri, CStrRef localname) {
  throw NotImplementedException(__func__);
}

bool c_domelement::t_hasattribute(CStrRef name) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  xmlNodePtr attr;
  attr = dom_get_dom1_attribute(nodep, (xmlChar*)name.data());
  if (attr == NULL) {
    return false;
  } else {
    return true;
  }
}

bool c_domelement::t_hasattributens(CStrRef namespaceuri, CStrRef localname) {
  throw NotImplementedException(__func__);
  xmlNodePtr elemp = m_node;
  xmlNs *nsp;
  xmlChar *value;
  value = xmlGetNsProp(elemp, (xmlChar*)localname.data(), (xmlChar*)namespaceuri.data());
  if (value != NULL) {
    xmlFree(value);
    return true;
  } else {
    if (xmlStrEqual((xmlChar*)namespaceuri.data(), (xmlChar*)DOM_XMLNS_NAMESPACE)) {
      nsp = dom_get_nsdecl(elemp, (xmlChar*)localname.data());
      if (nsp != NULL) {
        return true;
      }
    }
  }
  return false;
}

bool c_domelement::t_removeattribute(CStrRef name) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  xmlNodePtr attrp;
  if (dom_node_is_read_only(nodep)) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  attrp = dom_get_dom1_attribute(nodep, (xmlChar*)name.data());
  if (attrp == NULL) {
    return false;
  }
  switch (attrp->type) {
    case XML_ATTRIBUTE_NODE:
      //if (php_dom_object_get_data(attrp) == NULL) {
      //  node_list_unlink(attrp->children);
      //  xmlUnlinkNode(attrp);
      //  xmlFreeProp((xmlAttrPtr)attrp);
      //} else {
        xmlUnlinkNode(attrp);
      //}
      break;
    case XML_NAMESPACE_DECL:
      return false;
    default:
      break;
  }
  return true;
}

Variant c_domelement::t_removeattributenode(CObjRef oldattr) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  SmartObject<c_domattr> attr = oldattr.getTyped<c_domattr>();
  xmlAttrPtr attrp = (xmlAttrPtr)attr->m_node;
  if (dom_node_is_read_only(nodep)) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  if (attrp->type != XML_ATTRIBUTE_NODE || attrp->parent != nodep) {
    //php_dom_throw_error(NOT_FOUND_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  xmlUnlinkNode((xmlNodePtr)attrp);
  SmartObject<c_domattr> ret = NEW(c_domattr)();
  ret->m_node = (xmlNodePtr)attrp;
  return ret;
}

Variant c_domelement::t_removeattributens(CStrRef namespaceuri, CStrRef localname) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  xmlAttr *attrp;
  xmlNsPtr nsptr;
  if (dom_node_is_read_only(nodep)) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return null;
  }
  attrp = xmlHasNsProp(nodep, (xmlChar*)localname.data(), (xmlChar*)namespaceuri.data());
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
    //if (php_dom_object_get_data((xmlNodePtr) attrp) == NULL) {
    //  node_list_unlink(attrp->children);
    //  xmlUnlinkNode((xmlNodePtr)attrp);
    //  xmlFreeProp(attrp);
    //} else {
      xmlUnlinkNode((xmlNodePtr)attrp);
    //}
  }
  return null;
}

Variant c_domelement::t_setattribute(CStrRef name, CStrRef value) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  xmlNodePtr attr = NULL;
  int name_valid;
  if (name.size() == 0) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attribute Name is required");
    return false;
  }
  name_valid = xmlValidateName((xmlChar*)name.data(), 0);
  if (name_valid != 0) {
    //php_dom_throw_error(INVALID_CHARACTER_ERR, 1 TSRMLS_CC);
    return false;
  }
  if (dom_node_is_read_only(nodep)) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  attr = dom_get_dom1_attribute(nodep, (xmlChar*)name.data());
  if (attr != NULL) {
    switch (attr->type) {
      case XML_ATTRIBUTE_NODE:
        //node_list_unlink(attr->children);
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
    attr = (xmlNodePtr)xmlSetProp(nodep, (xmlChar*)name.data(), (xmlChar*)value.data());
  }
  if (!attr) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "No such attribute '%s'", name);
    return false;
  }
  SmartObject<c_domattr> ret = NEW(c_domattr)();
  ret->m_node = (xmlNodePtr)attr;
  return ret;
}

Variant c_domelement::t_setattributenode(CObjRef newattr) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  SmartObject<c_domattr> domattr = newattr.getTyped<c_domattr>();
  xmlAttrPtr attrp = (xmlAttrPtr)domattr->m_node;
  xmlAttr *existattrp = NULL;
  if (dom_node_is_read_only(nodep)) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  if (attrp->type != XML_ATTRIBUTE_NODE) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attribute node is required");
    return false;
  }
  if (!(attrp->doc == NULL || attrp->doc == nodep->doc)) {
    //php_dom_throw_error(WRONG_DOCUMENT_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  existattrp = xmlHasProp(nodep, attrp->name);
  if (existattrp != NULL && existattrp->type != XML_ATTRIBUTE_DECL) {
    //if ((oldobj = php_dom_object_get_data((xmlNodePtr)existattrp)) != NULL &&
    //  ((php_libxml_node_ptr*)oldobj->ptr)->node == (xmlNodePtr)attrp)
    //{
    //  RETURN_NULL();
    //}
    xmlUnlinkNode((xmlNodePtr)existattrp);
  }
  if (attrp->parent != NULL) {
    xmlUnlinkNode((xmlNodePtr)attrp);
  }
  if (attrp->doc == NULL && nodep->doc != NULL) {
    //attrobj->document = intern->document;
    //php_libxml_increment_doc_ref((php_libxml_node_object *)attrobj, NULL TSRMLS_CC);
  }
  xmlAddChild(nodep, (xmlNodePtr)attrp);
  /* Returns old property if removed otherwise NULL */
  if (existattrp != NULL) {
    SmartObject<c_domattr> ret = NEW(c_domattr)();
    ret->m_node = (xmlNodePtr)existattrp;
    return ret;
  } else {
    return null;
  }
}

Variant c_domelement::t_setattributenodens(CObjRef newattr) {
  throw NotImplementedException(__func__);
  xmlNs *nsp;
  xmlAttr *existattrp = NULL;
  xmlNodePtr nodep = m_node;
  SmartObject<c_domattr> domattr = newattr.getTyped<c_domattr>();
  xmlAttrPtr attrp = (xmlAttrPtr)domattr->m_node;
  if (dom_node_is_read_only(nodep)) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  if (attrp->type != XML_ATTRIBUTE_NODE) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attribute node is required");
    return false;
  }
  if (!(attrp->doc == NULL || attrp->doc == nodep->doc)) {
    //php_dom_throw_error(WRONG_DOCUMENT_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return false;
  }
  nsp = attrp->ns;
  if (nsp != NULL) {
    existattrp = xmlHasNsProp(nodep, nsp->href, attrp->name);
  } else {
    existattrp = xmlHasProp(nodep, attrp->name);
  }
  if (existattrp != NULL && existattrp->type != XML_ATTRIBUTE_DECL) {
    //if ((oldobj = php_dom_object_get_data((xmlNodePtr) existattrp)) != NULL &&·
    //  ((php_libxml_node_ptr *)oldobj->ptr)->node == (xmlNodePtr) attrp)
    //{
    //  RETURN_NULL();
    //}
    xmlUnlinkNode((xmlNodePtr)existattrp);
  }
  if (attrp->parent != NULL) {
    xmlUnlinkNode((xmlNodePtr) attrp);
  }
  if (attrp->doc == NULL && nodep->doc != NULL) {
    //attrobj->document = intern->document;
    //php_libxml_increment_doc_ref((php_libxml_node_object *)attrobj, NULL TSRMLS_CC);
  }
  xmlAddChild(nodep, (xmlNodePtr) attrp);
  /* Returns old property if removed otherwise NULL */
  if (existattrp != NULL) {
    SmartObject<c_domattr> ret = NEW(c_domattr)();
    ret->m_node = (xmlNodePtr)existattrp;
    return ret;
  } else {
    return null;
  }
}

Variant c_domelement::t_setattributens(CStrRef namespaceuri, CStrRef name, CStrRef value) {
  throw NotImplementedException(__func__);
  xmlNodePtr elemp = m_node;
  xmlNsPtr nsptr;
  xmlNode *nodep;
  xmlAttr *attr;
  char *localname = NULL, *prefix = NULL;
  int errorcode = 0, stricterror, is_xmlns = 0, name_valid;
  if (name.size() == 0) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Attribute Name is required");
    return false;
  }
  //stricterror = dom_get_strict_error(intern->document);
  if (dom_node_is_read_only(elemp)) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, stricterror TSRMLS_CC);
    return null;
  }
  errorcode = dom_check_qname((char*)name.data(), &localname, &prefix, namespaceuri.size(), name.size());
  if (errorcode == 0) {
    if (namespaceuri.size() > 0) {
      nodep = (xmlNodePtr)xmlHasNsProp(elemp, (xmlChar*)localname, (xmlChar*)namespaceuri.data());
      if (nodep != NULL && nodep->type != XML_ATTRIBUTE_DECL) {
        node_list_unlink(nodep->children);
      }
      if (xmlStrEqual((xmlChar*)prefix, (xmlChar*)"xmlns") &&
          xmlStrEqual((xmlChar*)namespaceuri.data(), (xmlChar*)DOM_XMLNS_NAMESPACE)) {
        is_xmlns = 1;
        nsptr = dom_get_nsdecl(elemp, (xmlChar*)localname);
      } else {
        nsptr = xmlSearchNsByHref(elemp->doc, elemp, (xmlChar*)namespaceuri.data());
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
            nsptr = dom_get_ns(elemp, (char*)namespaceuri.data(), &errorcode, prefix);
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
        attr = xmlSetNsProp(elemp, nsptr, (xmlChar*)localname, (xmlChar*)value.data());
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
    //php_dom_throw_error(errorcode, stricterror TSRMLS_CC);
  }
  return null;
}

Variant c_domelement::t_setidattribute(CStrRef name, bool isid) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  xmlAttrPtr attrp;
  if (dom_node_is_read_only(nodep)) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return null;
  }
  attrp = xmlHasNsProp(nodep, (xmlChar*)name.data(), NULL);
  if (attrp == NULL || attrp->type == XML_ATTRIBUTE_DECL) {
    //php_dom_throw_error(NOT_FOUND_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
  } else {
    php_set_attribute_id(attrp, isid);
  }
  return null;
}

Variant c_domelement::t_setidattributenode(CObjRef idattr, bool isid) {
  throw NotImplementedException(__func__);
  xmlNodePtr nodep = m_node;
  SmartObject<c_domattr> domattr = idattr.getTyped<c_domattr>();
  xmlAttrPtr attrp = (xmlAttrPtr)domattr->m_node;
  if (dom_node_is_read_only(nodep)) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return null;
  }
  if (attrp->parent != nodep) {
    //php_dom_throw_error(NOT_FOUND_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
  } else {
    php_set_attribute_id(attrp, isid);
  }
  return null;
}

Variant c_domelement::t_setidattributens(CStrRef namespaceuri, CStrRef localname, bool isid) {
  throw NotImplementedException(__func__);
  xmlNodePtr elemp = m_node;
  xmlAttrPtr attrp;
  if (dom_node_is_read_only(elemp)) {
    //php_dom_throw_error(NO_MODIFICATION_ALLOWED_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
    return null;
  }
  attrp = xmlHasNsProp(elemp, (xmlChar*)localname.data(), (xmlChar*)namespaceuri.data());
  if (attrp == NULL || attrp->type == XML_ATTRIBUTE_DECL) {
    //php_dom_throw_error(NOT_FOUND_ERR, dom_get_strict_error(intern->document) TSRMLS_CC);
  } else {
    php_set_attribute_id(attrp, isid);
  }
  return null;
}

c_domentity::c_domentity() {
}

c_domentity::~c_domentity() {
}

void c_domentity::t___construct() {
  throw NotImplementedException(__func__);
}

Variant c_domentity::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

c_domentityreference::c_domentityreference() {
}

c_domentityreference::~c_domentityreference() {
}

void c_domentityreference::t___construct(CVarRef name) {
  throw NotImplementedException(__func__);
}

Variant c_domentityreference::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

c_domnotation::c_domnotation() {
}

c_domnotation::~c_domnotation() {
}

void c_domnotation::t___construct() {
  throw NotImplementedException(__func__);
}

Variant c_domnotation::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

c_domprocessinginstruction::c_domprocessinginstruction() {
}

c_domprocessinginstruction::~c_domprocessinginstruction() {
}

void c_domprocessinginstruction::t___construct(CVarRef name, CVarRef value /*= null_variant*/) {
  throw NotImplementedException(__func__);
}

Variant c_domprocessinginstruction::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

c_domnamednodemap::c_domnamednodemap() {
}

c_domnamednodemap::~c_domnamednodemap() {
}

void c_domnamednodemap::t___construct() {
  throw NotImplementedException(__func__);
}

Variant c_domnamednodemap::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

Variant c_domnamednodemap::t_getnameditem(CStrRef name) {
  throw NotImplementedException(__func__);
}

Variant c_domnamednodemap::t_getnameditemns(CStrRef namespaceuri, CStrRef localname) {
  throw NotImplementedException(__func__);
}

Object c_domnamednodemap::t_item(int64 index) {
  throw NotImplementedException(__func__);
}

c_domnodelist::c_domnodelist() {
}

c_domnodelist::~c_domnodelist() {
}

void c_domnodelist::t___construct() {
  throw NotImplementedException(__func__);
}

Variant c_domnodelist::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

Object c_domnodelist::t_item(int64 index) {
  throw NotImplementedException(__func__);
}

c_domexception::c_domexception() {
}

c_domexception::~c_domexception() {
}

void c_domexception::t___construct() {
  throw NotImplementedException(__func__);
}

Variant c_domexception::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

c_domimplementation::c_domimplementation() {
}

c_domimplementation::~c_domimplementation() {
}

void c_domimplementation::t___construct() {
  throw NotImplementedException(__func__);
}

Variant c_domimplementation::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

Variant c_domimplementation::t_createdocument(CStrRef namespaceuri /* = null_string */, CStrRef qualifiedname /* = null_string */, CObjRef doctypeobj /* = null_object */) {
  throw NotImplementedException(__func__);
  xmlDoc *docp;
  xmlNode *nodep;
  xmlNsPtr nsptr = NULL;
  int errorcode = 0;
  char *prefix = NULL, *localname = NULL;
  xmlDtdPtr doctype = NULL;
  if (!doctypeobj.isNull()) {
    SmartObject<c_domdocumenttype> domdoctype = doctypeobj.getTyped<c_domdocumenttype>();
    doctype = (xmlDtdPtr)domdoctype->m_node;
    if (doctype->type == XML_DOCUMENT_TYPE_NODE) {
      //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid DocumentType object");
      return false;
    }
    if (doctype->doc != NULL) {
      //php_dom_throw_error(WRONG_DOCUMENT_ERR, 1 TSRMLS_CC);
      return false;
    }
  } else {
    //doctobj = NULL;
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
    //php_dom_throw_error(errorcode, 1 TSRMLS_CC);
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
      //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unexpected Error");
      return false;
    }
    nodep->nsDef = nsptr;
    xmlDocSetRootElement(docp, nodep);
    xmlFree(localname);
  }
  SmartObject<c_domdocument> ret = NEW(c_domdocument)();
  ret->m_node = (xmlNodePtr)docp;
  //if (doctobj != NULL) {
    //doctobj->document = ((dom_object *)((php_libxml_node_ptr *)docp->_private)->_private)->document;
    //php_libxml_increment_doc_ref((php_libxml_node_object *)doctobj, docp TSRMLS_CC);
  //}
  return ret;
}

Variant c_domimplementation::t_createdocumenttype(CStrRef qualifiedname /* = null_string */, CStrRef publicid /* = null_string */, CStrRef systemid /* = null_string */) {
  throw NotImplementedException(__func__);
  xmlDtd *doctype;
  xmlChar *pch1 = NULL, *pch2 = NULL, *localname = NULL;
  xmlURIPtr uri;
  if (qualifiedname.size() == 0) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "qualifiedName is required");
    return false;
  }
  if (publicid.size() > 0)
    pch1 = (xmlChar*)publicid.data();
  if (systemid.size() > 0)
    pch2 = (xmlChar*)systemid.data();
  uri = xmlParseURI((char*)qualifiedname.data());
  if (uri != NULL && uri->opaque != NULL) {
    localname = xmlStrdup((xmlChar*)uri->opaque);
    if (xmlStrchr(localname, (xmlChar)':') != NULL) {
      //php_dom_throw_error(NAMESPACE_ERR, 1 TSRMLS_CC);
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
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to create DocumentType");
    return false;
  }
  SmartObject<c_domnode> ret = NEW(c_domnode)();
  ret->m_node = (xmlNodePtr)doctype;
  return ret;
}

bool c_domimplementation::t_hasfeature(CStrRef feature, CStrRef version) {
  throw NotImplementedException(__func__);
  if (dom_has_feature((char*)feature.data(), (char*)version.data())) {
    return true;
  } else {
    return false;
  }
}

c_domxpath::c_domxpath() {
}

c_domxpath::~c_domxpath() {
}

void c_domxpath::t___construct(CVarRef doc) {
  throw NotImplementedException(__func__);
}

Variant c_domxpath::t___destruct() {
  throw NotImplementedException(__func__);
  return null;
}

Variant c_domxpath::t_evaluate(CStrRef expr, CObjRef context /* = null_object */) {
  throw NotImplementedException(__func__);
  return php_xpath_eval(this, expr, context, PHP_DOM_XPATH_EVALUATE);
}

Variant c_domxpath::t_query(CStrRef expr, CObjRef context /* = null_object */) {
  throw NotImplementedException(__func__);
  return php_xpath_eval(this, expr, context, PHP_DOM_XPATH_QUERY);
}

bool c_domxpath::t_registernamespace(CStrRef prefix, CStrRef uri) {
  throw NotImplementedException(__func__);
  xmlXPathContextPtr ctxp = (xmlXPathContextPtr)m_node;
  if (ctxp == NULL) {
    //php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid XPath Context");
    return false;
  }
  if (xmlXPathRegisterNs(ctxp, (xmlChar*)prefix.data(), (xmlChar*)uri.data()) != 0) {
    return false;
  }
  return true;
}

Variant c_domxpath::t_registerphpfunctions(CVarRef funcs /* = null */) {
  throw NotImplementedException(__func__);
}


///////////////////////////////////////////////////////////////////////////////

}
