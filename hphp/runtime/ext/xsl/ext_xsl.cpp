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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/stream-wrapper-registry.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/ext/simplexml/ext_simplexml.h"
#include "hphp/runtime/ext/domdocument/ext_domdocument.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/libxml/ext_libxml.h"
#include "hphp/util/string-vsnprintf.h"

#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/extensions.h>
#include <libxslt/variables.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libxslt/security.h>
#include <libexslt/exslt.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// constants

const int64_t k_XSL_SECPREF_NONE              = 0;
const int64_t k_XSL_SECPREF_READ_FILE         = 2;
const int64_t k_XSL_SECPREF_WRITE_FILE        = 4;
const int64_t k_XSL_SECPREF_CREATE_DIRECTORY  = 8;
const int64_t k_XSL_SECPREF_READ_NETWORK      = 16;
const int64_t k_XSL_SECPREF_WRITE_NETWORK     = 32;
const int64_t k_XSL_SECPREF_DEFAULT           = 44;

///////////////////////////////////////////////////////////////////////////////
// helpers

static xmlChar *xslt_string_to_xpathexpr(const char *);
static void xslt_ext_function_php(xmlXPathParserContextPtr, int, int);
static void xslt_ext_function_string_php(xmlXPathParserContextPtr, int);
static void xslt_ext_function_object_php(xmlXPathParserContextPtr, int);
static void xslt_ext_error_handler(void *, const char *, ...)
  ATTRIBUTE_PRINTF(2,3);

///////////////////////////////////////////////////////////////////////////////
// NativeData

const StaticString
  s_XSLTProcessorData("XSLTProcessorData"),
  s_DOMDocument("DOMDocument"),
  s_DOMElement("DOMElement"),
  s_DOMAttr("DOMAttr"),
  s_DOMText("DOMText"),
  s_DOMNode("DOMNode");

struct XSLTProcessorData {
  XSLTProcessorData() : m_stylesheet(nullptr), m_doc(nullptr),
  m_secprefs(k_XSL_SECPREF_DEFAULT),
  m_registerPhpFunctions(0) {
    if (m_params.empty()) {
      m_params = Array::Create();
    }

    if (m_registered_phpfunctions.empty()) {
      m_registered_phpfunctions = Array::Create();
    }
  };

  ~XSLTProcessorData() {
    sweep();
  }

  void sweep() {
    if (m_stylesheet) {
      xsltFreeStylesheet(m_stylesheet);
      m_stylesheet = nullptr;
    }
  }

  xmlDocPtr doc() { return m_doc ? m_doc->docp() : nullptr; }

  xsltStylesheetPtr m_stylesheet;
  XMLNode m_doc;
  Array m_params;
  int m_secprefs;
  int m_registerPhpFunctions;
  Array m_registered_phpfunctions;
  String m_profile;

  Array m_usedElements; // don't let DOMElements get free'd out from under us
                        // while preparing to create a new document

  xmlDocPtr apply_stylesheet();
};

xmlDocPtr XSLTProcessorData::apply_stylesheet() {
  SYNC_VM_REGS_SCOPED();

  if (m_stylesheet == nullptr || doc() == nullptr) {
    raise_error("Unable to apply stylesheet");
    return nullptr;
  }

  xsltTransformContextPtr ctxt = xsltNewTransformContext (m_stylesheet, doc());
  if (ctxt == nullptr) {
    raise_error("Unable to apply stylesheet");
    return nullptr;
  }

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

  assert(m_usedElements.empty());
  xmlDocPtr res = xsltApplyStylesheetUser(m_stylesheet,
                                          doc(),
                                          nullptr,
                                          nullptr,
                                          profile,
                                          ctxt);
  m_usedElements.clear(); // safe to clear used elements after we've forked
                          // the document

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
// helpers

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

static Object newNode(const String name, xmlNodePtr obj) {
  auto const cls = Unit::lookupClass(name.get());
  Object ret{cls};
  auto retData = Native::data<DOMNode>(ret);
  retData->setNode(obj);
  return ret;
}

static void xslt_ext_function_php(xmlXPathParserContextPtr ctxt,
                                  int nargs,
                                  int type) {
  XSLTProcessorData *intern = nullptr;
  int error = 0;

  xsltTransformContextPtr tctxt = xsltXPathGetTransformContext (ctxt);
  if (tctxt == nullptr) {
    xsltGenericError(xsltGenericErrorContext,
      "xsltExtFunctionTest: failed to get the transformation context\n"
    );
    error = 1;
  } else {
    intern = (XSLTProcessorData*)tctxt->_private;
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
              Object element = newNode(s_DOMElement,
                                       xmlCopyNode(node, /*extended*/ 1));
              arg.toArrRef().append(element);
            } else if (node->type == XML_ATTRIBUTE_NODE) {
              Object attribute =
                newNode(s_DOMAttr,
                        (xmlNodePtr)xmlCopyProp(nullptr, (xmlAttrPtr)node));
              arg.toArrRef().append(attribute);
            } else if (node->type == XML_TEXT_NODE) {
              Object text =
                newNode(s_DOMText,
                        (xmlNodePtr)xmlNewText(xmlNodeGetContent(node)));
              arg.toArrRef().append(text);
            } else {
              raise_warning("Unhandled node type '%d'", node->type);
              // Use a generic DOMNode as fallback for now.
              Object nodeobj = newNode(s_DOMNode,
                                       xmlCopyNode(node, /*extended*/ 1));
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

  if (!is_callable(handler)) {
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
        retval.getObjectData()->instanceof(s_DOMNode)) {
      ObjectData *retval_data = retval.asCObjRef().get();
      xmlNode* nodep = Native::data<DOMNode>(retval_data)->nodep();
      valuePush(ctxt, xmlXPathNewNodeSet(nodep));
      intern->m_usedElements.prepend(retval);
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

static void xslt_ext_error_handler(void *ctx,
                                   const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  try {
    std::string msg;
    string_vsnprintf(msg, fmt, args);

    /* remove any trailing \n */
    while (!msg.empty() && msg[msg.size() - 1] == '\n') {
      msg = msg.substr(0, msg.size() - 1);
    }

    raise_warning("%s", msg.c_str());
  } catch (...) {}
  va_end(args);
}

///////////////////////////////////////////////////////////////////////////////
// methods

static Variant HHVM_METHOD(XSLTProcessor, getParameter,
                        const Variant& namespaceURI,
                        const String& localName) {
  auto data = Native::data<XSLTProcessorData>(this_);

  // namespaceURI argument is unused in PHP5 XSL extension.
  if (data->m_params.exists(localName)) {
    assert(data->m_params[localName].isString());
    return data->m_params[localName].toString();
  }

  return false;
}

static int64_t HHVM_METHOD(XSLTProcessor, getSecurityPrefs) {
  auto data = Native::data<XSLTProcessorData>(this_);
  return data->m_secprefs;
}

static void HHVM_METHOD(XSLTProcessor, importStylesheet,
                        const Object& stylesheet) {
  SYNC_VM_REGS_SCOPED();

  auto data = Native::data<XSLTProcessorData>(this_);
  xmlDocPtr doc = nullptr;

  if (stylesheet.instanceof(s_DOMDocument)) {
    auto domdoc = Native::data<DOMNode>(stylesheet);
    // This doc will be freed by xsltFreeStylesheet.
    doc = xmlCopyDoc((xmlDocPtr)domdoc->nodep(), /*recursive*/ 1);
    if (doc == nullptr) {
      raise_error("Unable to import stylesheet");
    }
  } else if (stylesheet.instanceof(c_SimpleXMLElement::classof())) {
    auto elem = cast<c_SimpleXMLElement>(stylesheet);
    // This doc will be freed by xsltFreeStylesheet.
    doc = xmlNewDoc((const xmlChar*)"1.0");
    xmlNodePtr node = xmlCopyNode(elem->nodep(), /*extended*/ 1);
    if (doc == nullptr || node == nullptr) {
      raise_error("Unable to import stylesheet");
    }
    xmlDocSetRootElement(doc, node);
  } else {
    raise_error("Object must be an instance of DOMDocument or "
                "SimpleXMLElement");
  }

  if (doc) {
    data->m_stylesheet = xsltParseStylesheetDoc(doc);
    if (data->m_stylesheet == nullptr) {
      xmlFreeDoc(doc);
      raise_error("Unable to import stylesheet");
    }
  }
}

static bool HHVM_METHOD(XSLTProcessor, removeParameter,
                        const Variant& namespaceURI,
                        const String& localName) {
  auto data = Native::data<XSLTProcessorData>(this_);

  // namespaceURI argument is unused in PHP5 XSL extension.
  if (data->m_params.exists(localName)) {
    assert(data->m_params[localName].isString());
    data->m_params.remove(localName);

    return true;
  }

  return false;
}

static void HHVM_METHOD(XSLTProcessor, registerPHPFunctions,
                        const Variant& funcs /*= null_variant*/) {
  auto data = Native::data<XSLTProcessorData>(this_);

  if (funcs.isNull()) {
    // Register all available PHP functions.
    data->m_registerPhpFunctions = 1;
  } else if (funcs.isArray()) {
    Array arr = funcs.toArray();
    for (ArrayIter iter(arr); iter; ++iter) {
      if (iter.second().isString()) {
        data->m_registered_phpfunctions.set(iter.second(), "1");
      } else {
        raise_warning("PHP function name must be a string");
      }
    }
    // Register PHP functions given in array argument.
    data->m_registerPhpFunctions = 2;
  } else if (funcs.isString()) {
    data->m_registered_phpfunctions.set(funcs, "1");
    // Register PHP function given in string argument.
    data->m_registerPhpFunctions = 2;
  } else {
    raise_warning("PHP function name must be a string");
  }
}

static bool HHVM_METHOD(XSLTProcessor, setParameter,
                        const Variant& namespaceURI,
                        const Variant& localName,
                        const Variant& value /*= null_variant*/) {
  auto data = Native::data<XSLTProcessorData>(this_);

  // namespaceURI argument is unused in PHP5 XSL extension.
  if (localName.isString() && value.isString()) {
    if (data->m_params.exists(localName)) {
      data->m_params.set(localName, value);
    } else {
      data->m_params.add(localName, value);
    }

    return true;
  } else if (localName.isArray() && value.isNull()) {
    int ret = true;
    for (ArrayIter iter(localName); iter; ++iter) {
      if (iter.first().isString() && iter.second().isString()) {
        if (data->m_params.exists(iter.first().toString())) {
          data->m_params.set(iter.first().toString(), iter.second().toString());
        } else {
          data->m_params.add(iter.first().toString(), iter.second().toString());
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

static int64_t HHVM_METHOD(XSLTProcessor, setSecurityPrefs,
                           int64_t securityPrefs) {
  auto data = Native::data<XSLTProcessorData>(this_);

  int64_t previous = data->m_secprefs;
  data->m_secprefs = securityPrefs;

  return previous;
}

static bool HHVM_METHOD(XSLTProcessor, setProfiling,
                        const String& filename) {
  auto data = Native::data<XSLTProcessorData>(this_);

  if (filename.length() > 0) {
    String translated = File::TranslatePath(filename);
    Stream::Wrapper* w = Stream::getWrapperFromURI(translated);
    if (!w) return false;
    if (w->access(translated, W_OK)) {
      data->m_profile = translated;
      return true;
    }
  }

  return false;
}

static Variant HHVM_METHOD(XSLTProcessor, transformToDoc,
                           const Object& doc) {
  auto data = Native::data<XSLTProcessorData>(this_);

  if (doc.instanceof(s_DOMNode)) {
    auto domnode = Native::data<DOMNode>(doc);
    data->m_doc =
      libxml_register_node(xmlCopyDoc((xmlDocPtr)domnode->nodep(),
                                      /*recursive*/ 1));

    auto ret = newDOMDocument(false /* construct */);
    DOMNode* doc_data = Native::data<DOMNode>(ret);
    doc_data->setNode((xmlNodePtr)data->apply_stylesheet());

    return ret;
  }

  return false;
}

static Variant HHVM_METHOD(XSLTProcessor, transformToURI,
                           const Object& doc,
                           const String& uri) {
  auto data = Native::data<XSLTProcessorData>(this_);

  if (doc.instanceof(s_DOMDocument)) {
    auto domdoc = Native::data<DOMNode>(doc);
    data->m_doc =
      libxml_register_node(xmlCopyDoc ((xmlDocPtr)domdoc->nodep(),
                                       /*recursive*/ 1));

    String translated = libxml_get_valid_file_path(uri);
    if (translated.empty()) {
      raise_warning("Invalid URI");
      return false;
    }

    xmlDocPtr res = data->apply_stylesheet ();
    if (res == nullptr) {
      return false;
    }

    int bytes = xsltSaveResultToFilename(translated.data(),
                                         res,
                                         data->m_stylesheet,
                                         /*compression*/ 0);
    xmlFreeDoc(res);

    if (bytes == -1) {
      return false;
    }

    return bytes;
  }

  return false;
}

static Variant HHVM_METHOD(XSLTProcessor, transformToXML,
                           const Object& doc) {
  auto data = Native::data<XSLTProcessorData>(this_);

  if (doc.instanceof(s_DOMDocument)) {
    auto domdoc = Native::data<DOMNode>(doc);
    data->m_doc =
      libxml_register_node(xmlCopyDoc ((xmlDocPtr)domdoc->nodep(),
                                       /*recursive*/ 1));

    xmlDocPtr res = data->apply_stylesheet();
    if (res == nullptr) {
      return false;
    }

    xmlChar *mem;
    int size;
    if (xsltSaveResultToString(&mem, &size, res, data->m_stylesheet) < 0) {
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

///////////////////////////////////////////////////////////////////////////////
// extension

const StaticString s_XSL_SECPREF_NONE("XSL_SECPREF_NONE");
const StaticString s_XSL_SECPREF_READ_FILE("XSL_SECPREF_READ_FILE");
const StaticString s_XSL_SECPREF_WRITE_FILE("XSL_SECPREF_WRITE_FILE");
const StaticString
  s_XSL_SECPREF_CREATE_DIRECTORY("XSL_SECPREF_CREATE_DIRECTORY");
const StaticString s_XSL_SECPREF_READ_NETWORK("XSL_SECPREF_READ_NETWORK");
const StaticString s_XSL_SECPREF_WRITE_NETWORK("XSL_SECPREF_WRITE_NETWORK");
const StaticString s_XSL_SECPREF_DEFAULT("XSL_SECPREF_DEFAULT");

const StaticString s_xslt_version("LIBXSLT_VERSION");
const StaticString s_xslt_dotted_version("LIBXSLT_DOTTED_VERSION");
const StaticString s_xslt_dotted_version_value(LIBXSLT_DOTTED_VERSION);

class XSLExtension final : public Extension {
  public:
    XSLExtension() : Extension("xsl", "0.1") {};

    void moduleInit() override {
      xsltSetGenericErrorFunc(nullptr, xslt_ext_error_handler);
      exsltRegisterAll();
      Native::registerConstant<KindOfInt64>(
        s_XSL_SECPREF_NONE.get(), k_XSL_SECPREF_NONE
      );
      Native::registerConstant<KindOfInt64>(
        s_XSL_SECPREF_READ_FILE.get(), k_XSL_SECPREF_READ_FILE
      );
      Native::registerConstant<KindOfInt64>(
        s_XSL_SECPREF_WRITE_FILE.get(), k_XSL_SECPREF_WRITE_FILE
      );
      Native::registerConstant<KindOfInt64>(
        s_XSL_SECPREF_CREATE_DIRECTORY.get(), k_XSL_SECPREF_CREATE_DIRECTORY
      );
      Native::registerConstant<KindOfInt64>(
        s_XSL_SECPREF_READ_NETWORK.get(), k_XSL_SECPREF_READ_NETWORK
      );
      Native::registerConstant<KindOfInt64>(
        s_XSL_SECPREF_WRITE_NETWORK.get(), k_XSL_SECPREF_WRITE_NETWORK
      );
      Native::registerConstant<KindOfInt64>(
        s_XSL_SECPREF_DEFAULT.get(), k_XSL_SECPREF_DEFAULT
      );

      Native::registerConstant<KindOfInt64>(
        s_xslt_version.get(), LIBXSLT_VERSION
      );
      Native::registerConstant<KindOfString>(
        s_xslt_dotted_version.get(), s_xslt_dotted_version_value.get()
      );

      HHVM_ME(XSLTProcessor, getParameter);
      HHVM_ME(XSLTProcessor, getSecurityPrefs);
      HHVM_ME(XSLTProcessor, importStylesheet);
      HHVM_ME(XSLTProcessor, removeParameter);
      HHVM_ME(XSLTProcessor, registerPHPFunctions);
      HHVM_ME(XSLTProcessor, setParameter);
      HHVM_ME(XSLTProcessor, setSecurityPrefs);
      HHVM_ME(XSLTProcessor, setProfiling);
      HHVM_ME(XSLTProcessor, transformToDoc);
      HHVM_ME(XSLTProcessor, transformToURI);
      HHVM_ME(XSLTProcessor, transformToXML);

      Native::
        registerNativeDataInfo<XSLTProcessorData>(s_XSLTProcessorData.get());

      loadSystemlib();
    }
} s_xsl_extension;

///////////////////////////////////////////////////////////////////////////////
}
