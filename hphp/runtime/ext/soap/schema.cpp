/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/soap/sdl.h"
#include "hphp/runtime/ext/soap/soap.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static bool schema_simpleType
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr simpleType, sdlTypePtr cur_type);
static bool schema_complexType
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr compType, sdlTypePtr cur_type);
static bool schema_list
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr listType, sdlTypePtr cur_type);
static bool schema_union
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr unionType, sdlTypePtr cur_type);
static bool schema_simpleContent
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr simpCompType, sdlTypePtr cur_type);
static bool schema_restriction_simpleContent
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr restType, sdlTypePtr cur_type,
 int simpleType);
static bool schema_restriction_complexContent
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr restType, sdlTypePtr cur_type);
static bool schema_extension_simpleContent
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr extType, sdlTypePtr cur_type);
static bool schema_extension_complexContent
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr extType, sdlTypePtr cur_type);
static bool schema_sequence
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr seqType, sdlTypePtr cur_type,
 sdlContentModelPtr model);
static bool schema_all
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr extType, sdlTypePtr cur_type,
 sdlContentModelPtr model);
static bool schema_choice
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr choiceType, sdlTypePtr cur_type,
 sdlContentModelPtr model);
static bool schema_group
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr groupType, sdlTypePtr cur_type,
 sdlContentModelPtr model);
static bool schema_any
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr extType, sdlTypePtr cur_type,
 sdlContentModelPtr model);
static bool schema_element
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr element, sdlTypePtr cur_type,
 sdlContentModelPtr model);
static bool schema_attribute
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr attrType, sdlTypePtr cur_type,
 sdlCtx *ctx);
static bool schema_attributeGroup
(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr attrType, sdlTypePtr cur_type,
 sdlCtx *ctx);

static bool schema_restriction_var_int
(xmlNodePtr val, sdlRestrictionIntPtr &valptr);

static bool schema_restriction_var_char
(xmlNodePtr val, sdlRestrictionCharPtr &valptr);

static void schema_type_fixup(sdlCtx *ctx, sdlTypePtr type);

///////////////////////////////////////////////////////////////////////////////
// helpers

static encodePtr create_encoder(sdlPtr sdl, sdlTypePtr cur_type,
                                const xmlChar *ns, const xmlChar *type) {
  string nscat = (char*)ns;
  nscat += ':';
  nscat += (char*)type;

  encodeMap::iterator iter = sdl->encoders.find(nscat);
  encodePtr enc;
  if (iter != sdl->encoders.end()) {
    enc = iter->second;
  } else {
    enc = encodePtr(new encode());
  }

  enc->details.type = 0;
  enc->details.ns = (char*)ns;
  enc->details.type_str = (char*)type;
  enc->details.sdl_type = cur_type.get();
  enc->to_xml = sdl_guess_convert_xml;
  enc->to_zval = sdl_guess_convert_zval;

  if (iter == sdl->encoders.end()) {
    sdl->encoders[nscat] = enc;
  }
  return enc;
}

static encodePtr get_create_encoder(sdlPtr sdl, sdlTypePtr cur_type,
                                    const xmlChar *ns, const xmlChar *type) {
  encodePtr enc = get_encoder(sdl.get(), (char*)ns, (char*)type);
  if (!enc) {
    enc = create_encoder(sdl, cur_type, ns, type);
  }
  return enc;
}

static void schema_load_file(sdlCtx *ctx, xmlAttrPtr ns, xmlChar *location,
                             xmlAttrPtr tns, bool import) {
  if (location && ctx->docs.find((char*)location) == ctx->docs.end()) {
    xmlDocPtr doc = soap_xmlParseFile((char*)location);
    if (!doc) {
      throw SoapException("Parsing Schema: can't import schema from '%s'",
                          location);
    }
    xmlNodePtr schema = get_node(doc->children, "schema");
    if (!schema) {
      xmlFreeDoc(doc);
      throw SoapException("Parsing Schema: can't import schema from '%s'",
                          location);
    }
    xmlAttrPtr new_tns = get_attribute(schema->properties, "targetNamespace");
    if (import) {
      if (ns && (!new_tns || xmlStrcmp(ns->children->content,
                                       new_tns->children->content))) {
        xmlFreeDoc(doc);
        throw SoapException("Parsing Schema: can't import schema from '%s', "
                            "unexpected 'targetNamespace'='%s'",
                            location, ns->children->content);
      }
      if (!ns && new_tns) {
        xmlFreeDoc(doc);
        throw SoapException("Parsing Schema: can't import schema from '%s', "
                            "unexpected 'targetNamespace'='%s'",
                            location, new_tns->children->content);
      }
    } else {
      new_tns = get_attribute(schema->properties, "targetNamespace");
      if (!new_tns) {
        if (tns) {
          xmlSetProp(schema, BAD_CAST("targetNamespace"),
                     tns->children->content);
        }
      } else if (tns && xmlStrcmp(tns->children->content,
                                  new_tns->children->content)) {
        xmlFreeDoc(doc);
        throw SoapException("Parsing Schema: can't include schema from '%s', "
                            "different 'targetNamespace'", location);
      }
    }
    ctx->docs[(char*)location] = doc;
    load_schema(ctx, schema);
  }
}

bool checkBaseAttribute(sdlPtr sdl, xmlNodePtr extType, sdlTypePtr cur_type,
                        bool logError = true) {
  xmlAttrPtr base = get_attribute(extType->properties, "base");
  if (base) {
    string type, ns;
    parse_namespace(base->children->content, type, ns);
    xmlNsPtr nsptr = xmlSearchNs(extType->doc, extType, NS_STRING(ns));
    if (nsptr) {
      cur_type->encode = get_create_encoder(sdl, cur_type, nsptr->href,
                                            BAD_CAST(type.c_str()));
    }
    return true;
  }
  if (logError) {
    throw SoapException("Parsing Schema: restriction has no 'base' attribute");
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// load_schema

/*
2.6.1 xsi:type
2.6.2 xsi:nil
2.6.3 xsi:schemaLocation, xsi:noNamespaceSchemaLocation
*/

/*
<schema
  attributeFormDefault = (qualified | unqualified) : unqualified
  blockDefault = (#all | List of (extension | restriction | substitution))  : ''
  elementFormDefault = (qualified | unqualified) : unqualified
  finalDefault = (#all | List of (extension | restriction))  : ''
  id = ID
  targetNamespace = anyURI
  version = token
  xml:lang = language
  {any attributes with non-schema namespace . . .}>
  Content: ((include | import | redefine | annotation)*, (((simpleType | complexType | group | attributeGroup) | element | attribute | notation), annotation*)*)
</schema>
*/
bool load_schema(sdlCtx *ctx, xmlNodePtr schema) {
  xmlAttrPtr tns = get_attribute(schema->properties, "targetNamespace");
  if (!tns) {
    tns = xmlSetProp(schema, BAD_CAST("targetNamespace"), BAD_CAST(""));
    xmlNewNs(schema, BAD_CAST(""), NULL);
  }

  xmlNodePtr trav = schema->children;
  while (trav) {
    if (node_is_equal(trav,"include")) {
      xmlAttrPtr location = get_attribute(trav->properties, "schemaLocation");
      if (!location) {
        throw SoapException("Parsing Schema: include has no 'schemaLocation' "
                            "attribute");
      } else {
        xmlChar *uri;
        xmlChar *base = xmlNodeGetBase(trav->doc, trav);
        if (!base) {
          uri = xmlBuildURI(location->children->content, trav->doc->URL);
        } else {
          uri = xmlBuildURI(location->children->content, base);
          xmlFree(base);
        }
        schema_load_file(ctx, NULL, uri, tns, 0);
        xmlFree(uri);
      }

    } else if (node_is_equal(trav,"redefine")) {
      xmlAttrPtr location = get_attribute(trav->properties, "schemaLocation");
      if (!location) {
        throw SoapException("Parsing Schema: redefine has no 'schemaLocation' "
                            "attribute");
      } else {
        xmlChar *uri;
        xmlChar *base = xmlNodeGetBase(trav->doc, trav);
        if (!base) {
          uri = xmlBuildURI(location->children->content, trav->doc->URL);
        } else {
          uri = xmlBuildURI(location->children->content, base);
          xmlFree(base);
        }
        schema_load_file(ctx, NULL, uri, tns, 0);
        xmlFree(uri);
        /* TODO: <redefine> support */
      }

    } else if (node_is_equal(trav,"import")) {
      xmlChar *uri = NULL;

      xmlAttrPtr ns = get_attribute(trav->properties, "namespace");
      xmlAttrPtr location = get_attribute(trav->properties, "schemaLocation");

      if (ns && tns && xmlStrcmp(ns->children->content,
                                 tns->children->content) == 0) {
        if (location) {
          throw SoapException("Parsing Schema: can't import schema from '%s', "
                              "namespace must not match the enclosing schema "
                              "'targetNamespace'",
                              location->children->content);
        } else {
          throw SoapException("Parsing Schema: can't import schema. Namespace"
                              " must not match the enclosing schema "
                              "'targetNamespace'");
        }
      }
      if (location) {
        xmlChar *base = xmlNodeGetBase(trav->doc, trav);
        if (!base) {
          uri = xmlBuildURI(location->children->content, trav->doc->URL);
        } else {
          uri = xmlBuildURI(location->children->content, base);
          xmlFree(base);
        }
      }
      schema_load_file(ctx, ns, uri, tns, 1);
      if (uri) {xmlFree(uri);}
    } else if (node_is_equal(trav,"annotation")) {
      /* TODO: <annotation> support */
/* annotation cleanup
      xmlNodePtr tmp = trav;
      trav = trav->next;
      xmlUnlinkNode(tmp);
      xmlFreeNode(tmp);
      continue;
*/
    } else {
      break;
    }
    trav = trav->next;
  }

  while (trav) {
    if (node_is_equal(trav,"simpleType")) {
      schema_simpleType(ctx->sdl, tns, trav, sdlTypePtr());
    } else if (node_is_equal(trav,"complexType")) {
      schema_complexType(ctx->sdl, tns, trav, sdlTypePtr());
    } else if (node_is_equal(trav,"group")) {
      schema_group(ctx->sdl, tns, trav, sdlTypePtr(), sdlContentModelPtr());
    } else if (node_is_equal(trav,"attributeGroup")) {
      schema_attributeGroup(ctx->sdl, tns, trav, sdlTypePtr(), ctx);
    } else if (node_is_equal(trav,"element")) {
      schema_element(ctx->sdl, tns, trav, sdlTypePtr(), sdlContentModelPtr());
    } else if (node_is_equal(trav,"attribute")) {
      schema_attribute(ctx->sdl, tns, trav, sdlTypePtr(), ctx);
    } else if (node_is_equal(trav,"notation")) {
      /* TODO: <notation> support */
    } else if (node_is_equal(trav,"annotation")) {
      /* TODO: <annotation> support */
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in schema",
                          trav->name);
    }
    trav = trav->next;
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// XSD types

/*
<simpleType
  final = (#all | (list | union | restriction))
  id = ID
  name = NCName
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (restriction | list | union))
</simpleType>
*/
static bool schema_simpleType(sdlPtr sdl, xmlAttrPtr tns,
                              xmlNodePtr simpleType, sdlTypePtr cur_type) {
  xmlAttrPtr ns = get_attribute(simpleType->properties, "targetNamespace");
  if (!ns) {
    ns = tns;
  }

  xmlAttrPtr name = get_attribute(simpleType->properties, "name");

  sdlTypePtr newType(new sdlType());
  if (cur_type) {
    /* Anonymous type inside <element> or <restriction> */
    if (name) {
      newType->name = (char*)name->children->content;
      newType->namens = (char*)ns->children->content;
    } else {
      newType->name = cur_type->name;
      newType->namens = cur_type->namens;
    }
    sdl->types.push_back(newType);

    cur_type->encode = encodePtr(new encode());
    cur_type->encode->details.ns = newType->namens;
    cur_type->encode->details.type_str = newType->name;
    cur_type->encode->details.sdl_type = newType.get();
    cur_type->encode->to_xml = sdl_guess_convert_xml;
    cur_type->encode->to_zval = sdl_guess_convert_zval;
    sdl->encoders[lexical_cast<string>(sdl->encoders.size())] =
      cur_type->encode;

    cur_type = newType;

  } else if (name) {
    if (!cur_type) {
      sdl->types.push_back(newType);
    } else {
      cur_type->elements.push_back(newType);
    }
    cur_type = newType;
    create_encoder(sdl, cur_type, ns->children->content,
                   name->children->content);
  } else {
    throw SoapException("Parsing Schema: simpleType has no 'name' attribute");
  }

  xmlNodePtr trav = simpleType->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  if (trav) {
    if (node_is_equal(trav,"restriction")) {
      schema_restriction_simpleContent(sdl, tns, trav, cur_type, 1);
      trav = trav->next;
    } else if (node_is_equal(trav,"list")) {
      cur_type->kind = XSD_TYPEKIND_LIST;
      schema_list(sdl, tns, trav, cur_type);
      trav = trav->next;
    } else if (node_is_equal(trav,"union")) {
      cur_type->kind = XSD_TYPEKIND_UNION;
      schema_union(sdl, tns, trav, cur_type);
      trav = trav->next;
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in simpleType",
                          trav->name);
    }
  } else {
    throw SoapException("Parsing Schema: expected <restriction>, <list> or "
                        "<union> in simpleType");
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in simpleType",
                        trav->name);
  }

  return true;
}

/*
<list
  id = ID
  itemType = QName
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleType?))
</list>
*/
static bool schema_list(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr listType,
                        sdlTypePtr cur_type) {
  xmlAttrPtr itemType = get_attribute(listType->properties, "itemType");
  if (itemType) {
    string type, ns;
    parse_namespace(itemType->children->content, type, ns);
    xmlNsPtr nsptr = xmlSearchNs(listType->doc, listType, NS_STRING(ns));
    if (nsptr) {
      sdlTypePtr newType(new sdlType());
      newType->name = type;
      newType->namens = (char*)nsptr->href;
      newType->encode = get_create_encoder(sdl, newType, nsptr->href,
                                           BAD_CAST(type.c_str()));
      cur_type->elements.push_back(newType);
    }
  }

  xmlNodePtr trav = listType->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  if (trav && node_is_equal(trav,"simpleType")) {
    if (itemType) {
      throw SoapException("Parsing Schema: element has both 'itemType' "
                          "attribute and subtype");
    }
    sdlTypePtr newType(new sdlType());
    newType->name = "anonymous" + lexical_cast<string>(sdl->types.size());
    newType->namens = (char*)tns->children->content;
    cur_type->elements.push_back(newType);

    schema_simpleType(sdl, tns, trav, newType);

    trav = trav->next;
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in list", trav->name);
  }
  return true;
}

/*
<union
  id = ID
  memberTypes = List of QName
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleType*))
</union>
*/
static bool schema_union(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr unionType,
                         sdlTypePtr cur_type) {
  xmlAttrPtr memberTypes = get_attribute(unionType->properties, "memberTypes");
  if (memberTypes) {
    char *str, *start, *end, *next;
    string type, ns;
    xmlNsPtr nsptr;

    str = strdup((char*)memberTypes->children->content);
    whiteSpace_collapse(BAD_CAST(str));
    start = str;
    while (start && *start != '\0') {
      end = strchr(start,' ');
      if (end == NULL) {
        next = NULL;
      } else {
        *end = '\0';
        next = end+1;
      }

      parse_namespace(BAD_CAST(start), type, ns);
      nsptr = xmlSearchNs(unionType->doc, unionType, NS_STRING(ns));
      if (nsptr) {
        sdlTypePtr newType(new sdlType());
        newType->name = type;
        newType->namens = (char*)nsptr->href;
        newType->encode = get_create_encoder(sdl, newType, nsptr->href,
                                             BAD_CAST(type.c_str()));
        cur_type->elements.push_back(newType);
      }
      start = next;
    }
    free(str);
  }

  xmlNodePtr trav = unionType->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  while (trav) {
    if (node_is_equal(trav,"simpleType")) {
      sdlTypePtr newType(new sdlType());
      newType->name = "anonymous" + lexical_cast<string>(sdl->types.size());
      newType->namens = (char*)tns->children->content;
      cur_type->elements.push_back(newType);
      schema_simpleType(sdl, tns, trav, newType);
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in union",
                          trav->name);
    }
    trav = trav->next;
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in union",
                        trav->name);
  }
  return true;
}

/*
<simpleContent
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (restriction | extension))
</simpleContent>
*/
static bool schema_simpleContent(sdlPtr sdl, xmlAttrPtr tns,
                                 xmlNodePtr simpCompType,
                                 sdlTypePtr cur_type) {
  xmlNodePtr trav = simpCompType->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  if (trav) {
    if (node_is_equal(trav, "restriction")) {
      cur_type->kind = XSD_TYPEKIND_RESTRICTION;
      schema_restriction_simpleContent(sdl, tns, trav, cur_type, 0);
      trav = trav->next;
    } else if (node_is_equal(trav, "extension")) {
      cur_type->kind = XSD_TYPEKIND_EXTENSION;
      schema_extension_simpleContent(sdl, tns, trav, cur_type);
      trav = trav->next;
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in simpleContent",
                          trav->name);
    }
  } else {
    throw SoapException("Parsing Schema: expected <restriction> or <extension>"
                        " in simpleContent");
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in simpleContent",
                        trav->name);
  }
  return true;
}

/*
simpleType:<restriction
  base = QName
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleType?, (minExclusive | minInclusive | maxExclusive | maxInclusive | totalDigits | fractionDigits | length | minLength | maxLength | enumeration | whiteSpace | pattern)*)?)
</restriction>
simpleContent:<restriction
  base = QName
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleType?, (minExclusive | minInclusive | maxExclusive | maxInclusive | totalDigits | fractionDigits | length | minLength | maxLength | enumeration | whiteSpace | pattern)*)?, ((attribute | attributeGroup)*, anyAttribute?))
</restriction>
*/
static bool schema_restriction_simpleContent(sdlPtr sdl, xmlAttrPtr tns,
                                             xmlNodePtr restType,
                                             sdlTypePtr cur_type,
                                             int simpleType) {
  checkBaseAttribute(sdl, restType, cur_type, !simpleType);

  if (cur_type->restrictions == NULL) {
    cur_type->restrictions = sdlRestrictionsPtr(new sdlRestrictions());
  }

  xmlNodePtr trav = restType->children;
  if (trav && node_is_equal(trav, "annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  if (trav && node_is_equal(trav, "simpleType")) {
    schema_simpleType(sdl, tns, trav, cur_type);
    trav = trav->next;
  }
  while (trav) {
    if (node_is_equal(trav, "minExclusive")) {
      schema_restriction_var_int(trav, cur_type->restrictions->minExclusive);
    } else if (node_is_equal(trav, "minInclusive")) {
      schema_restriction_var_int(trav, cur_type->restrictions->minInclusive);
    } else if (node_is_equal(trav, "maxExclusive")) {
      schema_restriction_var_int(trav, cur_type->restrictions->maxExclusive);
    } else if (node_is_equal(trav, "maxInclusive")) {
      schema_restriction_var_int(trav, cur_type->restrictions->maxInclusive);
    } else if (node_is_equal(trav, "totalDigits")) {
      schema_restriction_var_int(trav, cur_type->restrictions->totalDigits);
    } else if (node_is_equal(trav, "fractionDigits")) {
      schema_restriction_var_int(trav,
                                 cur_type->restrictions->fractionDigits);
    } else if (node_is_equal(trav, "length")) {
      schema_restriction_var_int(trav, cur_type->restrictions->length);
    } else if (node_is_equal(trav, "minLength")) {
      schema_restriction_var_int(trav, cur_type->restrictions->minLength);
    } else if (node_is_equal(trav, "maxLength")) {
      schema_restriction_var_int(trav, cur_type->restrictions->maxLength);
    } else if (node_is_equal(trav, "whiteSpace")) {
      schema_restriction_var_char(trav, cur_type->restrictions->whiteSpace);
    } else if (node_is_equal(trav, "pattern")) {
      schema_restriction_var_char(trav, cur_type->restrictions->pattern);
    } else if (node_is_equal(trav, "enumeration")) {
      sdlRestrictionCharPtr enumval;
      schema_restriction_var_char(trav, enumval);
      cur_type->restrictions->enumeration.push_back(enumval);
    } else {
      break;
    }
    trav = trav->next;
  }
  if (!simpleType) {
    while (trav) {
      if (node_is_equal(trav,"attribute")) {
        schema_attribute(sdl, tns, trav, cur_type, NULL);
      } else if (node_is_equal(trav,"attributeGroup")) {
        schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
      } else if (node_is_equal(trav,"anyAttribute")) {
        /* TODO: <anyAttribute> support */
        trav = trav->next;
        break;
      } else {
        throw SoapException("Parsing Schema: unexpected <%s> in restriction",
                            trav->name);
      }
      trav = trav->next;
    }
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in restriction",
                        trav->name);
  }

  return true;
}

/*
<restriction
  base = QName
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (group | all | choice | sequence)?, ((attribute | attributeGroup)*, anyAttribute?))
</restriction>
*/
static bool schema_restriction_complexContent(sdlPtr sdl, xmlAttrPtr tns,
                                              xmlNodePtr restType,
                                              sdlTypePtr cur_type) {
  checkBaseAttribute(sdl, restType, cur_type);

  xmlNodePtr trav = restType->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  if (trav) {
    if (node_is_equal(trav,"group")) {
      schema_group(sdl, tns, trav, cur_type, sdlContentModelPtr());
      trav = trav->next;
    } else if (node_is_equal(trav,"all")) {
      schema_all(sdl, tns, trav, cur_type, sdlContentModelPtr());
      trav = trav->next;
    } else if (node_is_equal(trav,"choice")) {
      schema_choice(sdl, tns, trav, cur_type, sdlContentModelPtr());
      trav = trav->next;
    } else if (node_is_equal(trav,"sequence")) {
      schema_sequence(sdl, tns, trav, cur_type, sdlContentModelPtr());
      trav = trav->next;
    }
  }
  while (trav) {
    if (node_is_equal(trav,"attribute")) {
      schema_attribute(sdl, tns, trav, cur_type, NULL);
    } else if (node_is_equal(trav,"attributeGroup")) {
      schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
    } else if (node_is_equal(trav,"anyAttribute")) {
      /* TODO: <anyAttribute> support */
      trav = trav->next;
      break;
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in restriction",
                          trav->name);
    }
    trav = trav->next;
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in restriction",
                        trav->name);
  }

  return true;
}

static bool schema_restriction_var_int(xmlNodePtr val,
                                       sdlRestrictionIntPtr &valptr) {
  valptr = sdlRestrictionIntPtr(new sdlRestrictionInt());

  xmlAttrPtr fixed = get_attribute(val->properties, "fixed");
  valptr->fixed = false;
  if (fixed) {
    if (!strncmp((char*)fixed->children->content, "true", sizeof("true")) ||
        !strncmp((char*)fixed->children->content, "1", sizeof("1"))) {
      valptr->fixed = true;
    }
  }

  xmlAttrPtr value = get_attribute(val->properties, "value");
  if (value == NULL) {
    throw SoapException("Parsing Schema: missing restriction value");
  } else {
    valptr->value = atoi((char*)value->children->content);
  }

  return true;
}

static bool schema_restriction_var_char(xmlNodePtr val,
                                        sdlRestrictionCharPtr &valptr) {
  valptr = sdlRestrictionCharPtr(new sdlRestrictionChar());

  xmlAttrPtr fixed = get_attribute(val->properties, "fixed");
  valptr->fixed = false;
  if (fixed) {
    if (!strncmp((char*)fixed->children->content, "true", sizeof("true")) ||
        !strncmp((char*)fixed->children->content, "1", sizeof("1"))) {
      valptr->fixed = true;
    }
  }

  xmlAttrPtr value = get_attribute(val->properties, "value");
  if (value == NULL) {
    throw SoapException("Parsing Schema: missing restriction value");
  } else {
    valptr->value = (char*)value->children->content;
  }

  return true;
}

/*
From simpleContent (not supported):
<extension
  base = QName
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, ((attribute | attributeGroup)*, anyAttribute?))
</extension>
*/
static bool schema_extension_simpleContent(sdlPtr sdl, xmlAttrPtr tns,
                                           xmlNodePtr extType,
                                           sdlTypePtr cur_type) {
  checkBaseAttribute(sdl, extType, cur_type);

  xmlNodePtr trav = extType->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  while (trav) {
    if (node_is_equal(trav,"attribute")) {
      schema_attribute(sdl, tns, trav, cur_type, NULL);
    } else if (node_is_equal(trav,"attributeGroup")) {
      schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
    } else if (node_is_equal(trav,"anyAttribute")) {
      /* TODO: <anyAttribute> support */
      trav = trav->next;
      break;
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in extension",
                          trav->name);
    }
    trav = trav->next;
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in extension",
                        trav->name);
  }
  return true;
}

/*
From complexContent:
<extension
  base = QName
  id = ID
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, ((group | all | choice | sequence)?, ((attribute | attributeGroup)*, anyAttribute?)))
</extension>
*/
static bool schema_extension_complexContent(sdlPtr sdl, xmlAttrPtr tns,
                                            xmlNodePtr extType,
                                            sdlTypePtr cur_type) {
  checkBaseAttribute(sdl, extType, cur_type);

  xmlNodePtr trav = extType->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  if (trav) {
    if (node_is_equal(trav,"group")) {
      schema_group(sdl, tns, trav, cur_type, sdlContentModelPtr());
      trav = trav->next;
    } else if (node_is_equal(trav,"all")) {
      schema_all(sdl, tns, trav, cur_type, sdlContentModelPtr());
      trav = trav->next;
    } else if (node_is_equal(trav,"choice")) {
      schema_choice(sdl, tns, trav, cur_type, sdlContentModelPtr());
      trav = trav->next;
    } else if (node_is_equal(trav,"sequence")) {
      schema_sequence(sdl, tns, trav, cur_type, sdlContentModelPtr());
      trav = trav->next;
    }
  }
  while (trav) {
    if (node_is_equal(trav,"attribute")) {
      schema_attribute(sdl, tns, trav, cur_type, NULL);
    } else if (node_is_equal(trav,"attributeGroup")) {
      schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
    } else if (node_is_equal(trav,"anyAttribute")) {
      /* TODO: <anyAttribute> support */
      trav = trav->next;
      break;
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in extension",
                          trav->name);
    }
    trav = trav->next;
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in extension",
                        trav->name);
  }
  return true;
}

void schema_min_max(xmlNodePtr node, sdlContentModelPtr model) {
  xmlAttrPtr attr = get_attribute(node->properties, "minOccurs");
  if (attr) {
    model->min_occurs = atoi((char*)attr->children->content);
  } else {
    model->min_occurs = 1;
  }

  attr = get_attribute(node->properties, "maxOccurs");
  if (attr) {
    if (!strncmp((char*)attr->children->content, "unbounded",
                 sizeof("unbounded"))) {
      model->max_occurs = -1;
    } else {
      model->max_occurs = atoi((char*)attr->children->content);
    }
  } else {
    model->max_occurs = 1;
  }
}

/*
<all
  id = ID
  maxOccurs = 1 : 1
  minOccurs = (0 | 1) : 1
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, element*)
</all>
*/
static bool schema_all(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr all,
                       sdlTypePtr cur_type, sdlContentModelPtr model) {
  sdlContentModelPtr newModel(new sdlContentModel());
  newModel->kind = XSD_CONTENT_ALL;
  if (model) {
    model->u_content.push_back(newModel);
  } else {
    cur_type->model = newModel;
  }
  schema_min_max(all, newModel);

  xmlNodePtr trav = all->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  while (trav) {
    if (node_is_equal(trav,"element")) {
      schema_element(sdl, tns, trav, cur_type, newModel);
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in all",
                          trav->name);
    }
    trav = trav->next;
  }
  return true;
}

/*
<group
  name = NCName
  Content: (annotation?, (all | choice | sequence))
</group>
<group
  name = NCName
  ref = QName>
  Content: (annotation?)
</group>
*/
static bool schema_group(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr groupType,
                         sdlTypePtr cur_type, sdlContentModelPtr model) {
  xmlAttrPtr ns = get_attribute(groupType->properties, "targetNamespace");
  if (ns == NULL) {
    ns = tns;
  }

  xmlAttrPtr ref = NULL;
  xmlAttrPtr name = get_attribute(groupType->properties, "name");
  if (name == NULL) {
    name = ref = get_attribute(groupType->properties, "ref");
  }

  sdlContentModelPtr newModel;
  if (name) {
    string key;
    if (ref) {
      string type, ns;
      parse_namespace(ref->children->content, type, ns);
      xmlNsPtr nsptr = xmlSearchNs(groupType->doc, groupType, NS_STRING(ns));
      if (nsptr) {
        key += (char*)nsptr->href;
        key += ':';
      }
      key += type;

      newModel = sdlContentModelPtr(new sdlContentModel());
      newModel->kind = XSD_CONTENT_GROUP_REF;
      newModel->u_group_ref = key;
    } else {
      newModel = sdlContentModelPtr(new sdlContentModel());
      newModel->kind = XSD_CONTENT_SEQUENCE; /* will be redefined */

      key += (char*)ns->children->content;
      key += ':';
      key += (char*)name->children->content;
    }

    if (cur_type == NULL) {
      sdlTypePtr &newType = sdl->groups[key];
      if (newType) {
        throw SoapException("Parsing Schema: group '%s' already defined",
                            key.c_str());
      }
      cur_type = newType = sdlTypePtr(new sdlType());
    }

    if (model == NULL) {
      cur_type->model = newModel;
    } else {
      model->u_content.push_back(newModel);
    }
  } else {
    throw SoapException("Parsing Schema: group has no 'name' nor 'ref'"
                        " attributes");
  }

  schema_min_max(groupType, newModel);

  xmlNodePtr trav = groupType->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  if (trav) {
    if (node_is_equal(trav,"choice")) {
      if (ref) {
        throw SoapException("Parsing Schema: group has both 'ref' attribute"
                            " and subcontent");
      }
      newModel->kind = XSD_CONTENT_CHOICE;
      schema_choice(sdl, tns, trav, cur_type, newModel);
      trav = trav->next;
    } else if (node_is_equal(trav,"sequence")) {
      if (ref) {
        throw SoapException("Parsing Schema: group has both 'ref' attribute"
                            " and subcontent");
      }
      newModel->kind = XSD_CONTENT_SEQUENCE;
      schema_sequence(sdl, tns, trav, cur_type, newModel);
      trav = trav->next;
    } else if (node_is_equal(trav,"all")) {
      if (ref) {
        throw SoapException("Parsing Schema: group has both 'ref' attribute"
                            " and subcontent");
      }
      newModel->kind = XSD_CONTENT_ALL;
      schema_all(sdl, tns, trav, cur_type, newModel);
      trav = trav->next;
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in group",
                          trav->name);
    }
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in group",
                        trav->name);
  }
  return true;
}

/*
<choice
  id = ID
  maxOccurs = (nonNegativeInteger | unbounded)  : 1
  minOccurs = nonNegativeInteger : 1
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (element | group | choice | sequence | any)*)
</choice>
*/
static bool schema_choice(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr choiceType,
                          sdlTypePtr cur_type, sdlContentModelPtr model) {
  sdlContentModelPtr newModel(new sdlContentModel());
  newModel->kind = XSD_CONTENT_CHOICE;
  if (model) {
    model->u_content.push_back(newModel);
  } else {
    cur_type->model = newModel;
  }
  schema_min_max(choiceType, newModel);

  xmlNodePtr trav = choiceType->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  while (trav) {
    if (node_is_equal(trav,"element")) {
      schema_element(sdl, tns, trav, cur_type, newModel);
    } else if (node_is_equal(trav,"group")) {
      schema_group(sdl, tns, trav, cur_type, newModel);
    } else if (node_is_equal(trav,"choice")) {
      schema_choice(sdl, tns, trav, cur_type, newModel);
    } else if (node_is_equal(trav,"sequence")) {
      schema_sequence(sdl, tns, trav, cur_type, newModel);
    } else if (node_is_equal(trav,"any")) {
      schema_any(sdl, tns, trav, cur_type, newModel);
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in choice",
                          trav->name);
    }
    trav = trav->next;
  }
  return true;
}

/*
<sequence
  id = ID
  maxOccurs = (nonNegativeInteger | unbounded)  : 1
  minOccurs = nonNegativeInteger : 1
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (element | group | choice | sequence | any)*)
</sequence>
*/
static bool schema_sequence(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr seqType,
                            sdlTypePtr cur_type, sdlContentModelPtr model) {
  sdlContentModelPtr newModel(new sdlContentModel());
  newModel->kind = XSD_CONTENT_SEQUENCE;
  if (model) {
    model->u_content.push_back(newModel);
  } else {
    cur_type->model = newModel;
  }
  schema_min_max(seqType, newModel);

  xmlNodePtr trav = seqType->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  while (trav) {
    if (node_is_equal(trav,"element")) {
      schema_element(sdl, tns, trav, cur_type, newModel);
    } else if (node_is_equal(trav,"group")) {
      schema_group(sdl, tns, trav, cur_type, newModel);
    } else if (node_is_equal(trav,"choice")) {
      schema_choice(sdl, tns, trav, cur_type, newModel);
    } else if (node_is_equal(trav,"sequence")) {
      schema_sequence(sdl, tns, trav, cur_type, newModel);
    } else if (node_is_equal(trav,"any")) {
      schema_any(sdl, tns, trav, cur_type, newModel);
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in sequence",
                          trav->name);
    }
    trav = trav->next;
  }
  return true;
}

/*
<any
  id = ID
  maxOccurs = (nonNegativeInteger | unbounded)  : 1
  minOccurs = nonNegativeInteger : 1
  namespace = ((##any | ##other) | List of (anyURI | (##targetNamespace | ##local)) )  : ##any
  processContents = (lax | skip | strict) : strict
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?)
</any>
*/
static bool schema_any(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr anyType,
                       sdlTypePtr cur_type, sdlContentModelPtr model) {
  if (model) {
    sdlContentModelPtr newModel(new sdlContentModel());
    newModel->kind = XSD_CONTENT_ANY;
    schema_min_max(anyType, newModel);
    model->u_content.push_back(newModel);
  }
  return true;
}

/*
<complexContent
  id = ID
  mixed = boolean
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (restriction | extension))
</complexContent>
*/
static bool schema_complexContent(sdlPtr sdl, xmlAttrPtr tns,
                                  xmlNodePtr compCont, sdlTypePtr cur_type) {
  xmlNodePtr trav = compCont->children;
  if (trav && node_is_equal(trav,"annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  if (trav) {
    if (node_is_equal(trav, "restriction")) {
      cur_type->kind = XSD_TYPEKIND_RESTRICTION;
      schema_restriction_complexContent(sdl, tns, trav, cur_type);
      trav = trav->next;
    } else if (node_is_equal(trav, "extension")) {
      cur_type->kind = XSD_TYPEKIND_EXTENSION;
      schema_extension_complexContent(sdl, tns, trav, cur_type);
      trav = trav->next;
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in complexContent",
                          trav->name);
    }
  } else {
    throw SoapException("Parsing Schema: <restriction> or <extension> expected"
                        " in complexContent");
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in complexContent",
                        trav->name);
  }

  return true;
}

/*
<complexType
  abstract = boolean : false
  block = (#all | List of (extension | restriction))
  final = (#all | List of (extension | restriction))
  id = ID
  mixed = boolean : false
  name = NCName
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleContent | complexContent | ((group | all | choice | sequence)?, ((attribute | attributeGroup)*, anyAttribute?))))
</complexType>
*/
static bool schema_complexType(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr compType,
                               sdlTypePtr cur_type) {
  xmlAttrPtr attrs = compType->properties;
  xmlAttrPtr ns = get_attribute(attrs, "targetNamespace");
  if (ns == NULL) {
    ns = tns;
  }

  xmlAttrPtr name = get_attribute(attrs, "name");
  if (cur_type) {
    /* Anonymous type inside <element> */
    sdlTypePtr newType(new sdlType());
    newType->kind = XSD_TYPEKIND_COMPLEX;
    if (name) {
      newType->name = (char*)name->children->content;
      newType->namens = (char*)ns->children->content;
    } else {
      newType->name = cur_type->name;
      newType->namens = cur_type->namens;
    }
    sdl->types.push_back(newType);

    cur_type->encode = encodePtr(new encode());
    cur_type->encode->details.ns = newType->namens;
    cur_type->encode->details.type_str = newType->name;
    cur_type->encode->details.sdl_type = newType.get();
    cur_type->encode->to_xml = sdl_guess_convert_xml;
    cur_type->encode->to_zval = sdl_guess_convert_zval;
    sdl->encoders[lexical_cast<string>(sdl->encoders.size())] =
      cur_type->encode;

    cur_type = newType;

  } else if (name) {
    sdlTypePtr newType(new sdlType());
    newType->kind = XSD_TYPEKIND_COMPLEX;
    newType->name = (char*)name->children->content;
    newType->namens = (char*)ns->children->content;
    sdl->types.push_back(newType);
    cur_type = newType;
    create_encoder(sdl, cur_type, ns->children->content,
                   name->children->content);
  } else {
    throw SoapException("Parsing Schema: complexType has no 'name' attribute");
  }

  xmlNodePtr trav = compType->children;
  if (trav && node_is_equal(trav, "annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  if (trav) {
    if (node_is_equal(trav,"simpleContent")) {
      schema_simpleContent(sdl, tns, trav, cur_type);
      trav = trav->next;
    } else if (node_is_equal(trav,"complexContent")) {
      schema_complexContent(sdl, tns, trav, cur_type);
      trav = trav->next;
    } else {
      if (node_is_equal(trav,"group")) {
        schema_group(sdl, tns, trav, cur_type, sdlContentModelPtr());
        trav = trav->next;
      } else if (node_is_equal(trav,"all")) {
        schema_all(sdl, tns, trav, cur_type, sdlContentModelPtr());
        trav = trav->next;
      } else if (node_is_equal(trav,"choice")) {
        schema_choice(sdl, tns, trav, cur_type, sdlContentModelPtr());
        trav = trav->next;
      } else if (node_is_equal(trav,"sequence")) {
        schema_sequence(sdl, tns, trav, cur_type, sdlContentModelPtr());
        trav = trav->next;
      }
      while (trav) {
        if (node_is_equal(trav,"attribute")) {
          schema_attribute(sdl, tns, trav, cur_type, NULL);
        } else if (node_is_equal(trav,"attributeGroup")) {
          schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
        } else if (node_is_equal(trav,"anyAttribute")) {
          /* TODO: <anyAttribute> support */
          trav = trav->next;
          break;
        } else {
          throw SoapException("Parsing Schema: unexpected <%s> in complexType",
                              trav->name);
        }
        trav = trav->next;
      }
    }
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in complexType",
                        trav->name);
  }
  return true;
}

/*
<element
  abstract = boolean : false
  block = (#all | List of (extension | restriction | substitution))
  default = string
  final = (#all | List of (extension | restriction))
  fixed = string
  form = (qualified | unqualified)
  id = ID
  maxOccurs = (nonNegativeInteger | unbounded)  : 1
  minOccurs = nonNegativeInteger : 1
  name = NCName
  nillable = boolean : false
  ref = QName
  substitutionGroup = QName
  type = QName
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, ((simpleType | complexType)?, (unique | key | keyref)*))
</element>
*/
static bool schema_element(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr element,
                           sdlTypePtr cur_type, sdlContentModelPtr model) {
  xmlAttrPtr attrs = element->properties;
  xmlAttrPtr ns = get_attribute(attrs, "targetNamespace");
  if (ns == NULL) {
    ns = tns;
  }

  xmlAttrPtr ref = NULL;
  xmlAttrPtr name = get_attribute(attrs, "name");
  if (name == NULL) {
    name = ref = get_attribute(attrs, "ref");
  }

  if (name) {
    sdlTypePtr newType(new sdlType());
    if (ref) {
      string type, ns;
      parse_namespace(ref->children->content, type, ns);
      xmlNsPtr nsptr = xmlSearchNs(element->doc, element, NS_STRING(ns));
      string nscat;
      if (nsptr) {
        nscat += (char*)nsptr->href;
        nscat += ':';
        newType->namens = (char*)nsptr->href;
      }
      nscat += type;
      newType->name = type;
      newType->ref = nscat;
    } else {
      newType->name = (char*)name->children->content;
      newType->namens = (char*)ns->children->content;
    }

    if (cur_type == NULL) {
      string key = newType->namens;
      key += ':';
      key += newType->name;

      sdlTypePtr &type = sdl->elements[key];
      if (type) {
        throw SoapException("Parsing Schema: element '%s' already defined",
                            key.c_str());
      }
      type = newType;
    } else {
      cur_type->elements.push_back(newType);
    }

    if (model) {
      sdlContentModelPtr newModel(new sdlContentModel());
      newModel->kind = XSD_CONTENT_ELEMENT;
      newModel->u_element = newType.get();
      schema_min_max(element, newModel);
      model->u_content.push_back(newModel);
    }
    cur_type = newType;
  } else {
    throw SoapException("Parsing Schema: element has no 'name' nor 'ref' "
                        "attributes");
  }

  /* nillable = boolean : false */
  attrs = element->properties;
  xmlAttrPtr attr = get_attribute(attrs, "nillable");
  if (attr) {
    if (ref) {
      throw SoapException("Parsing Schema: element has both 'ref' and "
                          "'nillable' attributes");
    }
    if (!strcmp((char*)attr->children->content, "true") ||
        !strcmp((char*)attr->children->content, "1")) {
      cur_type->nillable = true;
    } else {
      cur_type->nillable = false;
    }
  } else {
    cur_type->nillable = false;
  }

  attr = get_attribute(attrs, "fixed");
  if (attr) {
    if (ref) {
      throw SoapException("Parsing Schema: element has both 'ref' and 'fixed' "
                          "attributes");
    }
    cur_type->fixed = (char*)attr->children->content;
  }

  attr = get_attribute(attrs, "default");
  if (attr) {
    if (ref) {
      throw SoapException("Parsing Schema: element has both 'ref' and 'fixed' "
                          "attributes");
    } else if (ref) {
      throw SoapException("Parsing Schema: element has both 'default' and "
                          "'fixed' attributes");
    }
    cur_type->def = (char*)attr->children->content;
  }

  /* form */
  attr = get_attribute(attrs, "form");
  if (attr) {
    if (strncmp((char*)attr->children->content, "qualified",
                sizeof("qualified")) == 0) {
      cur_type->form = XSD_FORM_QUALIFIED;
    } else if (strncmp((char*)attr->children->content, "unqualified",
                       sizeof("unqualified")) == 0) {
      cur_type->form = XSD_FORM_UNQUALIFIED;
    } else {
      cur_type->form = XSD_FORM_DEFAULT;
    }
  } else {
    cur_type->form = XSD_FORM_DEFAULT;
  }
  if (cur_type->form == XSD_FORM_DEFAULT) {
     xmlNodePtr parent = element->parent;
     while (parent) {
       if (node_is_equal_ex(parent, "schema", SCHEMA_NAMESPACE)) {
        xmlAttrPtr def = get_attribute(parent->properties,
                                       "elementFormDefault");
        if (def == NULL ||
            strncmp((char*)def->children->content, "qualified",
                    sizeof("qualified"))) {
          cur_type->form = XSD_FORM_UNQUALIFIED;
        } else {
          cur_type->form = XSD_FORM_QUALIFIED;
        }
        break;
      }
      parent = parent->parent;
    }
    if (parent == NULL) {
      cur_type->form = XSD_FORM_UNQUALIFIED;
    }
  }

  /* type = QName */
  xmlAttrPtr type = get_attribute(attrs, "type");
  if (type) {
    string cptype, str_ns;
    if (ref) {
      throw SoapException("Parsing Schema: element has both 'ref' and 'type' "
                          "attributes");
    }
    parse_namespace(type->children->content, cptype, str_ns);
    xmlNsPtr nsptr = xmlSearchNs(element->doc, element, NS_STRING(str_ns));
    if (nsptr) {
      cur_type->encode = get_create_encoder(sdl, cur_type, nsptr->href,
                                            BAD_CAST(cptype.c_str()));
    }
  }

  xmlNodePtr trav = element->children;
  if (trav && node_is_equal(trav, "annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  if (trav) {
    if (node_is_equal(trav,"simpleType")) {
      if (ref) {
        throw SoapException("Parsing Schema: element has both 'ref' "
                            "attribute and subtype");
      } else if (type) {
        throw SoapException("Parsing Schema: element has both 'type' "
                            "attribute and subtype");
      }
      schema_simpleType(sdl, tns, trav, cur_type);
      trav = trav->next;
    } else if (node_is_equal(trav,"complexType")) {
      if (ref) {
        throw SoapException("Parsing Schema: element has both 'ref' "
                            "attribute and subtype");
      } else if (type) {
        throw SoapException("Parsing Schema: element has both 'type' "
                            "attribute and subtype");
      }
      schema_complexType(sdl, tns, trav, cur_type);
      trav = trav->next;
    }
  }
  while (trav) {
    if (node_is_equal(trav,"unique")) {
      /* TODO: <unique> support */
    } else if (node_is_equal(trav,"key")) {
      /* TODO: <key> support */
    } else if (node_is_equal(trav,"keyref")) {
      /* TODO: <keyref> support */
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in element",
                          trav->name);
    }
    trav = trav->next;
  }

  return true;
}

/*
<attribute
  default = string
  fixed = string
  form = (qualified | unqualified)
  id = ID
  name = NCName
  ref = QName
  type = QName
  use = (optional | prohibited | required) : optional
  {any attributes with non-schema namespace . . .}>
  Content: (annotation?, (simpleType?))
</attribute>
*/
static bool schema_attribute(sdlPtr sdl, xmlAttrPtr tns, xmlNodePtr attrType,
                             sdlTypePtr cur_type, sdlCtx *ctx) {
  sdlAttributePtr newAttr;
  xmlAttrPtr attr, ref = NULL, type = NULL;

  xmlAttrPtr name = get_attribute(attrType->properties, "name");
  if (name == NULL) {
    name = ref = get_attribute(attrType->properties, "ref");
  }
  if (name) {
    newAttr = sdlAttributePtr(new sdlAttribute());

    string key;
    if (ref) {
      string attr_name, ns;
      parse_namespace(ref->children->content, attr_name, ns);
      xmlNsPtr nsptr = xmlSearchNs(attrType->doc, attrType, NS_STRING(ns));
      if (nsptr) {
        key += (char*)nsptr->href;
        key += ':';
        newAttr->namens = (char*)nsptr->href;
      }
      key += attr_name;
      newAttr->ref = key;
    } else {
      xmlAttrPtr ns = get_attribute(attrType->properties, "targetNamespace");
      if (ns == NULL) {
        ns = tns;
      }
      if (ns) {
        key += (char*)ns->children->content;
        key += ':';
        newAttr->namens = (char*)ns->children->content;
      }
      key += (char*)name->children->content;
    }

    sdlAttributeMap *addHash;
    if (cur_type == NULL) {
      addHash = &ctx->attributes;
    } else {
      addHash = &cur_type->attributes;
    }

    sdlAttributePtr &rattr = (*addHash)[key];
    if (rattr) {
      throw SoapException("Parsing Schema: attribute '%s' already defined",
                          key.c_str());
    }
    rattr = newAttr;
  } else{
    throw SoapException("Parsing Schema: attribute has no 'name' nor 'ref' "
                        "attributes");
  }

  /* type = QName */
  type = get_attribute(attrType->properties, "type");
  if (type) {
    if (ref) {
      throw SoapException("Parsing Schema: attribute has both 'ref' and "
                          "'type' attributes");
    }
    string cptype, str_ns;
    parse_namespace(type->children->content, cptype, str_ns);
    xmlNsPtr nsptr = xmlSearchNs(attrType->doc, attrType, NS_STRING(str_ns));
    if (nsptr) {
      newAttr->encode = get_create_encoder(sdl, cur_type, nsptr->href,
                                           BAD_CAST(cptype.c_str()));
    }
  }

  attr = attrType->properties;
  while (attr) {
    if (attr_is_equal_ex(attr, "default", SCHEMA_NAMESPACE)) {
      newAttr->def = (char*)attr->children->content;
    } else if (attr_is_equal_ex(attr, "fixed", SCHEMA_NAMESPACE)) {
      newAttr->fixed = (char*)attr->children->content;
    } else if (attr_is_equal_ex(attr, "form", SCHEMA_NAMESPACE)) {
      if (strncmp((char*)attr->children->content, "qualified",
                  sizeof("qualified")) == 0) {
        newAttr->form = XSD_FORM_QUALIFIED;
      } else if (strncmp((char*)attr->children->content, "unqualified",
                         sizeof("unqualified")) == 0) {
        newAttr->form = XSD_FORM_UNQUALIFIED;
      } else {
        newAttr->form = XSD_FORM_DEFAULT;
      }
    } else if (attr_is_equal_ex(attr, "id", SCHEMA_NAMESPACE)) {
      /* skip */
    } else if (attr_is_equal_ex(attr, "name", SCHEMA_NAMESPACE)) {
      newAttr->name = (char*)attr->children->content;
    } else if (attr_is_equal_ex(attr, "ref", SCHEMA_NAMESPACE)) {
      /* already processed */
    } else if (attr_is_equal_ex(attr, "type", SCHEMA_NAMESPACE)) {
      /* already processed */
    } else if (attr_is_equal_ex(attr, "use", SCHEMA_NAMESPACE)) {
      if (strncmp((char*)attr->children->content, "prohibited",
                  sizeof("prohibited")) == 0) {
        newAttr->use = XSD_USE_PROHIBITED;
      } else if (strncmp((char*)attr->children->content, "required",
                         sizeof("required")) == 0) {
        newAttr->use = XSD_USE_REQUIRED;
      } else if (strncmp((char*)attr->children->content, "optional",
                         sizeof("optional")) == 0) {
        newAttr->use = XSD_USE_OPTIONAL;
      } else {
        newAttr->use = XSD_USE_DEFAULT;
      }
    } else {
      xmlNsPtr nsPtr = attr_find_ns(attr);
      if (strncmp((char*)nsPtr->href, SCHEMA_NAMESPACE,
                  sizeof(SCHEMA_NAMESPACE))) {
        sdlExtraAttributePtr ext(new sdlExtraAttribute());
        string value, ns;
        parse_namespace(attr->children->content, value, ns);
        xmlNsPtr nsptr = xmlSearchNs(attr->doc, attr->parent, NS_STRING(ns));
        if (nsptr) {
          ext->ns = (char*)nsptr->href;
          ext->val = value;
        } else {
          ext->val = (char*)attr->children->content;
        }

        string key2;
        key2 += (char*)nsPtr->href;
        key2 += ':';
        key2 += (char*)attr->name;
        newAttr->extraAttributes[key2] = ext;
      }
    }
    attr = attr->next;
  }
  if (newAttr->form == XSD_FORM_DEFAULT) {
     xmlNodePtr parent = attrType->parent;
     while (parent) {
      if (node_is_equal_ex(parent, "schema", SCHEMA_NAMESPACE)) {
        xmlAttrPtr def = get_attribute(parent->properties,
                                       "attributeFormDefault");
        if (def == NULL ||
            strncmp((char*)def->children->content, "qualified",
                    sizeof("qualified"))) {
          newAttr->form = XSD_FORM_UNQUALIFIED;
        } else {
          newAttr->form = XSD_FORM_QUALIFIED;
        }
        break;
      }
      parent = parent->parent;
    }
    if (parent == NULL) {
      newAttr->form = XSD_FORM_UNQUALIFIED;
    }
  }

  xmlNodePtr trav = attrType->children;
  if (trav && node_is_equal(trav, "annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  if (trav) {
    if (node_is_equal(trav,"simpleType")) {
      if (ref) {
        throw SoapException("Parsing Schema: attribute has both 'ref' "
                            "attribute and subtype");
      } else if (type) {
        throw SoapException("Parsing Schema: attribute has both 'type' "
                            "attribute and subtype");
      }
      sdlTypePtr dummy_type(new sdlType());
      dummy_type->name = string("anonymous") +
        lexical_cast<string>(sdl->types.size());
      dummy_type->namens = (char*)tns->children->content;
      schema_simpleType(sdl, tns, trav, dummy_type);
      newAttr->encode = dummy_type->encode;
      trav = trav->next;
    }
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in attribute",
                        trav->name);
  }
  return true;
}

static bool schema_attributeGroup(sdlPtr sdl, xmlAttrPtr tns,
                                  xmlNodePtr attrGroup, sdlTypePtr cur_type,
                                  sdlCtx *ctx) {
  xmlAttrPtr ref = NULL;
  xmlAttrPtr name = get_attribute(attrGroup->properties, "name");
  if (name == NULL) {
    name = ref = get_attribute(attrGroup->properties, "ref");
  }
  if (name) {
    if (!cur_type) {
      xmlAttrPtr ns = get_attribute(attrGroup->properties, "targetNamespace");
      if (ns == NULL) {
        ns = tns;
      }
      sdlTypePtr newType(new sdlType());
      newType->name = (char*)name->children->content;
      newType->namens = (char*)ns->children->content;

      string key;
      key += newType->namens;
      key += ':';
      key += newType->name;

      sdlTypePtr &type = ctx->attributeGroups[key];
      if (type) {
        throw SoapException("Parsing Schema: attributeGroup '%s' already"
                            " defined", key.c_str());
      } else {
        type = newType;
      }
      cur_type = newType;
    } else if (ref) {
      sdlAttributePtr newAttr(new sdlAttribute());
      string group_name, ns;
      parse_namespace(ref->children->content, group_name, ns);
      xmlNsPtr nsptr = xmlSearchNs(attrGroup->doc, attrGroup, NS_STRING(ns));
      string key;
      if (nsptr) {
        key += (char*)nsptr->href;
        key += ':';
      }
      key += group_name;
      newAttr->ref = key;
      cur_type->attributes[lexical_cast<string>(cur_type->attributes.size())] =
        newAttr;
      cur_type = sdlTypePtr();
    }
  } else{
    throw SoapException("Parsing Schema: attributeGroup has no 'name' nor"
                        " 'ref' attributes");
  }

  xmlNodePtr trav = attrGroup->children;
  if (trav && node_is_equal(trav, "annotation")) {
    /* TODO: <annotation> support */
    trav = trav->next;
  }
  while (trav) {
    if (node_is_equal(trav,"attribute")) {
      if (ref) {
        throw SoapException("Parsing Schema: attributeGroup has both 'ref' "
                            "attribute and subattribute");
      }
      schema_attribute(sdl, tns, trav, cur_type, NULL);
    } else if (node_is_equal(trav,"attributeGroup")) {
      if (ref) {
        throw SoapException("Parsing Schema: attributeGroup has both 'ref' "
                            "attribute and subattribute");
      }
      schema_attributeGroup(sdl, tns, trav, cur_type, NULL);
    } else if (node_is_equal(trav,"anyAttribute")) {
      if (ref) {
        throw SoapException("Parsing Schema: attributeGroup has both 'ref' "
                            "attribute and subattribute");
      }
      /* TODO: <anyAttribute> support */
      trav = trav->next;
      break;
    } else {
      throw SoapException("Parsing Schema: unexpected <%s> in attributeGroup",
                          trav->name);
    }
    trav = trav->next;
  }
  if (trav) {
    throw SoapException("Parsing Schema: unexpected <%s> in attributeGroup",
                  trav->name);
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// schema_pass2

static void copy_extra_attributes(sdlAttributePtr dest, sdlAttributePtr src) {
  for (sdlExtraAttributeMap::const_iterator iter =
         src->extraAttributes.begin(); iter != src->extraAttributes.end();
       ++iter) {
    sdlExtraAttributePtr eattr(new sdlExtraAttribute());
    eattr->ns = iter->second->ns;
    eattr->val = iter->second->val;
    dest->extraAttributes[iter->first] = eattr;
  }
}

static void schema_attribute_fixup(sdlCtx *ctx, sdlAttributePtr attr) {
  if (!attr->ref.empty()) {
    sdlAttributeMap::const_iterator iter = ctx->attributes.find(attr->ref);
    if (iter != ctx->attributes.end()) {
      sdlAttributePtr tmp = iter->second;
      schema_attribute_fixup(ctx, tmp);
      if (!tmp->name.empty() && attr->name.empty()) {
        attr->name = tmp->name;
      }
      if (!tmp->namens.empty() && attr->namens.empty()) {
        attr->namens = tmp->namens;
      }
      if (!tmp->def.empty() && attr->def.empty()) {
        attr->def = tmp->def;
      }
      if (!tmp->fixed.empty() && attr->fixed.empty()) {
        attr->fixed = tmp->fixed;
      }
      if (attr->form == XSD_FORM_DEFAULT) {
        attr->form = tmp->form;
      }
      if (attr->use == XSD_USE_DEFAULT) {
        attr->use = tmp->use;
      }
      copy_extra_attributes(attr, tmp);
      attr->encode = tmp->encode;
    }
    if (attr->name.empty() && !attr->ref.empty()) {
      const char *name = strrchr(attr->ref.c_str(), ':');
      if (name) {
        attr->name = name+1;
      } else{
        attr->name = attr->ref;
      }
    }
    attr->ref.clear();
  }
}

static bool schema_attributegroup_fixup(sdlCtx *ctx, sdlAttributePtr attr,
                                        sdlAttributeMap &fixed) {
  if (!attr->ref.empty()) {
    sdlTypeMap::iterator iter = ctx->attributeGroups.find(attr->ref);
    if (iter != ctx->attributeGroups.end()) {
      sdlAttributeMap &attributes = iter->second->attributes;
      for (sdlAttributeMap::iterator it = attributes.begin();
           it != attributes.end(); ++it) {
        if (isdigit(it->first[0])) {
          if (!schema_attributegroup_fixup(ctx, it->second, fixed)) {
            fixed[it->first] = it->second;
          }
        } else {
          schema_attribute_fixup(ctx, it->second);
          sdlAttributePtr newAttr(new sdlAttribute());
          *newAttr = *it->second;
          copy_extra_attributes(newAttr, it->second);
          fixed[it->first] = newAttr;
        }
      }
      attr->ref.clear();
      return true;
    }
    attr->ref.clear();
  }
  return false;
}

static void schema_content_model_fixup(sdlCtx *ctx, sdlContentModelPtr model) {
  switch (model->kind) {
  case XSD_CONTENT_GROUP_REF: {
    sdlTypeMap::iterator iter = ctx->sdl->groups.find(model->u_group_ref);
    if (iter != ctx->sdl->groups.end()) {
      schema_type_fixup(ctx, iter->second);
      model->u_group_ref.clear();
      model->kind = XSD_CONTENT_GROUP;
      model->u_group = iter->second.get();
    } else {
      throw SoapException("Parsing Schema: unresolved group 'ref' attribute");
    }
    break;
  }
  case XSD_CONTENT_CHOICE:
    if (model->max_occurs != 1) {
      for (unsigned int i = 0; i < model->u_content.size(); i++) {
        sdlContentModelPtr tmp = model->u_content[i];
        tmp->min_occurs = 0;
        tmp->max_occurs = model->max_occurs;
      }
      model->kind = XSD_CONTENT_ALL;
      model->min_occurs = 1;
      model->max_occurs = 1;
    }
    // fall through
  case XSD_CONTENT_SEQUENCE:
  case XSD_CONTENT_ALL:
    for (unsigned int i = 0; i < model->u_content.size(); i++) {
      sdlContentModelPtr tmp = model->u_content[i];
      schema_content_model_fixup(ctx, tmp);
    }
    break;
  default:
    break;
  }
}

static void schema_type_fixup(sdlCtx *ctx, sdlTypePtr type) {
  if (!type->ref.empty()) {
    sdlTypeMap::const_iterator iter = ctx->sdl->elements.find(type->ref);
    if (iter != ctx->sdl->elements.end()) {
      const sdlTypePtr src = iter->second;
      type->kind = src->kind;
      type->encode = src->encode;
      if (src->nillable) {
        type->nillable = true;
      }
      if (!src->fixed.empty()) {
        type->fixed = src->fixed;
      }
      if (!src->def.empty()) {
        type->def = src->def;
      }
      type->form = src->form;
    } else if (type->ref == SCHEMA_NAMESPACE ":schema") {
      type->encode = get_conversion(XSD_ANYXML);
    } else {
      throw SoapException("Parsing Schema: unresolved element 'ref' "
                          "attribute");
    }
    type->ref.clear();
  }

  for (unsigned int i = 0; i < type->elements.size(); i++) {
    schema_type_fixup(ctx, type->elements[i]);
  }

  if (type->model) {
    schema_content_model_fixup(ctx, type->model);
  }

  sdlAttributeMap fixed;
  for (sdlAttributeMap::iterator iter = type->attributes.begin();
       iter != type->attributes.end(); ++iter) {
    if (isdigit(iter->first[0])) {
      if (!schema_attributegroup_fixup(ctx, iter->second, fixed)) {
        fixed[iter->first] = iter->second;
      }
    } else {
      schema_attribute_fixup(ctx, iter->second);
      fixed[iter->first] = iter->second;
    }
  }
  type->attributes = fixed;
}

void schema_pass2(sdlCtx *ctx) {
  for (sdlAttributeMap::iterator iter = ctx->attributes.begin();
       iter != ctx->attributes.end(); ++iter) {
    schema_attribute_fixup(ctx, iter->second);
  }
  for (sdlTypeMap::iterator iter = ctx->attributeGroups.begin();
       iter != ctx->attributeGroups.end(); ++iter) {
    schema_type_fixup(ctx, iter->second);
  }

  sdlPtr sdl = ctx->sdl;
  for (sdlTypeMap::iterator iter = sdl->elements.begin();
       iter != sdl->elements.end(); ++iter) {
    schema_type_fixup(ctx, iter->second);
  }
  for (sdlTypeMap::iterator iter = sdl->groups.begin();
       iter != sdl->groups.end(); ++iter) {
    schema_type_fixup(ctx, iter->second);
  }
  for (unsigned int i = 0; i < sdl->types.size(); i++) {
    schema_type_fixup(ctx, sdl->types[i]);
  }

  ctx->attributes.clear();
  ctx->attributeGroups.clear();
}

///////////////////////////////////////////////////////////////////////////////
}
