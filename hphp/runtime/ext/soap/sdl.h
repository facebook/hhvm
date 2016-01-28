/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_PHP_SDL_H
#define incl_HPHP_PHP_SDL_H

#include <unordered_map>
#include <vector>
#include <memory>

#include "hphp/runtime/ext/soap/encoding.h"
#include "hphp/util/hash-map-typedefs.h"
#include "hphp/runtime/base/http-client.h"

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

struct sdlType;
struct sdlContentModel;
struct sdlAttribute;
struct sdlExtraAttribute;
struct sdlRestrictions;
struct sdlRestrictionChar;
struct sdlRestrictionInt;

using sdlTypePtr = std::shared_ptr<sdlType>;
using sdlTypePtrVec = std::vector<std::shared_ptr<sdlType>>;
using sdlTypeMap = hphp_string_hash_map<std::shared_ptr<sdlType>,sdlType>;
using sdlAttributePtr = std::shared_ptr<sdlAttribute>;
using sdlAttributeMap =
      hphp_string_hash_map<std::shared_ptr<sdlAttribute>,sdlAttribute>;
using sdlExtraAttributeMap =
     hphp_string_hash_map<std::shared_ptr<sdlExtraAttribute>,sdlExtraAttribute>;

struct sdlBinding;
struct sdlContentModel;
struct sdl;
struct sdlRestrictions;
struct sdlSoapBindingFunctionFault;
struct sdlSoapBindingFunction;
struct sdlSoapBinding;

using sdlBindingPtr = std::shared_ptr<sdlBinding>;
using sdlContentModelPtr = std::shared_ptr<sdlContentModel>;
using sdlPtr = std::shared_ptr<sdl>;
using sdlRestrictionsPtr = std::shared_ptr<sdlRestrictions>;
using sdlSoapBindingFunctionFaultPtr = std::shared_ptr<sdlSoapBindingFunctionFault>;
using sdlSoapBindingFunctionPtr = std::shared_ptr<sdlSoapBindingFunction>;
using sdlSoapBindingPtr = std::shared_ptr<sdlSoapBinding>;

struct sdlRestrictionInt {
  int value;
  bool fixed;
};

using sdlRestrictionIntPtr = std::shared_ptr<sdlRestrictionInt>;
using sdlRestrictionIntPtrVec = std::vector<sdlRestrictionIntPtr>;

struct sdlRestrictionChar {
  std::string value;
  bool fixed;
};

using sdlRestrictionCharPtr = std::shared_ptr<sdlRestrictionChar>;
using sdlRestrictionCharPtrVec = std::vector<sdlRestrictionCharPtr>;

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
  std::vector<std::shared_ptr<sdlContentModel>>
                        u_content;   // array of sequnce,all,choice
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

struct sdlType;
using sdlTypeVec = std::vector<std::shared_ptr<sdlType>>;

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

struct sdl;
struct sdlSoapBinding;
struct sdlBinding;
struct sdlSoapBindingFunctionHeader;
struct sdlSoapBindingFunctionFault;
struct sdlSoapBindingFunction;
struct sdlParam;
struct sdlFault;
struct sdlFunction;

using sdlBindingMap =
      hphp_string_hash_map<std::shared_ptr<sdlBinding>,sdlBinding>;
using sdlSoapBindingFunctionHeaderMap =
      hphp_string_hash_map<std::shared_ptr<sdlSoapBindingFunctionHeader>,
                           sdlSoapBindingFunctionHeader>;
using sdlParamPtr = std::shared_ptr<sdlParam>;
using sdlParamVec = std::vector<std::shared_ptr<sdlParam>>;
typedef hphp_string_hash_map<std::shared_ptr<sdlFault>,sdlFault>
        sdlFaultMap;
typedef hphp_string_hash_map<std::shared_ptr<sdlFunction>,sdlFunction>
        sdlFunctionMap;

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
  std::vector<std::string> functionsOrder;
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

  ~sdlCtx();
};

///////////////////////////////////////////////////////////////////////////////
// top level functions

encodePtr get_encoder_from_prefix(sdl *sdl, xmlNodePtr data,
                                  const xmlChar *type);
encodePtr get_encoder(sdl *sdl, const char *ns, const char *type);
encodePtr get_encoder_ex(sdl *sdl, const std::string &nscat);

sdlBindingPtr get_binding_from_type(sdl *sdl, int type);
sdlBindingPtr get_binding_from_name(sdl *sdl, char *name, char *ns);

sdlPtr load_wsdl(char *struri, HttpClient *http = nullptr);
bool load_schema(sdlCtx *ctx, xmlNodePtr schema);
void schema_pass2(sdlCtx *ctx);

///////////////////////////////////////////////////////////////////////////////
}

#endif
