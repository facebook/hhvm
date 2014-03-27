
#include "hphp/runtime/ext/ext_xsltprocessor.h"
#include "hphp/runtime/ext/ext_domdocument.h"
#include "hphp/runtime/ext/ext_simplexml.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const StaticString s_xslt_dotted_version("LIBXSLT_DOTTED_VERSION");
const StaticString s_xslt_dotted_version_value(LIBXSLT_DOTTED_VERSION);

class XslExtension : public Extension {
 public:
  XslExtension() : Extension("xsl", "0.1") { }
  void moduleInit() {
    Native::registerConstant<KindOfString>(s_xslt_dotted_version.get(),
                                           s_xslt_dotted_version_value.get());
  }
};
XslExtension s_xsl_extension;

static xmlChar *xslt_string_to_xpathexpr(const char *str) {
  const xmlChar *string = (const xmlChar*)str;
  int str_len = xmlStrlen(string) + 3;

  xmlChar *value;
  if (xmlStrchr(string, '"')) {
    if (xmlStrchr(string, '\'')) {
      raise_warning("Cannot create XPath expression (string contains both "
                    "quote and double-quotes)");
      return nullptr;
    }

    value = (xmlChar*)malloc(str_len * sizeof(xmlChar));
    snprintf((char*)value, str_len, "'%s'", string);
  } else {
    value = (xmlChar*)malloc(str_len * sizeof(xmlChar));
    snprintf((char*)value, str_len, "\"%s\"", string);
  }

  return value;
}

static String xslt_get_valid_file_path(const String& source) {
  String translated;

  xmlURIPtr uri = xmlCreateURI();
  xmlChar *escsource = xmlURIEscapeStr((xmlChar*)source.c_str(), (xmlChar*)":");
  xmlParseURIReference(uri, (char*)escsource);
  xmlFree(escsource);
  if (uri->scheme != nullptr) {
    if (strncasecmp(source.c_str(), "file:///", 8) == 0) {
      translated = File::TranslatePath(String(source.substr(7), CopyString));
    } else if (strncasecmp(source.c_str(), "file://localhost/", 17) == 0) {
      translated = File::TranslatePath(String(source.substr(16), CopyString));
    }
  } else {
    translated = File::TranslatePath(String (source, CopyString));
  }
  xmlFreeURI(uri);

  return translated;
}

static void xslt_ext_function_php(xmlXPathParserContextPtr ctxt,
                                  int nargs,
                                  int type) {
  c_XSLTProcessor *intern = nullptr;
  int error = 0;

  xsltTransformContextPtr tctxt = xsltXPathGetTransformContext (ctxt);
  if (tctxt == nullptr) {
    xsltGenericError(xsltGenericErrorContext,
      "xsltExtFunctionTest: failed to get the transformation context\n"
    );
    error = 1;
  } else {
    intern = (c_XSLTProcessor*)tctxt->_private;
    if (intern == nullptr) {
      xsltGenericError(xsltGenericErrorContext,
        "xsltExtFunctionTest: failed to get the internal object\n"
      );
      error = 1;
    } else {
      if (intern->m_registerPhpFunctions == 0) {
        xsltGenericError(xsltGenericErrorContext,
          "xsltExtFunctionTest: PHP Object did not register PHP functions\n"
        );
        error = 1;
      }
    }
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
  // Reverse order to pop values off ctxt stack
  for (int i = nargs - 2; i >= 0; i--) {
    Variant arg;
    obj = valuePop(ctxt);
    switch (obj->type) {
    case XPATH_STRING:
      arg = String((char*)obj->stringval, CopyString);
      break;
    case XPATH_BOOLEAN:
      arg = (bool)obj->boolval;
      break;
    case XPATH_NUMBER:
      arg = (double)obj->floatval;
      break;
    case XPATH_NODESET:
      if (type == 1) {
        char *str = (char*)xmlXPathCastToString(obj);
        arg = String(str, CopyString);
        xmlFree(str);
      } else if (type == 2) {
        arg = Array::Create();
        if (obj->nodesetval && obj->nodesetval->nodeNr > 0) {
          for (int j = 0; j < obj->nodesetval->nodeNr; j++) {
            // TODO: not sure this is the right thing to do.
            xmlNodePtr node = obj->nodesetval->nodeTab[j];

            if (node->type == XML_ELEMENT_NODE) {
              c_DOMElement *elemobj = NEWOBJ(c_DOMElement)();
              elemobj->m_node = xmlCopyNode(node, /*extended*/ 1);
              arg.toArrRef().append(elemobj);
            } else if (node->type == XML_ATTRIBUTE_NODE) {
              c_DOMAttr *attrobj = NEWOBJ(c_DOMAttr)();
              attrobj->m_node = (xmlNodePtr)xmlCopyProp(nullptr,
                                                        (xmlAttrPtr)node);
              arg.toArrRef().append(attrobj);
            } else if (node->type == XML_TEXT_NODE) {
              c_DOMText *textobj = NEWOBJ(c_DOMText)();
              textobj->m_node = (xmlNodePtr)xmlNewText(xmlNodeGetContent(node));
              arg.toArrRef().append(textobj);
            } else {
              raise_warning("Unhandled node type '%d'", node->type);
              // Use a generic DOMNode as fallback for now.
              c_DOMNode *nodeobj = NEWOBJ(c_DOMNode)();
              nodeobj->m_node = xmlCopyNode(node, /*extended*/ 1);
              arg.toArrRef().append(nodeobj);
            }
          }
        }
      }
      break;
    default:
      arg = String((char*)xmlXPathCastToString(obj), CopyString);
    }
    xmlXPathFreeObject(obj);
    args.prepend(arg);
  }

  obj = valuePop(ctxt);
  if (obj->stringval == nullptr) {
    raise_warning("Handler name must be a string");
    xmlXPathFreeObject(obj);
    // Push an empty string to get an xslt result.
    valuePush(ctxt, xmlXPathNewString((xmlChar*)""));
    return;
  }
  String handler((char*)obj->stringval, CopyString);
  xmlXPathFreeObject(obj);

  if (!f_is_callable(handler)) {
    raise_warning("Unable to call handler %s()", handler.data());
    // Push an empty string to get an xslt result.
    valuePush(ctxt, xmlXPathNewString((xmlChar*)""));
  } else if (intern->m_registerPhpFunctions == 2 &&
             !intern->m_registered_phpfunctions.exists(handler)) {
    raise_warning("Not allowed to call handler '%s()'", handler.data());
    // Push an empty string to get an xslt result.
    valuePush(ctxt, xmlXPathNewString((xmlChar*)""));
  } else {
    Variant retval = vm_call_user_func(handler, args);
    if (retval.isObject() &&
        retval.getObjectData()->instanceof(c_DOMNode::classof())) {
      xmlNode *nodep = retval.asCObjRef().getTyped<c_DOMNode>()->m_node;
      valuePush(ctxt, xmlXPathNewNodeSet(nodep));
    } else if (retval.is(KindOfBoolean)) {
      valuePush(ctxt, xmlXPathNewBoolean(retval.toBoolean()));
    } else if (retval.isObject()) {
      raise_warning("A PHP Object cannot be converted to an XPath-string");
      // Push an empty string to get an xslt result.
      valuePush(ctxt, xmlXPathNewString((xmlChar*)""));
    } else {
      String sretval = retval.toString();
      valuePush(ctxt, xmlXPathNewString((xmlChar*)sretval.data()));
    }
  }
}

static void xslt_ext_function_string_php(xmlXPathParserContextPtr ctxt,
                                         int nargs) {
  xslt_ext_function_php(ctxt, nargs, 1);
}

static void xslt_ext_function_object_php(xmlXPathParserContextPtr ctxt,
                                         int nargs) {
  xslt_ext_function_php(ctxt, nargs, 2);
}

c_XSLTProcessor::c_XSLTProcessor(Class *cb) :
  ExtObjectData(cb),
  m_stylesheet(nullptr), m_doc(nullptr), m_secprefs(k_XSL_SECPREF_DEFAULT),
  m_registerPhpFunctions(0) {
  xsltSetGenericErrorFunc(nullptr, php_libxml_ctx_error);
  exsltRegisterAll();
}

c_XSLTProcessor::~c_XSLTProcessor() {
  sweep();
}

void c_XSLTProcessor::sweep() {
  if (m_stylesheet) {
    xsltFreeStylesheet(m_stylesheet);
    m_stylesheet = nullptr;
  }

  if (m_doc) {
    xmlFreeDoc(m_doc);
    m_doc = nullptr;
  }
}

void c_XSLTProcessor::t___construct() {
  if (m_params.empty()) {
    m_params = Array::Create();
  }

  if (m_registered_phpfunctions.empty()) {
    m_registered_phpfunctions = Array::Create();
  }
}

Variant c_XSLTProcessor::t_getparameter(const String& namespaceURI,
                                        const String& localName) {
  // namespaceURI argument is unused in Zend PHP XSL extension.
  if (m_params.exists(localName)) {
    assert(m_params[localName].isString());
    return m_params[localName].toString();
  }

  return false;
}

int64_t c_XSLTProcessor::t_getsecurityprefs() {
  return m_secprefs;
}

bool c_XSLTProcessor::t_hasexsltsupport() {
  return true;
}

void c_XSLTProcessor::t_importstylesheet(const Object& stylesheet) {
  xmlDocPtr doc = nullptr;

  if (stylesheet.instanceof(c_DOMDocument::classof())) {
    c_DOMDocument *domdoc = stylesheet.getTyped<c_DOMDocument>();
    // This doc will be freed by xsltFreeStylesheet.
    doc = xmlCopyDoc((xmlDocPtr)domdoc->m_node, /*recursive*/ 1);
    if (doc == nullptr) {
      raise_error("Unable to import stylesheet");
    }
  } else if (stylesheet.instanceof(c_SimpleXMLElement::classof())) {
    c_SimpleXMLElement *elem = stylesheet.getTyped<c_SimpleXMLElement>();
    // This doc will be freed by xsltFreeStylesheet.
    doc = xmlNewDoc((const xmlChar*)"1.0");
    xmlNodePtr node = xmlCopyNode(elem->node, /*extended*/ 1);
    if (doc == nullptr || node == nullptr) {
      raise_error("Unable to import stylesheet");
    }
    xmlDocSetRootElement(doc, node);
  } else {
    raise_error("Object must be an instance of DOMDocument or "
                "SimpleXMLElement");
  }

  if (doc) {
    m_stylesheet = xsltParseStylesheetDoc(doc);
    if (m_stylesheet == nullptr) {
      raise_error("Unable to import stylesheet");
    }
  }
}

bool c_XSLTProcessor::t_removeparameter(const String& namespaceURI,
                                        const String& localName) {
  // namespaceURI argument is unused in Zend PHP XSL extension.
  if (m_params.exists(localName)) {
    assert(m_params[localName].isString());
    m_params.remove(localName);

    return true;
  }

  return false;
}

void c_XSLTProcessor::t_registerphpfunctions(
    const Variant& funcs /*=null_variant */) {
  if (funcs.isNull()) {
    // Register all available PHP functions.
    m_registerPhpFunctions = 1;
  } else if (funcs.isArray()) {
    Array arr = funcs.toArray();
    for (ArrayIter iter(arr); iter; ++iter) {
      if (iter.second().isString()) {
        m_registered_phpfunctions.set(iter.second(), "1");
      } else {
        raise_warning("PHP function name must be a string");
      }
    }
    // Register PHP functions given in array argument.
    m_registerPhpFunctions = 2;
  } else if (funcs.isString()) {
    m_registered_phpfunctions.set(funcs, "1");
    // Register PHP function given in string argument.
    m_registerPhpFunctions = 2;
  } else {
    raise_warning("PHP function name must be a string");
  }
}

bool c_XSLTProcessor::t_setparameter(const String& namespaceURI,
                                     const Variant& localName,
                                     const Variant& value /*=null_variant */) {
  // namespaceURI argument is unused in Zend PHP XSL extension.
  if (localName.isString() && value.isString()) {
    if (m_params.exists(localName)) {
      m_params.set(localName, value);
    } else {
      m_params.add(localName, value);
    }

    return true;
  } else if (localName.isArray() && value.isNull()) {
    int ret = true;
    for (ArrayIter iter(localName); iter; ++iter) {
      if (iter.first().isString() && iter.second().isString()) {
        if (m_params.exists(iter.first().toString())) {
          m_params.set(iter.first().toString(), iter.second().toString());
        } else {
          m_params.add(iter.first().toString(), iter.second().toString());
        }
      } else {
        ret = false;
      }
    }

    if (ret == false) {
      raise_warning("Invalid parameter array");
    }
    return ret;
  }

  if (localName.isString()) {
    raise_warning("Invalid parameter '%s': value must be a string",
                  localName.toString().c_str());
  } else {
    raise_warning ("Invalid parameter: localName and value must be strings");
  }

  return false;
}

int64_t c_XSLTProcessor::t_setsecurityprefs(int64_t securityPrefs) {
  int64_t previous = m_secprefs;
  m_secprefs = securityPrefs;

  return previous;
}

bool c_XSLTProcessor::t_setprofiling(const String& filename) {
  if (filename.length() > 0) {
    String translated = File::TranslatePath(filename);
    Stream::Wrapper* w = Stream::getWrapperFromURI(translated);
    if (w->access(translated, W_OK)) {
      m_profile = translated;
      return true;
    }
  }

  return false;
}

Variant c_XSLTProcessor::t_transformtodoc(const Object& doc) {
  if (doc.instanceof(c_DOMNode::classof())) {
    c_DOMNode *domnode = doc.getTyped<c_DOMNode>();
    m_doc = xmlCopyDoc((xmlDocPtr)domnode->m_node, /*recursive*/ 1);

    c_DOMDocument *res = NEWOBJ(c_DOMDocument)();
    res->m_node = (xmlNodePtr)apply_stylesheet();
    res->m_owner = true;

    return res;
  }

  return false;
}

Variant c_XSLTProcessor::t_transformtouri(const Object& doc,
                                          const String& uri) {
  if (doc.instanceof(c_DOMDocument::classof())) {
    c_DOMDocument *domdoc = doc.getTyped<c_DOMDocument>();
    m_doc = xmlCopyDoc ((xmlDocPtr)domdoc->m_node, /*recursive*/ 1);

    String translated = xslt_get_valid_file_path(uri);
    if (translated.empty()) {
      raise_warning("Invalid URI");
      return false;
    }

    xmlDocPtr res = apply_stylesheet ();
    if (res == nullptr) {
      return false;
    }

    int bytes = xsltSaveResultToFilename(translated.data(),
                                         res,
                                         m_stylesheet,
                                         /*compression*/ 0);
    xmlFreeDoc(res);

    if (bytes == -1) {
      return false;
    }

    return bytes;
  }

  return false;
}

Variant c_XSLTProcessor::t_transformtoxml(const Object& doc) {
  if (doc.instanceof(c_DOMDocument::classof())) {
    c_DOMDocument *domdoc = doc.getTyped<c_DOMDocument>();
    m_doc = xmlCopyDoc ((xmlDocPtr)domdoc->m_node, /*recursive*/ 1);

    xmlDocPtr res = apply_stylesheet();
    if (res == nullptr) {
      return false;
    }

    xmlChar *mem;
    int size;
    if (xsltSaveResultToString(&mem, &size, res, m_stylesheet) < 0) {
      if (mem) {
        xmlFree(mem);
      }
      return false;
    }

    String ret = String((char*)mem, size, CopyString);
    xmlFree(mem);
    return ret;
  }

  return false;
}

xmlDocPtr c_XSLTProcessor::apply_stylesheet() {
  SYNC_VM_REGS_SCOPED();

  if (m_stylesheet == nullptr || m_doc == nullptr) {
    raise_error("Unable to apply stylesheet");
    return nullptr;
  }

  xsltTransformContextPtr ctxt = xsltNewTransformContext (m_stylesheet, m_doc);
  ctxt->_private = this;

  xsltSecurityPrefsPtr prefs = nullptr;
  if (m_secprefs != k_XSL_SECPREF_NONE) {
    prefs = xsltNewSecurityPrefs();
    int error = 0;

    if (m_secprefs & k_XSL_SECPREF_READ_FILE) {
      if (xsltSetSecurityPrefs(prefs,
                               XSLT_SECPREF_READ_FILE,
                               xsltSecurityForbid) != 0) {
        error = 1;
      }
    }

    if (m_secprefs & k_XSL_SECPREF_WRITE_FILE) {
      if (xsltSetSecurityPrefs(prefs,
                               XSLT_SECPREF_WRITE_FILE,
                               xsltSecurityForbid) != 0) {
        error = 1;
      }
    }

    if (m_secprefs & k_XSL_SECPREF_CREATE_DIRECTORY) {
      if (xsltSetSecurityPrefs(prefs,
                               XSLT_SECPREF_CREATE_DIRECTORY,
                               xsltSecurityForbid) != 0) {
        error = 1;
      }
    }

    if (m_secprefs & k_XSL_SECPREF_READ_NETWORK) {
      if (xsltSetSecurityPrefs(prefs,
                               XSLT_SECPREF_READ_NETWORK,
                               xsltSecurityForbid) != 0) {
        error = 1;
      }
    }

    if (m_secprefs & k_XSL_SECPREF_WRITE_NETWORK) {
      if (xsltSetSecurityPrefs(prefs,
                               XSLT_SECPREF_WRITE_NETWORK,
                               xsltSecurityForbid) != 0) {
        error = 1;
      }
    }

    if (xsltSetCtxtSecurityPrefs(prefs, ctxt) != 0) {
      error = 1;
    }

    if (error == 1) {
      raise_error("Can't set libxslt security properties, not doing "
                  "transformation for security reasons");
      return nullptr;
    }
  }

  xsltRegisterExtFunction(ctxt,
    (const xmlChar*) "functionString",
    (const xmlChar*) "http://php.net/xsl",
    xslt_ext_function_string_php
  );

  xsltRegisterExtFunction(ctxt,
    (const xmlChar*) "function",
    (const xmlChar*) "http://php.net/xsl",
    xslt_ext_function_object_php
  );

  for (ArrayIter iter(m_params); iter; ++iter) {
    assert(iter.first().isString());
    assert(iter.second().isString());

    xmlChar *value = xslt_string_to_xpathexpr(iter.second().toString().c_str());
    if (value) {
      xsltEvalOneUserParam(ctxt,
        (const xmlChar*)iter.first().toString().c_str(),
        (const xmlChar*)value
      );

      xmlFree(value);
    }
  }

  FILE *profile = nullptr;
  if (m_profile) {
    profile = fopen(m_profile.data(), "w");
  }

  xmlDocPtr res = xsltApplyStylesheetUser(m_stylesheet,
                                          m_doc,
                                          nullptr,
                                          nullptr,
                                          profile,
                                          ctxt);

  if (profile) {
    fclose(profile);
  }

  xsltFreeTransformContext(ctxt);

  if (prefs) {
    xsltFreeSecurityPrefs(prefs);
  }

  return res;
}

///////////////////////////////////////////////////////////////////////////////
}
