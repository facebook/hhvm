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

#ifndef incl_HPHP_EXT_SIMPLEXML_H_
#define incl_HPHP_EXT_SIMPLEXML_H_

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/ext_simplexml_include.h"
namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Variant f_simplexml_import_dom(const Object& node,
                               const String& class_name = "SimpleXMLElement");
Variant f_simplexml_load_string(const String& data,
                                const String& class_name = "SimpleXMLElement",
                                int64_t options = 0,
                                const String& ns = "",
                                bool is_prefix = false);
Variant f_simplexml_load_file(const String& filename, const String& class_name = "SimpleXMLElement", int64_t options = 0, const String& ns = "", bool is_prefix = false);
Variant f_libxml_get_errors();
Variant f_libxml_get_last_error();
void f_libxml_clear_errors();
bool f_libxml_use_internal_errors(bool use_errors = false);
bool f_libxml_disable_entity_loader(bool disable = true);

///////////////////////////////////////////////////////////////////////////////
// SimpleXMLElement

typedef enum {
  SXE_ITER_NONE     = 0,
  SXE_ITER_ELEMENT  = 1,
  SXE_ITER_CHILD    = 2,
  SXE_ITER_ATTRLIST = 3
} SXE_ITER;

FORWARD_DECLARE_CLASS(SimpleXMLElement);

class c_SimpleXMLElement :
      public ExtObjectDataFlags<ObjectData::UseGet|
                                ObjectData::UseSet|
                                ObjectData::UseIsset|
                                ObjectData::UseUnset|
                                ObjectData::CallToImpl|
                                ObjectData::HasClone>,
      public Sweepable {
 public:
  DECLARE_CLASS(SimpleXMLElement)

  public: c_SimpleXMLElement(Class* cls = c_SimpleXMLElement::classof());
  public: ~c_SimpleXMLElement();
  public: void t___construct(const String& data, int64_t options = 0, bool data_is_url = false, const String& ns = "", bool is_prefix = false);
  public: bool t_offsetexists(const Variant& index);
  public: Variant t_offsetget(const Variant& index);
  public: void t_offsetset(const Variant& index, const Variant& newvalue);
  public: void t_offsetunset(const Variant& index);
  public: Variant t_getiterator();
  public: int64_t t_count();
  public: Variant t_xpath(const String& path);
  public: bool t_registerxpathnamespace(const String& prefix, const String& ns);
  public: Variant t_asxml(const String& filename = "");
  public: Variant t_savexml(const String& filename = "");
  public: Array t_getnamespaces(bool recursive = false);
  public: Array t_getdocnamespaces(bool recursive = false,
                                   bool from_root = true);
  public: Object t_children(const String& ns = "", bool is_prefix = false);
  public: String t_getname();
  public: Object t_attributes(const String& ns = "", bool is_prefix = false);
  public: Variant t_addchild(const String& qname, const String& value = null_string, const String& ns = null_string);
  public: void t_addattribute(const String& qname, const String& value = null_string, const String& ns = null_string);
  public: String t___tostring();
  public: Variant t___get(Variant name);
  public: Variant t___set(Variant name, Variant value);
  public: bool t___isset(Variant name);
  public: Variant t___unset(Variant name);

 public:
  static c_SimpleXMLElement* Clone(ObjectData* obj);
  static bool    ToBool(const ObjectData* obj) noexcept;
  static int64_t ToInt64(const ObjectData* obj) noexcept;
  static double  ToDouble(const ObjectData* obj) noexcept;
  static Array   ToArray(const ObjectData* obj);

  Resource   document;
  xmlNodePtr node;
  xmlXPathContextPtr xpath;
  struct {
    xmlChar* name;
    xmlChar* nsprefix;
    bool     isprefix;
    SXE_ITER type;
    Object   data;
  } iter;
};

///////////////////////////////////////////////////////////////////////////////
// class SimpleXMLElementIterator

FORWARD_DECLARE_CLASS(SimpleXMLElementIterator);

class c_SimpleXMLElementIterator : public ExtObjectData, public Sweepable {
 public:
  DECLARE_CLASS(SimpleXMLElementIterator)

  public: c_SimpleXMLElementIterator(Class* cls = c_SimpleXMLElementIterator::classof());
  public: ~c_SimpleXMLElementIterator();
  public: void t___construct();
  public: Variant t_current();
  public: Variant t_key();
  public: Variant t_next();
  public: Variant t_rewind();
  public: Variant t_valid();

public:
  c_SimpleXMLElement* sxe;
};

///////////////////////////////////////////////////////////////////////////////
// class LibXMLError

FORWARD_DECLARE_CLASS(LibXMLError);

class c_LibXMLError : public ExtObjectData {
 public:
  DECLARE_CLASS_NO_SWEEP(LibXMLError)

  public: c_LibXMLError(Class* cls = c_LibXMLError::classof());
  public: ~c_LibXMLError();
  public: void t___construct();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_SIMPLEXML_H_
