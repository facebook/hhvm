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

#ifndef PHP_SDL_H
#define PHP_SDL_H

#include <runtime/ext/soap/encoding.h>

///////////////////////////////////////////////////////////////////////////////
// defines

#define XSD_WHITESPACE_COLLAPSE 1
#define XSD_WHITESPACE_PRESERVE 1
#define XSD_WHITESPACE_REPLACE  1

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// XSD enums

enum sdlContentKind {
  XSD_CONTENT_ELEMENT,
  XSD_CONTENT_SEQUENCE,
  XSD_CONTENT_ALL,
  XSD_CONTENT_CHOICE,
  XSD_CONTENT_GROUP_REF,
  XSD_CONTENT_GROUP,
  XSD_CONTENT_ANY
};

enum sdlTypeKind {
  XSD_TYPEKIND_SIMPLE,
  XSD_TYPEKIND_LIST,
  XSD_TYPEKIND_UNION,
  XSD_TYPEKIND_COMPLEX,
  XSD_TYPEKIND_RESTRICTION,
  XSD_TYPEKIND_EXTENSION
};

enum sdlUse {
  XSD_USE_DEFAULT,
  XSD_USE_OPTIONAL,
  XSD_USE_PROHIBITED,
  XSD_USE_REQUIRED
};

enum sdlForm {
  XSD_FORM_DEFAULT,
  XSD_FORM_QUALIFIED,
  XSD_FORM_UNQUALIFIED
};

///////////////////////////////////////////////////////////////////////////////
// XSD types

DECLARE_BOOST_TYPES(sdlType);
DECLARE_BOOST_TYPES(sdlContentModel);
DECLARE_BOOST_TYPES(sdlAttribute);
DECLARE_BOOST_TYPES(sdlExtraAttribute);
DECLARE_BOOST_TYPES(sdlRestrictions);
DECLARE_BOOST_TYPES(sdlRestrictionChar);
DECLARE_BOOST_TYPES(sdlRestrictionInt);

typedef StringTosdlTypePtrMap sdlTypeMap;
typedef sdlTypePtrVec sdlTypeVec;
typedef StringTosdlAttributePtrMap sdlAttributeMap;
typedef StringTosdlExtraAttributePtrMap sdlExtraAttributeMap;

struct sdlRestrictionInt {
  int value;
  bool fixed;
};

struct sdlRestrictionChar {
  std::string value;
  bool fixed;
};

struct sdlRestrictions {
  sdlRestrictionCharPtrVec enumeration;
  sdlRestrictionIntPtr minExclusive;
  sdlRestrictionIntPtr minInclusive;
  sdlRestrictionIntPtr maxExclusive;
  sdlRestrictionIntPtr maxInclusive;
  sdlRestrictionIntPtr totalDigits;
  sdlRestrictionIntPtr fractionDigits;
  sdlRestrictionIntPtr length;
  sdlRestrictionIntPtr minLength;
  sdlRestrictionIntPtr maxLength;
  sdlRestrictionCharPtr whiteSpace;
  sdlRestrictionCharPtr pattern;
};

struct sdlContentModel {
  sdlContentKind kind;
  int min_occurs;
  int max_occurs;

  // only one of these is effective, depending on "kind"
  sdlType              *u_element;   // pointer to element
  sdlType              *u_group;     // pointer to group
  sdlContentModelPtrVec u_content;   // array of sequnce,all,choice
  std::string           u_group_ref; // reference to group
};

struct sdlExtraAttribute {
  std::string ns;
  std::string val;
};

struct sdlAttribute {
  std::string name;
  std::string namens;
  std::string ref;
  std::string def;
  std::string fixed;
  sdlForm    form;
  sdlUse     use;
  sdlExtraAttributeMap extraAttributes;
  encodePtr  encode;
};

struct sdlType {
  sdlType() : kind(XSD_TYPEKIND_SIMPLE), nillable(false),
              form(XSD_FORM_DEFAULT) {}

  sdlTypeKind         kind;
  std::string         name;
  std::string         namens;
  bool                nillable;
  sdlTypeVec          elements;
  sdlAttributeMap     attributes;
  sdlRestrictionsPtr  restrictions;
  encodePtr           encode;
  sdlContentModelPtr  model;
  std::string         def;
  std::string         fixed;
  std::string         ref;
  sdlForm             form;
};

///////////////////////////////////////////////////////////////////////////////
// SOAP enums

enum sdlBindingType {
  BINDING_SOAP = 1,
  BINDING_HTTP = 2
};

enum sdlEncodingStyle {
  SOAP_RPC      = 1,
  SOAP_DOCUMENT = 2
};

enum sdlRpcEncodingStyle {
  SOAP_ENCODING_DEFAULT = 0,
  SOAP_ENCODING_1_1     = 1,
  SOAP_ENCODING_1_2     = 2
};

enum sdlEncodingUse {
  SOAP_ENCODED = 1,
  SOAP_LITERAL = 2
};

enum sdlTransport {
  SOAP_TRANSPORT_HTTP = 1
};

///////////////////////////////////////////////////////////////////////////////
// SOAP types

/* Soap Binding Specfic stuff */
DECLARE_BOOST_TYPES(sdl);
DECLARE_BOOST_TYPES(sdlSoapBinding);
DECLARE_BOOST_TYPES(sdlBinding);
DECLARE_BOOST_TYPES(sdlSoapBindingFunctionHeader);
DECLARE_BOOST_TYPES(sdlSoapBindingFunctionFault);
DECLARE_BOOST_TYPES(sdlSoapBindingFunction);
DECLARE_BOOST_TYPES(sdlParam);
DECLARE_BOOST_TYPES(sdlFault);
DECLARE_BOOST_TYPES(sdlFunction);

typedef sdlTypePtrVec sdlTypeVec;
typedef StringTosdlBindingPtrMap sdlBindingMap;
typedef StringTosdlSoapBindingFunctionHeaderPtrMap \
  sdlSoapBindingFunctionHeaderMap;
typedef sdlParamPtrVec sdlParamVec;
typedef StringTosdlFaultPtrMap sdlFaultMap;
typedef StringTosdlFunctionPtrMap sdlFunctionMap;

struct sdlSoapBinding {
  sdlEncodingStyle  style;
  sdlTransport      transport; /* not implemented yet */
};

struct sdlBinding {
  std::string       name;
  std::string       location;
  sdlBindingType    bindingType;
  sdlSoapBindingPtr bindingAttributes;
};

struct sdlSoapBindingFunctionHeader {
  std::string          name;
  std::string          ns;
  sdlEncodingUse       use;
  sdlTypePtr           element;
  encodePtr            encode;
  sdlRpcEncodingStyle  encodingStyle; /* not implemented yet */
  sdlSoapBindingFunctionHeaderMap  headerfaults;
};

struct sdlSoapBindingFunctionFault {
  std::string          ns;
  sdlEncodingUse       use;
  sdlRpcEncodingStyle  encodingStyle; /* not implemented yet */
};

struct sdlSoapBindingFunctionBody {
  std::string          ns;
  sdlEncodingUse       use;
  sdlRpcEncodingStyle  encodingStyle;  /* not implemented yet */
  sdlSoapBindingFunctionHeaderMap headers;
};

struct sdlSoapBindingFunction {
  std::string                 soapAction;
  sdlEncodingStyle            style;

  sdlSoapBindingFunctionBody  input;
  sdlSoapBindingFunctionBody  output;
};

struct sdlParam {
  int         order;
  sdlTypePtr  element;
  encodePtr   encode;
  std::string paramName;
};

struct sdlFault {
  std::string name;
  sdlParamVec details;
  sdlSoapBindingFunctionFaultPtr bindingAttributes;
};

struct sdlFunction {
  std::string        functionName;
  std::string        requestName;
  std::string        responseName;
  sdlParamVec        requestParameters;
  sdlParamVec        responseParameters;
  sdlBindingPtr      binding;
  sdlSoapBindingFunctionPtr bindingAttributes;
  sdlFaultMap        faults;
};

///////////////////////////////////////////////////////////////////////////////
// top level data structures

struct sdl {
  sdlTypeMap     elements;
  sdlTypeMap     groups;
  sdlTypeVec     types;

  encodeMap      encoders;

  sdlFunctionMap functions;
  sdlBindingMap  bindings;
  sdlFunctionMap requests;
  std::string    target_ns;
  std::string    source;
};

struct sdlCtx {
  sdlPtr          sdl;
  sdlAttributeMap attributes;
  sdlTypeMap      attributeGroups;

  xmlDocMap       docs;
  xmlNodeMap      messages;
  xmlNodeMap      portTypes;
  xmlNodeMap      bindings;
  xmlNodeMap      services;
};

///////////////////////////////////////////////////////////////////////////////
// top level functions

encodePtr get_encoder_from_prefix(sdl *sdl, xmlNodePtr data,
                                  const xmlChar *type);
encodePtr get_encoder(sdl *sdl, const char *ns, const char *type);
encodePtr get_encoder_ex(sdl *sdl, const std::string &nscat);

sdlBindingPtr get_binding_from_type(sdl *sdl, int type);
sdlBindingPtr get_binding_from_name(sdl *sdl, char *name, char *ns);

sdlPtr load_wsdl(char *struri);
bool load_schema(sdlCtx *ctx, xmlNodePtr schema);
void schema_pass2(sdlCtx *ctx);

///////////////////////////////////////////////////////////////////////////////
}

#endif
