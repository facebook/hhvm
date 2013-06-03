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

encodePtr get_encoder_from_prefix(sdl *sdl, xmlNodePtr node,
                                  const xmlChar *type) {
  string ns, cptype;
  parse_namespace(type, cptype, ns);
  xmlNsPtr nsptr = xmlSearchNs(node->doc, node, NS_STRING(ns));
  encodePtr enc;
  if (nsptr) {
    enc = get_encoder(sdl, (char*)nsptr->href, cptype.c_str());
    if (enc == NULL) {
      enc = get_encoder_ex(sdl, cptype);
    }
  } else {
    enc = get_encoder_ex(sdl, (char*)type);
  }
  return enc;
}

static sdlTypePtr get_element(sdl *sdl, xmlNodePtr node, const xmlChar *type) {
  sdlTypePtr ret;
  if (!sdl->elements.empty()) {
    string ns, cptype;
    parse_namespace(type, cptype, ns);
    xmlNsPtr nsptr = xmlSearchNs(node->doc, node, NS_STRING(ns));
    if (nsptr) {
      string nscat = (char*)nsptr->href;
      nscat += ':';
      nscat += cptype;
      sdlTypeMap::iterator iter = sdl->elements.find(nscat);
      if (iter != sdl->elements.end()) {
        ret = iter->second;
      } else {
        iter = sdl->elements.find((char*)type);
        if (iter != sdl->elements.end()) {
          ret = iter->second;
        }
      }
    } else {
      sdlTypeMap::iterator iter = sdl->elements.find((char*)type);
      if (iter != sdl->elements.end()) {
        ret = iter->second;
      }
    }
  }
  return ret;
}

encodePtr get_encoder(sdl *sdl, const char *ns, const char *type) {
  string nscat = ns;
  nscat += ':';
  nscat += type;
  encodePtr enc = get_encoder_ex(sdl, nscat);

  if (!enc &&
      (strcmp(ns, SOAP_1_1_ENC_NAMESPACE) == 0 ||
       strcmp(ns, SOAP_1_2_ENC_NAMESPACE) == 0)) {
    string enc_nscat = XSD_NAMESPACE;
    enc_nscat += ':';
    enc_nscat += type;
    enc = get_encoder_ex(NULL, enc_nscat);
    if (enc && sdl) {
      encodePtr new_enc(new encode());
      *new_enc = *enc;
      sdl->encoders[nscat] = new_enc;
      enc = new_enc;
    }
  }
  return enc;
}

encodePtr get_encoder_ex(sdl *sdl, const std::string &nscat) {
  USE_SOAP_GLOBAL;
  encodeMap::iterator iter = SOAP_GLOBAL(defEnc).find(nscat);
  if (iter != SOAP_GLOBAL(defEnc).end()) {
    return iter->second;
  }
  if (sdl) {
    iter = sdl->encoders.find(nscat);
    if (iter != sdl->encoders.end()) {
      return iter->second;
    }
  }
  return encodePtr();
}

sdlBindingPtr get_binding_from_type(sdl *sdl, int type) {
  if (sdl) {
    for (sdlBindingMap::iterator iter = sdl->bindings.begin();
         iter != sdl->bindings.end(); ++iter) {
      if (iter->second->bindingType == type) {
        return iter->second;
      }
    }
  }
  return sdlBindingPtr();
}

sdlBindingPtr get_binding_from_name(sdl *sdl, char *name, char *ns) {
  string key = ns;
  key += ':';
  key += name;
  sdlBindingMap::iterator iter = sdl->bindings.find(key);
  if (iter != sdl->bindings.end()) {
    return iter->second;
  }
  return sdlBindingPtr();
}

static bool is_wsdl_element(xmlNodePtr node) {
  if (node->ns && strcmp((char*)node->ns->href, WSDL_NAMESPACE) != 0) {
    xmlAttrPtr attr = get_attribute_ex(node->properties, "required",
                                       WSDL_NAMESPACE);
    if (attr && attr->children && attr->children->content &&
        (strcmp((char*)attr->children->content, "1") == 0 ||
         strcmp((char*)attr->children->content, "true") == 0)) {
      throw SoapException("Parsing WSDL: Unknown required WSDL extension '%s'",
                          node->ns->href);
    }
    return false;
  }
  return true;
}

static void load_wsdl_ex(char *struri, sdlCtx *ctx, bool include,
                         HttpClient *http) {
  if (ctx->docs.find(struri) != ctx->docs.end()) {
    return;
  }

  xmlDocPtr wsdl;
  if (http) {
    HeaderMap headers;
    StringBuffer response;
    int code = http->get(struri, response, &headers);
    if (code != 200) {
      throw SoapException("Parsing WSDL: Couldn't load from '%s'", struri);
    }
    String msg = response.detach();
    wsdl = soap_xmlParseMemory(msg.data(), msg.size(), false);
    if (wsdl) {
      wsdl->URL = xmlCharStrdup(struri);
    }
  } else {
    wsdl = soap_xmlParseFile(struri);
  }
  if (!wsdl) {
    xmlErrorPtr xmlErrorPtr = xmlGetLastError();
    if (xmlErrorPtr) {
      throw SoapException("Parsing WSDL: Couldn't load from '%s' : %s",
                          struri, xmlErrorPtr->message);
    } else {
      throw SoapException("Parsing WSDL: Couldn't load from '%s'", struri);
    }
  }

  sdlPtr tmpsdl = ctx->sdl;
  ctx->docs[struri] = wsdl;

  xmlNodePtr root = wsdl->children;
  xmlNodePtr definitions = get_node_ex(root, "definitions", WSDL_NAMESPACE);
  if (!definitions) {
    if (include) {
      xmlNodePtr schema = get_node_ex(root, "schema", XSD_NAMESPACE);
      if (schema) {
        load_schema(ctx, schema);
        return;
      }
    }
    throw SoapException("Parsing WSDL: Couldn't find <definitions> in '%s'",
                        struri);
  }

  xmlAttrPtr targetNamespace;
  if (!include) {
    targetNamespace = get_attribute(definitions->properties,
                                    "targetNamespace");
    if (targetNamespace) {
      tmpsdl->target_ns = (char*)targetNamespace->children->content;
    }
  }

  xmlNodePtr trav = definitions->children;
  while (trav) {
    if (!is_wsdl_element(trav)) {
      trav = trav->next;
      continue;
    }
    if (node_is_equal(trav,"types")) {
      /* TODO: Only one "types" is allowed */
      xmlNodePtr trav2 = trav->children;
      while (trav2) {
        if (node_is_equal_ex(trav2, "schema", XSD_NAMESPACE)) {
          load_schema(ctx, trav2);
        } else if (is_wsdl_element(trav2) &&
                   !node_is_equal(trav2,"documentation")) {
          throw SoapException("Parsing WSDL: Unexpected WSDL element <%s>",
                              trav2->name);
        }
        trav2 = trav2->next;
      }
    } else if (node_is_equal(trav,"import")) {
      /* TODO: namespace ??? */
      xmlAttrPtr tmp = get_attribute(trav->properties, "location");
      if (tmp) {
        xmlChar *uri;
        xmlChar *base = xmlNodeGetBase(trav->doc, trav);
        if (base == NULL) {
          uri = xmlBuildURI(tmp->children->content, trav->doc->URL);
        } else {
          uri = xmlBuildURI(tmp->children->content, base);
          xmlFree(base);
        }
        load_wsdl_ex((char*)uri, ctx, true, NULL);
        xmlFree(uri);
      }

    } else if (node_is_equal(trav,"message")) {
      xmlAttrPtr name = get_attribute(trav->properties, "name");
      if (name && name->children && name->children->content) {
        char *content = (char*)name->children->content;
        if (ctx->messages.find(content) != ctx->messages.end()) {
          throw SoapException("Parsing WSDL: <message> '%s' already defined",
                              content);
        } else {
          ctx->messages[content] = trav;
        }
      } else {
        throw SoapException("Parsing WSDL: <message> hasn't name attribute");
      }

    } else if (node_is_equal(trav,"portType")) {
      xmlAttrPtr name = get_attribute(trav->properties, "name");
      if (name && name->children && name->children->content) {
        char *content = (char*)name->children->content;
        if (ctx->portTypes.find(content) != ctx->portTypes.end()) {
          throw SoapException("Parsing WSDL: <portType> '%s' already defined",
                              content);
        } else {
          ctx->portTypes[content] = trav;
        }
      } else {
        throw SoapException("Parsing WSDL: <portType> hasn't name attribute");
      }

    } else if (node_is_equal(trav,"binding")) {
      xmlAttrPtr name = get_attribute(trav->properties, "name");
      if (name && name->children && name->children->content) {
        char *content = (char*)name->children->content;
        if (ctx->bindings.find(content) != ctx->bindings.end()) {
          throw SoapException("Parsing WSDL: <binding> '%s' already defined",
                              content);
        } else {
          ctx->bindings[content] = trav;
        }
      } else {
        throw SoapException("Parsing WSDL: <binding> hasn't name attribute");
      }

    } else if (node_is_equal(trav,"service")) {
      xmlAttrPtr name = get_attribute(trav->properties, "name");
      if (name && name->children && name->children->content) {
        char *content = (char*)name->children->content;
        if (ctx->services.find(content) != ctx->services.end()) {
          throw SoapException("Parsing WSDL: <service> '%s' already defined",
                              content);
        } else {
          ctx->services[content] = trav;
        }
      } else {
        throw SoapException("Parsing WSDL: <service> hasn't name attribute");
      }
    } else if (!node_is_equal(trav,"documentation")) {
      throw SoapException("Parsing WSDL: Unexpected WSDL element <%s>",
                          trav->name);
    }
    trav = trav->next;
  }
}

static sdlSoapBindingFunctionHeaderPtr wsdl_soap_binding_header
(sdlCtx* ctx, xmlNodePtr header, char* wsdl_soap_namespace, bool fault) {
  xmlAttrPtr tmp = get_attribute(header->properties, "message");
  if (!tmp) {
    throw SoapException("Parsing WSDL: Missing message attribute for <header>");
  }

  char *ctype = strrchr((char*)tmp->children->content,':');
  if (ctype == NULL) {
    ctype = (char*)tmp->children->content;
  } else {
    ++ctype;
  }
  xmlNodeMap::iterator iter = ctx->messages.find(ctype);
  if (iter == ctx->messages.end()) {
    throw SoapException("Parsing WSDL: Missing <message> with name '%s'",
                        tmp->children->content);
  }
  xmlNodePtr message = iter->second;

  tmp = get_attribute(header->properties, "part");
  if (!tmp) {
    throw SoapException("Parsing WSDL: Missing part attribute for <header>");
  }
  xmlNodePtr part = get_node_with_attribute_ex
    (message->children, "part", WSDL_NAMESPACE, "name",
     (char*)tmp->children->content, NULL);
  if (!part) {
    throw SoapException("Parsing WSDL: Missing part '%s' in <message>",
                        tmp->children->content);
  }

  sdlSoapBindingFunctionHeaderPtr h(new sdlSoapBindingFunctionHeader());
  h->name = (char*)tmp->children->content;

  tmp = get_attribute(header->properties, "use");
  if (tmp &&
      !strncmp((char*)tmp->children->content, "encoded", sizeof("encoded"))) {
    h->use = SOAP_ENCODED;
  } else {
    h->use = SOAP_LITERAL;
  }

  tmp = get_attribute(header->properties, "namespace");
  if (tmp) {
    h->ns = (char*)tmp->children->content;
  }

  if (h->use == SOAP_ENCODED) {
    tmp = get_attribute(header->properties, "encodingStyle");
    if (tmp) {
      if (strncmp((char*)tmp->children->content, SOAP_1_1_ENC_NAMESPACE,
                  sizeof(SOAP_1_1_ENC_NAMESPACE)) == 0) {
        h->encodingStyle = SOAP_ENCODING_1_1;
      } else if (strncmp((char*)tmp->children->content, SOAP_1_2_ENC_NAMESPACE,
                         sizeof(SOAP_1_2_ENC_NAMESPACE)) == 0) {
        h->encodingStyle = SOAP_ENCODING_1_2;
      } else {
        throw SoapException("Parsing WSDL: Unknown encodingStyle '%s'",
                            tmp->children->content);
      }
    } else {
      throw SoapException("Parsing WSDL: Unspecified encodingStyle");
    }
  }

  tmp = get_attribute(part->properties, "type");
  if (tmp) {
    h->encode = get_encoder_from_prefix(ctx->sdl.get(), part,
                                        tmp->children->content);
  } else {
    tmp = get_attribute(part->properties, "element");
    if (tmp) {
      h->element = get_element(ctx->sdl.get(), part, tmp->children->content);
      if (h->element) {
        h->encode = h->element->encode;
        if (h->ns.empty() && !h->element->namens.empty()) {
          h->ns = h->element->namens;
        }
        if (!h->element->name.empty()) {
          h->name = h->element->name;
        }
      }
    }
  }
  if (!fault) {
    xmlNodePtr trav = header->children;
    while (trav) {
      if (node_is_equal_ex(trav, "headerfault", wsdl_soap_namespace)) {
        sdlSoapBindingFunctionHeaderPtr hf =
          wsdl_soap_binding_header(ctx, trav, wsdl_soap_namespace, true);
        string key;
        if (!hf->ns.empty()) {
          key += hf->ns;
          key += ':';
        }
        key += hf->name;
        h->headerfaults[key] = hf;
      } else if (is_wsdl_element(trav) &&
                 !node_is_equal(trav,"documentation")) {
        throw SoapException("Parsing WSDL: Unexpected WSDL element <%s>",
                            trav->name);
      }
      trav = trav->next;
    }
  }
  return h;
}

static void wsdl_soap_binding_body(sdlCtx* ctx, xmlNodePtr node,
                                   char* wsdl_soap_namespace,
                                   sdlSoapBindingFunctionBody *binding,
                                   sdlParamVec &params) {
  xmlNodePtr body;
  xmlAttrPtr tmp;
  xmlNodePtr trav = node->children;
  while (trav) {
    if (node_is_equal_ex(trav, "body", wsdl_soap_namespace)) {
      body = trav;
      tmp = get_attribute(body->properties, "use");
      if (tmp &&
          !strncmp((char*)tmp->children->content, "literal",
                   sizeof("literal"))) {
        binding->use = SOAP_LITERAL;
      } else {
        binding->use = SOAP_ENCODED;
      }

      tmp = get_attribute(body->properties, "namespace");
      if (tmp) {
        binding->ns = (char*)tmp->children->content;
      }

      tmp = get_attribute(body->properties, "parts");
      if (tmp) {
        sdlParamVec ht;
        char *parts = (char*)tmp->children->content;

        /* Delete all parts those are not in the "parts" attribute */
        while (*parts) {
          bool found = false;
          while (*parts == ' ') ++parts;
          if (*parts == '\0') break;
          char *end = strchr(parts, ' ');
          if (end) *end = '\0';
          for (unsigned int i = 0; i < params.size(); i++) {
            sdlParamPtr param = params[i];
            if (param->paramName == parts) {
              sdlParamPtr x_param(new sdlParam);
              *x_param = *param;
              ht.push_back(x_param);
              found = true;
              break;
            }
          }
          if (!found) {
            throw SoapException("Parsing WSDL: Missing part '%s' in <message>",
                                parts);
          }
          parts += strlen(parts);
          if (end) *end = ' ';
        }
        params = ht;
      }

      if (binding->use == SOAP_ENCODED) {
        tmp = get_attribute(body->properties, "encodingStyle");
        if (tmp) {
          if (strncmp((char*)tmp->children->content, SOAP_1_1_ENC_NAMESPACE,
                      sizeof(SOAP_1_1_ENC_NAMESPACE)) == 0) {
            binding->encodingStyle = SOAP_ENCODING_1_1;
          } else if (strncmp((char*)tmp->children->content,
                             SOAP_1_2_ENC_NAMESPACE,
                             sizeof(SOAP_1_2_ENC_NAMESPACE)) == 0) {
            binding->encodingStyle = SOAP_ENCODING_1_2;
          } else {
            throw SoapException("Parsing WSDL: Unknown encodingStyle '%s'",
                                tmp->children->content);
          }
        } else {
          throw SoapException("Parsing WSDL: Unspecified encodingStyle");
        }
      }
    } else if (node_is_equal_ex(trav, "header", wsdl_soap_namespace)) {
      sdlSoapBindingFunctionHeaderPtr h =
        wsdl_soap_binding_header(ctx, trav, wsdl_soap_namespace, false);
      string key;
      if (!h->ns.empty()) {
        key += h->ns;
        key += ':';
      }
      key += h->name;
      binding->headers[key] = h;
    } else if (is_wsdl_element(trav) && !node_is_equal(trav,"documentation")) {
      throw SoapException("Parsing WSDL: Unexpected WSDL element <%s>",
                          trav->name);
    }
    trav = trav->next;
  }
}

static void wsdl_message(sdlCtx *ctx, sdlParamVec &parameters,
                         xmlChar* message_name) {
  char *ctype = strrchr((char*)message_name,':');
  if (ctype == NULL) {
    ctype = (char*)message_name;
  } else {
    ++ctype;
  }
  xmlNodeMap::iterator iter = ctx->messages.find(ctype);
  if (iter == ctx->messages.end()) {
    throw SoapException("Parsing WSDL: Missing <message> with name '%s'",
                        message_name);
  }

  xmlNodePtr part;
  xmlNodePtr message = iter->second;
  xmlNodePtr trav = message->children;
  while (trav) {
    if (trav->ns && strcmp((char*)trav->ns->href, WSDL_NAMESPACE) != 0) {
      throw SoapException("Parsing WSDL: Unexpected extensibility "
                          "element <%s>", trav->name);
    }
    if (node_is_equal(trav,"documentation")) {
      trav = trav->next;
      continue;
    }
    if (!node_is_equal(trav,"part")) {
      throw SoapException("Parsing WSDL: Unexpected WSDL element <%s>",
                          trav->name);
    }
    part = trav;
    sdlParamPtr param(new sdlParam);
    param->order = 0;
    xmlAttrPtr name = get_attribute(part->properties, "name");
    if (name == NULL) {
      throw SoapException("Parsing WSDL: No name associated with <part> '%s'",
                          message->name);
    }

    param->paramName = (char*)name->children->content;
    xmlAttrPtr type = get_attribute(part->properties, "type");
    if (type) {
      param->encode = get_encoder_from_prefix(ctx->sdl.get(), part,
                                              type->children->content);
    } else {
      xmlAttrPtr element = get_attribute(part->properties, "element");
      if (element) {
        param->element = get_element(ctx->sdl.get(), part,
                                     element->children->content);
        if (param->element) {
          param->encode = param->element->encode;
        }
      }
    }

    parameters.push_back(param);
    trav = trav->next;
  }
}

sdlPtr load_wsdl(char *struri, HttpClient *http) {
  sdlCtx ctx;
  ctx.sdl = sdlPtr(new sdl());
  ctx.sdl->source = struri;

  load_wsdl_ex(struri, &ctx, false, http);
  schema_pass2(&ctx);

  int i = 0;
  for (xmlNodeMap::iterator iter = ctx.services.begin();
       iter != ctx.services.end(); ++iter, ++i) {
    xmlNodePtr service = iter->second;
    xmlNodePtr port;
    bool has_soap_port = false;
    xmlNodePtr trav = service->children;
    while (trav) {
      if (!is_wsdl_element(trav) || node_is_equal(trav,"documentation")) {
        trav = trav->next;
        continue;
      }
      if (!node_is_equal(trav,"port")) {
        throw SoapException("Parsing WSDL: Unexpected WSDL element <%s>",
                            trav->name);
      }

      port = trav;

      sdlBindingPtr tmpbinding(new sdlBinding());
      xmlAttrPtr bindingAttr = get_attribute(port->properties, "binding");
      if (bindingAttr == NULL) {
        throw SoapException("Parsing WSDL: No binding associated with <port>");
      }

      char *wsdl_soap_namespace = NULL;

        /* find address and figure out binding type */
      xmlNodePtr address = NULL;
      xmlNodePtr trav2 = port->children;
      while (trav2) {
        if (node_is_equal(trav2,"address") && trav2->ns) {
          if (!strncmp((char*)trav2->ns->href, WSDL_SOAP11_NAMESPACE,
                       sizeof(WSDL_SOAP11_NAMESPACE))) {
            address = trav2;
            wsdl_soap_namespace = WSDL_SOAP11_NAMESPACE;
            tmpbinding->bindingType = BINDING_SOAP;
          } else if (!strncmp((char*)trav2->ns->href, WSDL_SOAP12_NAMESPACE,
                              sizeof(WSDL_SOAP12_NAMESPACE))) {
            address = trav2;
            wsdl_soap_namespace = WSDL_SOAP12_NAMESPACE;
            tmpbinding->bindingType = BINDING_SOAP;
          } else if (!strncmp((char*)trav2->ns->href, RPC_SOAP12_NAMESPACE,
                              sizeof(RPC_SOAP12_NAMESPACE))) {
            address = trav2;
            wsdl_soap_namespace = RPC_SOAP12_NAMESPACE;
            tmpbinding->bindingType = BINDING_SOAP;
          } else if (!strncmp((char*)trav2->ns->href, WSDL_HTTP11_NAMESPACE,
                              sizeof(WSDL_HTTP11_NAMESPACE))) {
            address = trav2;
            tmpbinding->bindingType = BINDING_HTTP;
          } else if (!strncmp((char*)trav2->ns->href, WSDL_HTTP12_NAMESPACE,
                              sizeof(WSDL_HTTP12_NAMESPACE))) {
            address = trav2;
            tmpbinding->bindingType = BINDING_HTTP;
          }
        }
        if (trav2 != address && is_wsdl_element(trav2) &&
            !node_is_equal(trav2,"documentation")) {
          throw SoapException("Parsing WSDL: Unexpected WSDL element <%s>",
                              trav2->name);
        }
        trav2 = trav2->next;
      }
      if (!address || tmpbinding->bindingType == BINDING_HTTP) {
        if (has_soap_port || trav->next || i < (int)ctx.services.size() - 1) {
          trav = trav->next;
          continue;
        } else if (!address) {
          throw SoapException("Parsing WSDL: No address associated "
                              "with <port>");
        }
      }
      has_soap_port = true;

      xmlAttrPtr location = get_attribute(address->properties, "location");
      if (!location) {
        throw SoapException("Parsing WSDL: No location associated "
                            "with <port>");
      }
      tmpbinding->location = (char*)location->children->content;

      char *ctype = strrchr((char*)bindingAttr->children->content,':');
      if (ctype == NULL) {
        ctype = (char*)bindingAttr->children->content;
      } else {
        ++ctype;
      }
      xmlNodeMap::iterator iterBinding = ctx.bindings.find(ctype);
      if (iterBinding == ctx.bindings.end()) {
        throw SoapException("Parsing WSDL: No <binding> element "
                            "with name '%s'", ctype);
      }
      xmlNodePtr binding = iterBinding->second;

      if (tmpbinding->bindingType == BINDING_SOAP) {
        xmlAttrPtr tmp;

        sdlSoapBindingPtr soapBinding(new sdlSoapBinding());
        soapBinding->style = SOAP_DOCUMENT;
        xmlNodePtr soapBindingNode = get_node_ex(binding->children, "binding",
                                                 wsdl_soap_namespace);
        if (soapBindingNode) {
          tmp = get_attribute(soapBindingNode->properties, "style");
          if (tmp &&
              !strncmp((char*)tmp->children->content, "rpc", sizeof("rpc"))) {
            soapBinding->style = SOAP_RPC;
          }

          tmp = get_attribute(soapBindingNode->properties, "transport");
          if (tmp) {
            if (strncmp((char*)tmp->children->content, WSDL_HTTP_TRANSPORT,
                        sizeof(WSDL_HTTP_TRANSPORT)) == 0) {
              soapBinding->transport = SOAP_TRANSPORT_HTTP;
            } else {
              throw SoapException("Parsing WSDL: PHP-SOAP doesn't support "
                                  "transport '%s'", tmp->children->content);
            }
          }
        }
        tmpbinding->bindingAttributes = soapBinding;
      }

      xmlAttrPtr name = get_attribute(binding->properties, "name");
      if (name == NULL) {
        throw SoapException("Parsing WSDL: Missing 'name' attribute "
                            "for <binding>");
      }
      tmpbinding->name = (char*)name->children->content;

      xmlAttrPtr type = get_attribute(binding->properties, "type");
      if (type == NULL) {
        throw SoapException("Parsing WSDL: Missing 'type' attribute "
                            "for <binding>");
      }

      xmlNodePtr portType, operation;
      ctype = strrchr((char*)type->children->content,':');
      if (ctype == NULL) {
        ctype = (char*)type->children->content;
      } else {
        ++ctype;
      }
      xmlNodeMap::iterator iter = ctx.portTypes.find(ctype);
      if (iter == ctx.portTypes.end()) {
        throw SoapException("Parsing WSDL: Missing <portType> with name '%s'",
                            name->children->content);
      }
      portType = iter->second;

      trav2 = binding->children;
      while (trav2) {
        if ((tmpbinding->bindingType == BINDING_SOAP &&
             node_is_equal_ex(trav2, "binding", wsdl_soap_namespace)) ||
            !is_wsdl_element(trav2) ||
            node_is_equal(trav2,"documentation")) {
          trav2 = trav2->next;
          continue;
        }
        if (!node_is_equal(trav2,"operation")) {
          throw SoapException("Parsing WSDL: Unexpected WSDL element <%s>",
                              trav2->name);
        }

        operation = trav2;
        xmlAttrPtr op_name = get_attribute(operation->properties, "name");
        if (op_name == NULL) {
          throw SoapException("Parsing WSDL: Missing 'name' attribute "
                              "for <operation>");
        }

        xmlNodePtr trav3 = operation->children;
        while  (trav3) {
          if (tmpbinding->bindingType == BINDING_SOAP &&
              node_is_equal_ex(trav3, "operation", wsdl_soap_namespace)) {
          } else if (is_wsdl_element(trav3) &&
                     !node_is_equal(trav3,"input") &&
                     !node_is_equal(trav3,"output") &&
                     !node_is_equal(trav3,"fault") &&
                     !node_is_equal(trav3,"documentation")) {
            throw SoapException("Parsing WSDL: Unexpected WSDL element <%s>",
                                trav3->name);
          }
          trav3 = trav3->next;
        }

        xmlNodePtr portTypeOperation =
          get_node_with_attribute_ex(portType->children, "operation",
                                     WSDL_NAMESPACE, "name",
                                     (char*)op_name->children->content, NULL);
        if (portTypeOperation == NULL) {
          throw SoapException("Parsing WSDL: Missing <portType>/<operation> "
                              "with name '%s'", op_name->children->content);
        }

        xmlNodePtr input, output, fault;
        xmlAttrPtr paramOrder;

        sdlFunctionPtr function(new sdlFunction());
        function->functionName = (char*)op_name->children->content;

        if (tmpbinding->bindingType == BINDING_SOAP) {
          sdlSoapBindingFunctionPtr soapFunctionBinding
            (new sdlSoapBindingFunction());
          sdlSoapBindingPtr soapBinding = tmpbinding->bindingAttributes;
          soapFunctionBinding->style = soapBinding->style;

          xmlNodePtr soapOperation = get_node_ex
            (operation->children, "operation", wsdl_soap_namespace);
          xmlAttrPtr tmp;
          if (soapOperation) {
            tmp = get_attribute(soapOperation->properties, "soapAction");
            if (tmp) {
              soapFunctionBinding->soapAction = (char*)tmp->children->content;
            }

            tmp = get_attribute(soapOperation->properties, "style");
            if (tmp) {
              if (!strncmp((char*)tmp->children->content, "rpc",
                           sizeof("rpc"))) {
                soapFunctionBinding->style = SOAP_RPC;
              } else {
                soapFunctionBinding->style = SOAP_DOCUMENT;
              }
            } else {
              soapFunctionBinding->style = soapBinding->style;
            }
          }

          function->bindingAttributes = soapFunctionBinding;
        }

        input = get_node_ex(portTypeOperation->children, "input",
                            WSDL_NAMESPACE);
        if (input) {
          xmlAttrPtr message = get_attribute(input->properties, "message");
          if (message == NULL) {
            throw SoapException("Parsing WSDL: Missing name for <input> "
                                "of '%s'", op_name->children->content);
          }
          wsdl_message(&ctx, function->requestParameters,
                       message->children->content);
/* FIXME
          xmlAttrPtr name = get_attribute(input->properties, "name");
            if (name) {
              function->requestName = estrdup(name->children->content);
            } else {
*/
          {
            function->requestName = function->functionName;
          }

          if (tmpbinding->bindingType == BINDING_SOAP) {
            input = get_node_ex(operation->children, "input", WSDL_NAMESPACE);
            if (input) {
              sdlSoapBindingFunctionPtr soapFunctionBinding =
                function->bindingAttributes;
              wsdl_soap_binding_body(&ctx, input, wsdl_soap_namespace,
                                     &soapFunctionBinding->input,
                                     function->requestParameters);
            }
          }
        }

        output = get_node_ex(portTypeOperation->children, "output",
                             WSDL_NAMESPACE);
        if (output) {
          xmlAttrPtr message = get_attribute(output->properties, "message");
          if (message == NULL) {
            throw SoapException("Parsing WSDL: Missing name for <output> "
                                "of '%s'", op_name->children->content);
          }
          wsdl_message(&ctx, function->responseParameters,
                       message->children->content);

/* FIXME
            xmlAttrPtr name = get_attribute(output->properties, "name");
            if (name) {
              function->responseName = estrdup(name->children->content);
            } else if (input == NULL) {
              function->responseName = estrdup(function->functionName);
            } else {
*/
          {
            function->responseName = function->functionName + "Response";
          }

          if (tmpbinding->bindingType == BINDING_SOAP) {
            output = get_node_ex(operation->children, "output",
                                 WSDL_NAMESPACE);
            if (output) {
              sdlSoapBindingFunctionPtr soapFunctionBinding =
                function->bindingAttributes;
              wsdl_soap_binding_body(&ctx, output, wsdl_soap_namespace,
                                     &soapFunctionBinding->output,
                                     function->responseParameters);
            }
          }
        }

        paramOrder = get_attribute(portTypeOperation->properties,
                                   "parameterOrder");
        if (paramOrder) {
          /* FIXME: */
        }

        fault = portTypeOperation->children;
        while (fault) {
          if (node_is_equal_ex(fault, "fault", WSDL_NAMESPACE)) {
            xmlAttrPtr name = get_attribute(fault->properties, "name");
            if (name == NULL) {
              throw SoapException("Parsing WSDL: Missing name for <fault> "
                                  "of '%s'", op_name->children->content);
            }
            xmlAttrPtr message = get_attribute(fault->properties, "message");
            if (message == NULL) {
              throw SoapException("Parsing WSDL: Missing name for <output> "
                                  "of '%s'", op_name->children->content);
            }

            sdlFaultPtr f(new sdlFault());
            f->name = (char*)name->children->content;
            wsdl_message(&ctx, f->details, message->children->content);
            if (f->details.size() != 1) {
              throw SoapException("Parsing WSDL: The fault message '%s' must "
                                  "have a single part",
                                  message->children->content);
            }

            if (tmpbinding->bindingType == BINDING_SOAP) {
              xmlNodePtr soap_fault =
                get_node_with_attribute_ex(operation->children, "fault",
                                           WSDL_NAMESPACE, "name",
                                           (char*)f->name.c_str(), NULL);
              if (soap_fault) {
                xmlNodePtr trav = soap_fault->children;
                while (trav) {
                  if (node_is_equal_ex(trav, "fault", wsdl_soap_namespace)) {
                    sdlSoapBindingFunctionFaultPtr binding =
                      f->bindingAttributes = sdlSoapBindingFunctionFaultPtr
                      (new sdlSoapBindingFunctionFault());
                    xmlAttrPtr tmp = get_attribute(trav->properties, "use");
                    if (tmp && !strncmp((char*)tmp->children->content,
                                        "encoded", sizeof("encoded"))) {
                      binding->use = SOAP_ENCODED;
                    } else {
                      binding->use = SOAP_LITERAL;
                    }

                    tmp = get_attribute(trav->properties, "namespace");
                    if (tmp) {
                      binding->ns = (char*)tmp->children->content;
                    }

                    if (binding->use == SOAP_ENCODED) {
                      tmp = get_attribute(trav->properties, "encodingStyle");
                      if (tmp) {
                        if (strncmp((char*)tmp->children->content,
                                    SOAP_1_1_ENC_NAMESPACE,
                                    sizeof(SOAP_1_1_ENC_NAMESPACE)) == 0) {
                          binding->encodingStyle = SOAP_ENCODING_1_1;
                        } else if (strncmp((char*)tmp->children->content,
                                           SOAP_1_2_ENC_NAMESPACE,
                                           sizeof(SOAP_1_2_ENC_NAMESPACE))
                                   == 0) {
                          binding->encodingStyle = SOAP_ENCODING_1_2;
                        } else {
                          throw SoapException("Parsing WSDL: Unknown "
                                              "encodingStyle '%s'",
                                              tmp->children->content);
                        }
                      } else {
                        throw SoapException("Parsing WSDL: Unspecified "
                                            "encodingStyle");
                      }
                    }
                  } else if (is_wsdl_element(trav) &&
                             !node_is_equal(trav,"documentation")) {
                    throw SoapException("Parsing WSDL: Unexpected WSDL "
                                        "element <%s>", trav->name);
                  }
                  trav = trav->next;
                }
              }
            }
            sdlFaultMap::iterator iter = function->faults.find(f->name);
            if (iter != function->faults.end()) {
              throw SoapException("Parsing WSDL: <fault> with name '%s' "
                                  "already defined in '%s'", f->name.c_str(),
                                  op_name->children->content);
            }
            function->faults[f->name] = f;
          }
          fault = fault->next;
        }

        function->binding = tmpbinding;

        {
          string tmp = Util::toLower(function->functionName);
          sdlFunctionMap::iterator iter = ctx.sdl->functions.find(tmp);
          if (iter != ctx.sdl->functions.end()) {
            ctx.sdl->functions[boost::lexical_cast<string>
                               (ctx.sdl->functions.size())] = function;
          } else {
            ctx.sdl->functions[tmp] = function;
          }
          if (function->requestName != function->functionName) {
            ctx.sdl->requests[Util::toLower(function->requestName)] = function;
          }
        }
        trav2 = trav2->next;
      }

      ctx.sdl->bindings[tmpbinding->name] = tmpbinding;
      trav= trav->next;
    }
  }

  if (i == 0) {
    throw SoapException("Parsing WSDL: Couldn't bind to service");
  }
  return ctx.sdl;
}

///////////////////////////////////////////////////////////////////////////////
}
