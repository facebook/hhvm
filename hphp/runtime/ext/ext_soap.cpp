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

#include "hphp/runtime/ext/ext_soap.h"

#include <map>
#include <memory>

#include "folly/ScopeGuard.h"

#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/http-protocol.h"
#include "hphp/runtime/base/class-info.h"
#include "hphp/runtime/ext/soap/soap.h"
#include "hphp/runtime/ext/soap/packet.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/ext/zlib/ext_zlib.h"
#include "hphp/runtime/ext/ext_network.h"
#include "hphp/runtime/ext/ext_array.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/ext_class.h"
#include "hphp/runtime/ext/ext_output.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/ext/ext_string.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

const StaticString s___dorequest("__dorequest");

IMPLEMENT_DEFAULT_EXTENSION_VERSION(soap, NO_EXTENSION_VERSION_YET);

///////////////////////////////////////////////////////////////////////////////
// helper classes for setting/resetting globals within a method call

using std::string;

class SoapScope {
public:
  SoapScope() {
    USE_SOAP_GLOBAL;
    m_old_handler = SOAP_GLOBAL(use_soap_error_handler);
    m_old_error_code = SOAP_GLOBAL(error_code);
    m_old_error_object = SOAP_GLOBAL(error_object);
    m_old_soap_version = SOAP_GLOBAL(soap_version);

    SOAP_GLOBAL(use_soap_error_handler) = true;
  }

  ~SoapScope() {
    USE_SOAP_GLOBAL;
    SOAP_GLOBAL(use_soap_error_handler) = m_old_handler;
    SOAP_GLOBAL(error_code) = m_old_error_code;
    SOAP_GLOBAL(error_object) = m_old_error_object;
    SOAP_GLOBAL(soap_version) = m_old_soap_version;
  }

private:
  bool m_old_handler;
  const char *m_old_error_code;
  Object m_old_error_object;
  int m_old_soap_version;
};

class SoapServerScope : public SoapScope {
public:
  explicit SoapServerScope(c_SoapServer *server) {
    USE_SOAP_GLOBAL;
    SOAP_GLOBAL(error_code) = "Server";
    SOAP_GLOBAL(error_object) = Object(server);
  }
};

class SoapClientScope : public SoapScope {
public:
  explicit SoapClientScope(c_SoapClient *client) {
    USE_SOAP_GLOBAL;
    SOAP_GLOBAL(error_code) = "Client";
    SOAP_GLOBAL(error_object) = Object(client);
  }
};

class SoapServiceScope {
public:
  explicit SoapServiceScope(c_SoapServer *server) {
    save();
    USE_SOAP_GLOBAL;
    SOAP_GLOBAL(soap_version) = server->m_version;
    SOAP_GLOBAL(sdl) = server->m_sdl;
    SOAP_GLOBAL(encoding) = server->m_encoding;
    SOAP_GLOBAL(classmap) = server->m_classmap;
    SOAP_GLOBAL(typemap) = server->m_typemap;
    SOAP_GLOBAL(features) = server->m_features;
  }

  explicit SoapServiceScope(c_SoapClient *client) {
    save();
    USE_SOAP_GLOBAL;
    SOAP_GLOBAL(soap_version) = client->m_soap_version;
    SOAP_GLOBAL(sdl) = client->m_sdl;
    SOAP_GLOBAL(encoding) = client->m_encoding;
    SOAP_GLOBAL(classmap) = client->m_classmap;
    SOAP_GLOBAL(typemap) = client->m_typemap;
    SOAP_GLOBAL(features) = client->m_features;
  }

  ~SoapServiceScope() {
    USE_SOAP_GLOBAL;
    SOAP_GLOBAL(soap_version) = m_old_soap_version;
    SOAP_GLOBAL(encoding) = m_old_encoding;
    SOAP_GLOBAL(sdl) = m_old_sdl;
    SOAP_GLOBAL(classmap) = m_old_classmap;
    SOAP_GLOBAL(typemap) = m_old_typemap;
    SOAP_GLOBAL(features) = m_old_features;
  }

private:
  sdl *m_old_sdl;
  xmlCharEncodingHandlerPtr m_old_encoding;
  Array m_old_classmap;
  encodeMap *m_old_typemap;
  int64_t m_old_features;
  int m_old_soap_version;

  void save() {
    USE_SOAP_GLOBAL;
    m_old_soap_version = SOAP_GLOBAL(soap_version);
    m_old_sdl = SOAP_GLOBAL(sdl);
    m_old_encoding = SOAP_GLOBAL(encoding);
    m_old_classmap = SOAP_GLOBAL(classmap);
    m_old_typemap = SOAP_GLOBAL(typemap);
    m_old_features = SOAP_GLOBAL(features);
  }
};

///////////////////////////////////////////////////////////////////////////////
// forward declarations

static void throw_soap_server_fault(litstr code, litstr fault);
static void model_to_string(sdlContentModelPtr model, StringBuffer &buf,
                            int level);

///////////////////////////////////////////////////////////////////////////////
// client helpers

static Object create_soap_fault(const String& code, const String& fault) {
  return Object(SystemLib::AllocSoapFaultObject(code, fault));
}

static Object create_soap_fault(Exception &e) {
  USE_SOAP_GLOBAL;
  return create_soap_fault(SOAP_GLOBAL(error_code), String(e.getMessage()));
}

static sdlParamPtr get_param(sdlFunction *function, const char *param_name,
                             int index, bool response) {
  sdlParamVec *ht;

  if (!function) {
    return sdlParamPtr();
  }

  if (!response) {
    ht = &function->requestParameters;
  } else {
    ht = &function->responseParameters;
  }

  if (ht->empty()) {
    return sdlParamPtr();
  }

  if (param_name) {
    for (unsigned int i = 0; i < ht->size(); i++) {
      sdlParamPtr p = (*ht)[i];
      if (p->paramName == param_name) return p;
    }
  } else {
    if (index >= 0 && index < (int)ht->size()) {
      return (*ht)[index];
    }
  }
  return sdlParamPtr();
}

static xmlNodePtr serialize_zval(const Variant& val, sdlParamPtr param,
                                 const char *paramName, int style,
                                 xmlNodePtr parent) {
  xmlNodePtr xmlParam;
  encodePtr enc;

  Variant v = val;
  if (param != NULL) {
    enc = param->encode;
    if (val.isNull()) {
      if (param->element) {
        if (!param->element->fixed.empty()) {
          v = String(param->element->fixed);
        } else if (!param->element->def.empty() && !param->element->nillable) {
          v = String(param->element->def);
        }
      }
    }
  } else {
    enc = encodePtr();
  }
  xmlParam = master_to_xml(enc, val, style, parent);
  if (!strcmp((char*)xmlParam->name, "BOGUS")) {
    xmlNodeSetName(xmlParam, BAD_CAST(paramName));
  }
  return xmlParam;
}

static xmlNodePtr serialize_parameter(sdlParamPtr param, Variant value,
                                      int index, const char *name, int style,
                                      xmlNodePtr parent) {
  if (!value.isNull() && value.isObject()) {
    c_SoapParam *p = value.toObject().getTyped<c_SoapParam>(true, true);
    if (p) {
      value = p->m_data;
      name = p->m_name.c_str();
    }
  }

  if (param && !param->paramName.empty()) {
    name = param->paramName.c_str();
  } else {
    if (name == NULL) {
      char paramNameBuf[10];
      snprintf(paramNameBuf, sizeof(paramNameBuf), "param%d", index);
      name = paramNameBuf;
    }
  }

  return serialize_zval(value, param, name, style, parent);
}

static xmlDocPtr serialize_function_call
(c_SoapClient *client, std::shared_ptr<sdlFunction> function,
  const char *function_name,
  const char *uri, const Array& arguments, const Array& soap_headers) {
  xmlNodePtr envelope = NULL, body, method = NULL, head = NULL;
  xmlNsPtr ns = NULL;
  int style, use;
  sdlSoapBindingFunctionHeaderMap *hdrs = NULL;

  encode_reset_ns();

  xmlDoc *doc = xmlNewDoc(BAD_CAST("1.0"));
  doc->encoding = xmlCharStrdup("UTF-8");
  doc->charset = XML_CHAR_ENCODING_UTF8;
  if (client->m_soap_version == SOAP_1_1) {
    envelope = xmlNewDocNode(doc, NULL, BAD_CAST("Envelope"), NULL);
    ns = xmlNewNs(envelope, BAD_CAST(SOAP_1_1_ENV_NAMESPACE),
                  BAD_CAST(SOAP_1_1_ENV_NS_PREFIX));
    xmlSetNs(envelope, ns);
  } else if (client->m_soap_version == SOAP_1_2) {
    envelope = xmlNewDocNode(doc, NULL, BAD_CAST("Envelope"), NULL);
    ns = xmlNewNs(envelope, BAD_CAST(SOAP_1_2_ENV_NAMESPACE),
                  BAD_CAST(SOAP_1_2_ENV_NS_PREFIX));
    xmlSetNs(envelope, ns);
  } else {
    throw SoapException("Unknown SOAP version");
  }
  xmlDocSetRootElement(doc, envelope);

  if (!soap_headers.empty()) {
    head = xmlNewChild(envelope, ns, BAD_CAST("Header"), NULL);
  }

  body = xmlNewChild(envelope, ns, BAD_CAST("Body"), NULL);

  if (function && function->binding->bindingType == BINDING_SOAP) {
    sdlSoapBindingFunctionPtr fnb = function->bindingAttributes;

    hdrs = &fnb->input.headers;
    style = fnb->style;
    /*FIXME: how to pass method name if style is SOAP_DOCUMENT */
    /*style = SOAP_RPC;*/
    use = fnb->input.use;
    if (style == SOAP_RPC) {
      ns = encode_add_ns(body, fnb->input.ns.c_str());
      if (!function->requestName.empty()) {
        method = xmlNewChild(body, ns,
                             BAD_CAST(function->requestName.c_str()), NULL);
      } else {
        method = xmlNewChild(body, ns,
                             BAD_CAST(function->functionName.c_str()), NULL);
      }
    }
  } else {
    use = client->m_use;
    style = client->m_style;
    /*FIXME: how to pass method name if style is SOAP_DOCUMENT */
    /*style = SOAP_RPC;*/
    if (style == SOAP_RPC) {
      ns = encode_add_ns(body, uri);
      if (function_name) {
        method = xmlNewChild(body, ns, BAD_CAST(function_name), NULL);
      } else if (function && !function->requestName.empty()) {
        method = xmlNewChild(body, ns,
                             BAD_CAST(function->requestName.c_str()), NULL);
      } else if (function && !function->functionName.empty()) {
        method = xmlNewChild(body, ns,
                             BAD_CAST(function->functionName.c_str()), NULL);
      } else {
        method = body;
      }
    } else {
      method = body;
    }
  }

  int i = 0;
  for (ArrayIter iter(arguments); iter; ++iter, ++i) {
    xmlNodePtr param;
    sdlParamPtr parameter;
    if (function) {
      parameter = get_param(function.get(), NULL, i, false);
    }

    if (style == SOAP_RPC) {
      if (parameter) {
        param = serialize_parameter(parameter, iter.second(), i, NULL,
                                    use, method);
      }
    } else if (style == SOAP_DOCUMENT) {
      param = serialize_parameter(parameter, iter.second(), i, NULL,
                                  use, body);
      if (function && function->binding->bindingType == BINDING_SOAP) {
        if (parameter && parameter->element) {
          ns = encode_add_ns(param, parameter->element->namens.c_str());
          xmlNodeSetName(param, BAD_CAST(parameter->element->name.c_str()));
          xmlSetNs(param, ns);
        }
      }
    }
  }

  if (function && !function->requestParameters.empty()) {
    int n = function->requestParameters.size();
    if (n > arguments.size()) {
      for (i = arguments.size(); i < n; i++) {
        xmlNodePtr param;
        sdlParamPtr parameter = get_param(function.get(), NULL, i, false);

        if (style == SOAP_RPC) {
          param = serialize_parameter(parameter, uninit_null(), i, NULL, use, method);
        } else if (style == SOAP_DOCUMENT) {
          param = serialize_parameter(parameter, uninit_null(), i, NULL, use, body);
          if (function && function->binding->bindingType == BINDING_SOAP) {
            if (parameter && parameter->element) {
              ns = encode_add_ns(param, parameter->element->namens.c_str());
              xmlNodeSetName(param,
                             BAD_CAST(parameter->element->name.c_str()));
              xmlSetNs(param, ns);
            }
          }
        }
      }
    }
  }

  if (head) {
    for (ArrayIter iter(soap_headers); iter; ++iter) {
      c_SoapHeader *header = iter.second().toObject().getTyped<c_SoapHeader>();

      xmlNodePtr h;
      xmlNsPtr nsptr;
      int hdr_use = SOAP_LITERAL;
      encodePtr enc;

      if (hdrs) {
        std::string key = header->m_namespace.data();
        key += ':';
        key += header->m_name.data();
        sdlSoapBindingFunctionHeaderMap::iterator iter = hdrs->find(key);
        if (iter != hdrs->end()) {
          auto hdr = iter->second;
          hdr_use = hdr->use;
          enc = hdr->encode;
          if (hdr_use == SOAP_ENCODED) {
            use = SOAP_ENCODED;
          }
        }
      }

      if (!header->m_data.isNull()) {
        h = master_to_xml(enc, header->m_data, hdr_use, head);
        xmlNodeSetName(h, BAD_CAST(header->m_name.data()));
      } else {
        h = xmlNewNode(NULL, BAD_CAST(header->m_name.data()));
        xmlAddChild(head, h);
      }
      nsptr = encode_add_ns(h, header->m_namespace.data());
      xmlSetNs(h, nsptr);

      if (header->m_mustUnderstand) {
        if (client->m_soap_version == SOAP_1_1) {
          xmlSetProp(h, BAD_CAST(SOAP_1_1_ENV_NS_PREFIX":mustUnderstand"),
                     BAD_CAST("1"));
        } else {
          xmlSetProp(h, BAD_CAST(SOAP_1_2_ENV_NS_PREFIX":mustUnderstand"),
                     BAD_CAST("true"));
        }
      }
      if (!header->m_actor.isNull()) {
        if (header->m_actor.isString()) {
          if (client->m_soap_version == SOAP_1_1) {
            xmlSetProp(h, BAD_CAST(SOAP_1_1_ENV_NS_PREFIX":actor"),
                       BAD_CAST(header->m_actor.toString().data()));
          } else {
            xmlSetProp(h, BAD_CAST(SOAP_1_2_ENV_NS_PREFIX":role"),
                       BAD_CAST(header->m_actor.toString().data()));
          }
        } else if (header->m_actor.isInteger()) {
          int64_t actor = header->m_actor.toInt64();
          if (client->m_soap_version == SOAP_1_1) {
            if (actor == SOAP_ACTOR_NEXT) {
              xmlSetProp(h, BAD_CAST(SOAP_1_1_ENV_NS_PREFIX":actor"),
                         BAD_CAST(SOAP_1_1_ACTOR_NEXT));
            }
          } else {
            if (actor == SOAP_ACTOR_NEXT) {
              xmlSetProp(h, BAD_CAST(SOAP_1_2_ENV_NS_PREFIX":role"),
                         BAD_CAST(SOAP_1_2_ACTOR_NEXT));
            } else if (actor == SOAP_ACTOR_NONE) {
              xmlSetProp(h, BAD_CAST(SOAP_1_2_ENV_NS_PREFIX":role"),
                         BAD_CAST(SOAP_1_2_ACTOR_NONE));
            } else if (actor == SOAP_ACTOR_UNLIMATERECEIVER) {
              xmlSetProp(h, BAD_CAST(SOAP_1_2_ENV_NS_PREFIX":role"),
                         BAD_CAST(SOAP_1_2_ACTOR_UNLIMATERECEIVER));
            }
          }
        }
      }
    }
  }

  if (use == SOAP_ENCODED) {
    xmlNewNs(envelope, BAD_CAST(XSD_NAMESPACE), BAD_CAST(XSD_NS_PREFIX));
    if (client->m_soap_version == SOAP_1_1) {
      xmlNewNs(envelope, BAD_CAST(SOAP_1_1_ENC_NAMESPACE),
               BAD_CAST(SOAP_1_1_ENC_NS_PREFIX));
      xmlSetNsProp(envelope, envelope->ns, BAD_CAST("encodingStyle"),
                   BAD_CAST(SOAP_1_1_ENC_NAMESPACE));
    } else if (client->m_soap_version == SOAP_1_2) {
      xmlNewNs(envelope, BAD_CAST(SOAP_1_2_ENC_NAMESPACE),
               BAD_CAST(SOAP_1_2_ENC_NS_PREFIX));
      if (method) {
        xmlSetNsProp(method, envelope->ns, BAD_CAST("encodingStyle"),
                     BAD_CAST(SOAP_1_2_ENC_NAMESPACE));
      }
    }
  }

  encode_finish();
  return doc;
}

static bool do_request(c_SoapClient *client, xmlDoc *request,
                       const char *location, const char *action, int version,
                       bool one_way, Variant &response) {
  char *buf; int buf_size;
  xmlDocDumpMemory(request, (xmlChar**)&buf, &buf_size);
  if (!buf) {
    client->m_soap_fault =
      create_soap_fault("HTTP", "Error build soap request");
    return false;
  }

  if (client->m_trace) {
    client->m_last_request = String((char*)buf, buf_size, CopyString);
  }
  response = client->o_invoke_few_args(s___dorequest, 5,
      String(buf, buf_size, CopyString),
      String(location, CopyString),
      String(action, CopyString),
      version, one_way);
  if (!response.isString()) {
    if (client->m_soap_fault.isNull()) {
      client->m_soap_fault =
        create_soap_fault("Client", "SoapClient::__doRequest() "
                          "returned non string value");
    }
  } else if (client->m_trace) {
    client->m_last_response = response;
  }
  xmlFree(buf);

  return client->m_soap_fault.isNull();
}

static void verify_soap_headers_array(Array &headers) {
  for (ArrayIter iter(headers); iter; ++iter) {
    Variant tmp = iter.second();
    if (!tmp.isObject() || !tmp.toObject().is<c_SoapHeader>()) {
      throw SoapException("Invalid SOAP header");
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
// shared helpers

const StaticString
  s_type_name("type_name"),
  s_type_ns("type_ns"),
  s_to_xml("to_xml"),
  s_from_xml("from_xml");

static encodeMapPtr soap_create_typemap_impl(sdl *sdl, Array &ht) {
  encodeMapPtr typemap(new encodeMap());
  for (ArrayIter iter(ht); iter; ++iter) {
    Variant tmp = iter.second();
    if (!tmp.isArray()) {
      raise_warning("Wrong 'typemap' option");
      return typemap;
    }
    Array ht2 = tmp.toArray();

    String type_name, type_ns;
    Variant to_xml, to_zval;
    for (ArrayIter it(ht2); it; ++it) {
      tmp = it.second();
      if (it.first().isString()) {
        String name = it.first().toString();
        if (name == s_type_name) {
          if (tmp.isString()) type_name = tmp.toString();
        } else if (name == s_type_ns) {
          if (tmp.isString()) type_ns = tmp.toString();
        } else if (name == s_to_xml) {
          to_xml = tmp;
        } else if (name == s_from_xml) {
          to_zval = tmp;
        }
      }
    }

    encodePtr enc, new_enc;
    if (!type_name.empty()) {
      if (!type_ns.empty()) {
        enc = get_encoder(sdl, type_ns.data(), type_name.data());
      } else {
        enc = get_encoder_ex(sdl, type_name.data());
      }

      new_enc = encodePtr(new encode());
      if (enc) {
        new_enc->details.type = enc->details.type;
        new_enc->details.ns = enc->details.ns;
        new_enc->details.type_str = enc->details.type_str;
        new_enc->details.sdl_type = enc->details.sdl_type;
      } else {
        enc = get_conversion(UNKNOWN_TYPE);
        new_enc->details.type = enc->details.type;
        if (!type_ns.empty()) {
          new_enc->details.ns = type_ns.data();
        }
        new_enc->details.type_str = type_name.data();
      }
      new_enc->to_xml = enc->to_xml;
      new_enc->to_zval = enc->to_zval;
      new_enc->details.map = std::make_shared<soapMapping>();
      if (to_xml.toBoolean()) {
        new_enc->details.map->to_xml = to_xml;
        new_enc->to_xml = to_xml_user;
      } else if (enc->details.map && !enc->details.map->to_xml.isNull()) {
        new_enc->details.map->to_xml = enc->details.map->to_xml;
      }
      if (to_zval.toBoolean()) {
        new_enc->details.map->to_zval = to_zval;
        new_enc->to_zval = to_zval_user;
      } else if (enc->details.map && !enc->details.map->to_zval.isNull()) {
        new_enc->details.map->to_zval = enc->details.map->to_zval;
      }

      string nscat;
      if (!type_ns.empty()) {
        nscat += type_ns.data();
        nscat += ':';
      }
      nscat += type_name.data();
      (*typemap)[nscat] = new_enc;
    }
  }
  return typemap;
}

static encodeMap *soap_create_typemap(sdl *sdl, Array &ht) {
  return s_soap_data->register_typemap(soap_create_typemap_impl(sdl, ht));
}

static void output_xml_header(int soap_version) {
  if (soap_version == SOAP_1_2) {
    f_header("Content-Type: application/soap+xml; charset=utf-8");
  } else {
    f_header("Content-Type: text/xml; charset=utf-8");
  }
}

///////////////////////////////////////////////////////////////////////////////
// server helpers

static void deserialize_parameters(xmlNodePtr params, sdlFunction *function,
                                   Array &parameters) {
  int num_of_params = 0;
  int cur_param = 0;
  if (function) {
    bool use_names = false;
    num_of_params = function->requestParameters.size();
    if (num_of_params == 0) return;
    for (int i = 0; i < num_of_params; i++) {
      sdlParamPtr param = function->requestParameters[i];
      if (get_node(params, (char*)param->paramName.c_str())) {
        use_names = true;
        break;
      }
    }
    if (use_names) {
      for (int i = 0; i < num_of_params; i++, cur_param++) {
        sdlParamPtr param = function->requestParameters[i];
        xmlNodePtr val = get_node(params, (char*)param->paramName.c_str());
        if (val) {
          parameters.append(master_to_zval(param->encode, val));
        }
      }
      return;
    }
  }
  if (params) {
    int num_of_params = 0;
    xmlNodePtr trav = params;
    while (trav != NULL) {
      if (trav->type == XML_ELEMENT_NODE) {
        num_of_params++;
      }
      trav = trav->next;
    }

    if (num_of_params == 1 && function && function->binding &&
        function->binding->bindingType == BINDING_SOAP &&
        function->bindingAttributes->style == SOAP_DOCUMENT &&
        function->requestParameters.empty() &&
        strcmp((char*)params->name, function->functionName.c_str()) == 0) {
      num_of_params = 0;
    } else if (num_of_params > 0) {
      trav = params;
      while (trav != 0 && cur_param < num_of_params) {
        if (trav->type == XML_ELEMENT_NODE) {
          encodePtr enc;
          sdlParamPtr param;
          if (function) {
            if (cur_param >= (int)function->requestParameters.size()) {
              throw_soap_server_fault("Client","Error cannot find parameter");
            }
            param = function->requestParameters[cur_param];
          }
          if (param == NULL) {
            enc.reset();
          } else {
            enc = param->encode;
          }
          parameters.set(cur_param, master_to_zval(enc, trav));
          cur_param++;
        }
        trav = trav->next;
      }
    }
  }
  if (num_of_params > cur_param) {
    throw_soap_server_fault("Client","Missing parameter");
  }
}

static std::shared_ptr<sdlFunction>
get_doc_function(sdl *sdl, xmlNodePtr params) {
  if (sdl) {
    for (sdlFunctionMap::iterator iter = sdl->functions.begin();
         iter != sdl->functions.end(); ++iter) {
      auto tmp = iter->second;
      if (tmp->binding && tmp->binding->bindingType == BINDING_SOAP) {
        sdlSoapBindingFunctionPtr fnb = tmp->bindingAttributes;
        if (fnb->style == SOAP_DOCUMENT) {
          if (params == NULL) {
            if (tmp->requestParameters.empty()) {
              return tmp;
            }
          } else if (!tmp->requestParameters.empty()) {
            bool ok = true;
            xmlNodePtr node = params;
            for (unsigned int i = 0; i < tmp->requestParameters.size(); i++) {
              sdlParamPtr param = tmp->requestParameters[i];
              if (param->element) {
                if (param->element->name != (char*)node->name) {
                  ok = false;
                  break;
                }
                if (!param->element->namens.empty() && node->ns) {
                  if (param->element->namens != (char*)node->ns->href) {
                    ok = false;
                    break;
                  }
                } else if ((param->element->namens.empty() && node->ns) ||
                           (!param->element->namens.empty() &&
                            node->ns == NULL)) {
                  ok = false;
                  break;
                }
              } else if (param->paramName != (char*)node->name) {
                ok = false;
                break;
              }
              node = node->next;
            }
            if (ok /*&& node == NULL*/) {
              return tmp;
            }
          }
        }
      }
    }
  }
  return std::shared_ptr<sdlFunction>();
}

static std::shared_ptr<sdlFunction>
get_function(sdl *sdl, const char *function_name) {
  if (sdl) {
    String lowered = f_strtolower(function_name);
    sdlFunctionMap::iterator iter = sdl->functions.find(lowered.data());
    if (iter == sdl->functions.end()) {
      iter = sdl->requests.find(lowered.data());
      if (iter == sdl->requests.end()) {
        return std::shared_ptr<sdlFunction>();
      }
    }
    return iter->second;
  }
  return std::shared_ptr<sdlFunction>();
}

static std::shared_ptr<sdlFunction>
find_function(sdl *sdl, xmlNodePtr func, String &function_name) {
  auto function = get_function(sdl, (char*)func->name);
  if (function && function->binding &&
      function->binding->bindingType == BINDING_SOAP) {
    sdlSoapBindingFunctionPtr fnb = function->bindingAttributes;
    if (fnb->style == SOAP_DOCUMENT) {
      if (func->children != NULL || !function->requestParameters.empty()) {
        function.reset();
      }
    }
  }
  if (sdl != NULL && function == NULL) {
    function = get_doc_function(sdl, func);
  }

  if (function != NULL) {
    function_name = String(function->functionName);
  } else {
    function_name = String((char *)func->name, CopyString);
  }

  return function;
}

static std::shared_ptr<sdlFunction> deserialize_function_call
(sdl *sdl, xmlDocPtr request, const char* actor, String &function_name,
 Array &parameters, int &version, Array &headers) {
  USE_SOAP_GLOBAL;
  char* envelope_ns = NULL;
  xmlNodePtr trav,env,head,body,func;
  xmlAttrPtr attr;
  std::shared_ptr<sdlFunction> function;

  encode_reset_ns();

  /* Get <Envelope> element */
  env = NULL;
  trav = request->children;
  while (trav != NULL) {
    if (trav->type == XML_ELEMENT_NODE) {
      if (env == NULL &&
          node_is_equal_ex(trav,"Envelope",SOAP_1_1_ENV_NAMESPACE)) {
        env = trav;
        version = SOAP_1_1;
        envelope_ns = SOAP_1_1_ENV_NAMESPACE;
        SOAP_GLOBAL(soap_version) = SOAP_1_1;
      } else if (env == NULL &&
                 node_is_equal_ex(trav,"Envelope",SOAP_1_2_ENV_NAMESPACE)) {
        env = trav;
        version = SOAP_1_2;
        envelope_ns = SOAP_1_2_ENV_NAMESPACE;
        SOAP_GLOBAL(soap_version) = SOAP_1_2;
      } else {
        throw_soap_server_fault("VersionMismatch", "Wrong Version");
      }
    }
    trav = trav->next;
  }
  if (env == NULL) {
    throw_soap_server_fault
      ("Client", "looks like we got XML without \"Envelope\" element");
  }

  attr = env->properties;
  while (attr != NULL) {
    if (attr->ns == NULL) {
      throw_soap_server_fault("Client", "A SOAP Envelope element cannot have "
                              "non Namespace qualified attributes");
    } else if (attr_is_equal_ex(attr,"encodingStyle",SOAP_1_2_ENV_NAMESPACE)) {
      if (version == SOAP_1_2) {
        throw_soap_server_fault("Client", "encodingStyle cannot be specified "
                                "on the Envelope");
      } else if (strcmp((char*)attr->children->content,
                        SOAP_1_1_ENC_NAMESPACE) != 0) {
        throw_soap_server_fault("Client", "Unknown data encoding style");
      }
    }
    attr = attr->next;
  }

  /* Get <Header> element */
  head = NULL;
  trav = env->children;
  while (trav != NULL && trav->type != XML_ELEMENT_NODE) {
    trav = trav->next;
  }
  if (trav != NULL && node_is_equal_ex(trav,"Header",envelope_ns)) {
    head = trav;
    trav = trav->next;
  }

  /* Get <Body> element */
  body = NULL;
  while (trav != NULL && trav->type != XML_ELEMENT_NODE) {
    trav = trav->next;
  }
  if (trav != NULL && node_is_equal_ex(trav,"Body",envelope_ns)) {
    body = trav;
    trav = trav->next;
  }
  while (trav != NULL && trav->type != XML_ELEMENT_NODE) {
    trav = trav->next;
  }
  if (body == NULL) {
    throw_soap_server_fault("Client", "Body must be present in a "
                            "SOAP envelope");
  }
  attr = body->properties;
  while (attr != NULL) {
    if (attr->ns == NULL) {
      if (version == SOAP_1_2) {
        throw_soap_server_fault("Client", "A SOAP Body element cannot have non"
                                " Namespace qualified attributes");
      }
    } else if (attr_is_equal_ex(attr,"encodingStyle",SOAP_1_2_ENV_NAMESPACE)) {
      if (version == SOAP_1_2) {
        throw_soap_server_fault("Client", "encodingStyle cannot be specified "
                                "on the Body");
      } else if (strcmp((char*)attr->children->content,
                        SOAP_1_1_ENC_NAMESPACE) != 0) {
        throw_soap_server_fault("Client", "Unknown data encoding style");
      }
    }
    attr = attr->next;
  }

  if (trav != NULL && version == SOAP_1_2) {
    throw_soap_server_fault("Client", "A SOAP 1.2 envelope can contain only "
                            "Header and Body");
  }

  func = NULL;
  trav = body->children;
  while (trav != NULL) {
    if (trav->type == XML_ELEMENT_NODE) {
/*
      if (func != NULL) {
        throw_soap_server_fault("Client", "looks like we got \"Body\" with "
        "several functions call", NULL, NULL, NULL);
      }
*/
      func = trav;
      break; /* FIXME: the rest of body is ignored */
    }
    trav = trav->next;
  }
  if (func == NULL) {
    function = get_doc_function(sdl, NULL);
    if (function != NULL) {
      function_name = String(function->functionName);
    } else {
      throw_soap_server_fault
        ("Client", "looks like we got \"Body\" without function call");
    }
  } else {
    if (version == SOAP_1_1) {
      attr = get_attribute_ex(func->properties,"encodingStyle",
                              SOAP_1_1_ENV_NAMESPACE);
      if (attr && strcmp((char*)attr->children->content,
                         SOAP_1_1_ENC_NAMESPACE) != 0) {
        throw_soap_server_fault("Client","Unknown Data Encoding Style");
      }
    } else {
      attr = get_attribute_ex(func->properties,"encodingStyle",
                              SOAP_1_2_ENV_NAMESPACE);
      if (attr && strcmp((char*)attr->children->content,
                         SOAP_1_2_ENC_NAMESPACE) != 0) {
        throw_soap_server_fault
          ("DataEncodingUnknown","Unknown Data Encoding Style");
      }
    }
    function = find_function(sdl, func, function_name);
    if (sdl != NULL && function == NULL) {
      if (version == SOAP_1_2) {
        throw_soap_server_fault
          ("rpc:ProcedureNotPresent","Procedure not present");
      } else {
        throw SoapException("Procedure '%s' not present", func->name);
      }
    }
  }

  headers = Array::Create();
  if (head) {
    soapHeader *h = NULL;
    attr = head->properties;
    while (attr != NULL) {
      if (attr->ns == NULL) {
        throw_soap_server_fault("Client", "A SOAP Header element cannot have "
                                "non Namespace qualified attributes");
      } else if (attr_is_equal_ex(attr,"encodingStyle",
                                  SOAP_1_2_ENV_NAMESPACE)) {
        if (version == SOAP_1_2) {
          throw_soap_server_fault("Client", "encodingStyle cannot be specified"
                                  " on the Header");
        } else if (strcmp((char*)attr->children->content,
                          SOAP_1_1_ENC_NAMESPACE) != 0) {
          throw_soap_server_fault("Client", "Unknown data encoding style");
        }
      }
      attr = attr->next;
    }
    trav = head->children;
    while (trav != NULL) {
      if (trav->type == XML_ELEMENT_NODE) {
        xmlNodePtr hdr_func = trav;
        xmlAttrPtr attr;
        int mustUnderstand = 0;

        if (version == SOAP_1_1) {
          attr = get_attribute_ex(hdr_func->properties,"encodingStyle",
                                  SOAP_1_1_ENV_NAMESPACE);
          if (attr && strcmp((char*)attr->children->content,
                             SOAP_1_1_ENC_NAMESPACE) != 0) {
            throw_soap_server_fault("Client","Unknown Data Encoding Style");
          }
          attr = get_attribute_ex(hdr_func->properties,"actor",envelope_ns);
          if (attr != NULL) {
            if (strcmp((char*)attr->children->content,
                       SOAP_1_1_ACTOR_NEXT) != 0 &&
                (actor == NULL ||
                 strcmp((char*)attr->children->content,actor) != 0)) {
              goto ignore_header;
            }
          }
        } else if (version == SOAP_1_2) {
          attr = get_attribute_ex(hdr_func->properties,"encodingStyle",
                                  SOAP_1_2_ENV_NAMESPACE);
          if (attr && strcmp((char*)attr->children->content,
                             SOAP_1_2_ENC_NAMESPACE) != 0) {
            throw_soap_server_fault
              ("DataEncodingUnknown","Unknown Data Encoding Style");
          }
          attr = get_attribute_ex(hdr_func->properties,"role",envelope_ns);
          if (attr != NULL) {
            if (strcmp((char*)attr->children->content,
                       SOAP_1_2_ACTOR_UNLIMATERECEIVER) != 0 &&
                strcmp((char*)attr->children->content,
                       SOAP_1_2_ACTOR_NEXT) != 0 &&
                (actor == NULL ||
                 strcmp((char*)attr->children->content,actor) != 0)) {
              goto ignore_header;
            }
          }
        }
        attr = get_attribute_ex(hdr_func->properties,"mustUnderstand",
                                envelope_ns);
        if (attr) {
          if (strcmp((char*)attr->children->content,"1") == 0 ||
              strcmp((char*)attr->children->content,"true") == 0) {
            mustUnderstand = 1;
          } else if (strcmp((char*)attr->children->content,"0") == 0 ||
                     strcmp((char*)attr->children->content,"false") == 0) {
            mustUnderstand = 0;
          } else {
            throw_soap_server_fault("Client",
                                    "mustUnderstand value is not boolean");
          }
        }
        h = NEWOBJ(soapHeader)();
        Resource hobj(h);
        h->function = find_function(sdl, hdr_func, h->function_name).get();
        h->mustUnderstand = mustUnderstand;
        h->hdr = NULL;
        if (!h->function && sdl && function && function->binding &&
            function->binding->bindingType == BINDING_SOAP) {
          sdlSoapBindingFunctionPtr fnb = function->bindingAttributes;
          if (!fnb->input.headers.empty()) {
            string key;
            if (hdr_func->ns) {
              key += (char*)hdr_func->ns->href;
              key += ':';
            }
            key += (std::string)h->function_name;
            sdlSoapBindingFunctionHeaderMap::iterator iter =
              fnb->input.headers.find(key);
            if (iter != fnb->input.headers.end()) {
              h->hdr = iter->second.get();
            }
          }
        }
        if (h->hdr) {
          h->parameters.append(master_to_zval(h->hdr->encode, hdr_func));
        } else {
          if (h->function && h->function->binding &&
              h->function->binding->bindingType == BINDING_SOAP) {
            sdlSoapBindingFunctionPtr fnb = h->function->bindingAttributes;
            if (fnb->style == SOAP_RPC) {
              hdr_func = hdr_func->children;
            }
          }
          deserialize_parameters(hdr_func, h->function, h->parameters);
        }
        headers.append(hobj);
      }
ignore_header:
      trav = trav->next;
    }
  }

  if (function && function->binding &&
      function->binding->bindingType == BINDING_SOAP) {
    sdlSoapBindingFunctionPtr fnb = function->bindingAttributes;
    if (fnb->style == SOAP_RPC) {
      func = func->children;
    }
  } else {
    func = func->children;
  }
  deserialize_parameters(func, function.get(), parameters);

  encode_finish();

  return function;
}

static int serialize_response_call2(xmlNodePtr body, sdlFunction *function,
                                    const char *function_name, const char *uri,
                                    Variant &ret, int version, int main) {
  xmlNodePtr method = NULL, param;
  sdlParamPtr parameter;
  int param_count;
  int style, use;
  xmlNsPtr ns = NULL;

  if (function && function->binding->bindingType == BINDING_SOAP) {
    sdlSoapBindingFunctionPtr fnb = function->bindingAttributes;
    style = fnb->style;
    use = fnb->output.use;
      ns = encode_add_ns(body, fnb->output.ns.c_str());
    if (style == SOAP_RPC) {
      if (!function->responseName.empty()) {
        method = xmlNewChild(body, ns,
                             BAD_CAST(function->responseName.c_str()), NULL);
      } else if (!function->responseParameters.empty()) {
        method = xmlNewChild(body, ns,
                             BAD_CAST(function->functionName.c_str()), NULL);
      }
    }
  } else {
    style = main?SOAP_RPC:SOAP_DOCUMENT;
    use = main?SOAP_ENCODED:SOAP_LITERAL;
    if (style == SOAP_RPC) {
      ns = encode_add_ns(body, uri);
      method = xmlNewChild(body, ns, BAD_CAST(function_name), NULL);
    }
  }

  if (function) {
    param_count = function->responseParameters.size();
  } else {
    param_count = 1;
  }

  if (param_count == 1) {
    parameter = get_param(function, NULL, 0, true);

    if (style == SOAP_RPC) {
      xmlNode *rpc_result;
      if (main && version == SOAP_1_2) {
        xmlNs *rpc_ns = xmlNewNs(body, BAD_CAST(RPC_SOAP12_NAMESPACE),
                                 BAD_CAST(RPC_SOAP12_NS_PREFIX));
        rpc_result = xmlNewChild(method, rpc_ns, BAD_CAST("result"), NULL);
        param = serialize_parameter(parameter, ret, 0, "return", use, method);
        xmlNodeSetContent(rpc_result,param->name);
      } else {
        param = serialize_parameter(parameter, ret, 0, "return", use, method);
      }
    } else {
      param = serialize_parameter(parameter, ret, 0, "return", use, body);
      if (function && function->binding->bindingType == BINDING_SOAP) {
        if (parameter && parameter->element) {
          ns = encode_add_ns(param, parameter->element->namens.c_str());
          xmlNodeSetName(param, BAD_CAST(parameter->element->name.c_str()));
          xmlSetNs(param, ns);
        }
      } else if (strcmp((char*)param->name,"return") == 0) {
        ns = encode_add_ns(param, uri);
        xmlNodeSetName(param, BAD_CAST(function_name));
        xmlSetNs(param, ns);
      }
    }
  } else if (param_count > 1 && ret.isArray()) {
    Array arr = ret.toArray();
    int i = 0;
    for (ArrayIter iter(arr); iter; ++iter, ++i) {
      Variant data = iter.second();
      Variant key = iter.first();
      String param_name;
      int64_t param_index = i;
      if (key.isString()) {
        param_name = key.toString();
      } else {
        param_index = key.toInt64();
      }

      parameter = get_param(function, param_name.c_str(), param_index, true);
      if (style == SOAP_RPC) {
        param = serialize_parameter(parameter, data, i, param_name.data(),
                                    use, method);
      } else {
        param = serialize_parameter(parameter, data, i, param_name.data(),
                                    use, body);
        if (function && function->binding->bindingType == BINDING_SOAP) {
          if (parameter && parameter->element) {
            ns = encode_add_ns(param, parameter->element->namens.c_str());
            xmlNodeSetName(param, BAD_CAST(parameter->element->name.c_str()));
            xmlSetNs(param, ns);
          }
        }
      }
    }
  }
  if (use == SOAP_ENCODED && version == SOAP_1_2 && method) {
    xmlSetNsProp(method, body->ns, BAD_CAST("encodingStyle"),
                 BAD_CAST(SOAP_1_2_ENC_NAMESPACE));
  }
  return use;
}

static xmlDocPtr serialize_response_call(
    std::shared_ptr<sdlFunction> function,
    const char *function_name,
    const char *uri, Variant &ret,
    const Array& headers, int version) {
  xmlNodePtr envelope = NULL, body, param;
  xmlNsPtr ns = NULL;
  int use = SOAP_LITERAL;
  xmlNodePtr head = NULL;

  encode_reset_ns();

  xmlDocPtr doc = xmlNewDoc(BAD_CAST("1.0"));
  doc->charset = XML_CHAR_ENCODING_UTF8;
  doc->encoding = xmlCharStrdup("UTF-8");

  if (version == SOAP_1_1) {
    envelope = xmlNewDocNode(doc, NULL, BAD_CAST("Envelope"), NULL);
    ns = xmlNewNs(envelope, BAD_CAST(SOAP_1_1_ENV_NAMESPACE),
                  BAD_CAST(SOAP_1_1_ENV_NS_PREFIX));
    xmlSetNs(envelope,ns);
  } else if (version == SOAP_1_2) {
    envelope = xmlNewDocNode(doc, NULL, BAD_CAST("Envelope"), NULL);
    ns = xmlNewNs(envelope, BAD_CAST(SOAP_1_2_ENV_NAMESPACE),
                  BAD_CAST(SOAP_1_2_ENV_NS_PREFIX));
    xmlSetNs(envelope,ns);
  } else {
    throw_soap_server_fault("Server", "Unknown SOAP version");
  }
  xmlDocSetRootElement(doc, envelope);

  if (ret.isObject() &&
      ret.toObject()->instanceof(SystemLib::s_SoapFaultClass)) {
    ObjectData* obj = ret.getObjectData();

    char *detail_name;
    std::shared_ptr<sdlFault> fault;
    string fault_ns;

    if (!headers.empty()) {
      xmlNodePtr head;
      encodePtr hdr_enc;
      int hdr_use = SOAP_LITERAL;
      Variant hdr_ret = obj->o_get("headerfault");
      soapHeader *h = headers[0].toResource().getTyped<soapHeader>();
      const char *hdr_ns   = h->hdr ? h->hdr->ns.c_str() : NULL;
      const char *hdr_name = h->function_name.data();

      head = xmlNewChild(envelope, ns, BAD_CAST("Header"), NULL);
      if (hdr_ret.isObject() && hdr_ret.toObject().is<c_SoapHeader>()) {
        c_SoapHeader *ht = hdr_ret.toObject().getTyped<c_SoapHeader>();

        string key;
        if (!ht->m_namespace.empty()) {
          key += ht->m_namespace.data();
          key += ':';
          hdr_ns = ht->m_namespace.data();
        }
        if (!ht->m_name.empty()) {
          key += ht->m_name.data();
          hdr_name = ht->m_name.data();
        }
        if (h->hdr) {
          sdlSoapBindingFunctionHeaderMap::iterator iter =
            h->hdr->headerfaults.find(key);
          if (iter != h->hdr->headerfaults.end()) {
            auto hdr = iter->second;
            hdr_enc = hdr->encode;
            hdr_use = hdr->use;
          }
        }
        hdr_ret = ht->m_data;
        obj->o_set("headerfault", hdr_ret);
      }

      if (h->function) {
        if (serialize_response_call2(head, h->function,
                                     h->function_name.data(), uri,
                                     hdr_ret, version, 0) == SOAP_ENCODED) {
          obj->o_set("headerfault", hdr_ret);
          use = SOAP_ENCODED;
        }
      } else {
        xmlNodePtr xmlHdr = master_to_xml(hdr_enc, hdr_ret, hdr_use, head);
        if (hdr_name) {
          xmlNodeSetName(xmlHdr, BAD_CAST(hdr_name));
        }
        if (hdr_ns) {
          xmlNsPtr nsptr = encode_add_ns(xmlHdr, hdr_ns);
          xmlSetNs(xmlHdr, nsptr);
        }
      }
    }

    body = xmlNewChild(envelope, ns, BAD_CAST("Body"), NULL);
    param = xmlNewChild(body, ns, BAD_CAST("Fault"), NULL);

    fault_ns = obj->o_get("faultcodens").toString().data();
    use = SOAP_LITERAL;
    if (!obj->o_get("_name").toString().empty()) {
      if (function) {
        sdlFaultMap::iterator iter =
          function->faults.find(obj->o_get("_name").toString().data());
        if (iter != function->faults.end()) {
          fault = iter->second;
          if (function->binding &&
              function->binding->bindingType == BINDING_SOAP &&
              fault->bindingAttributes) {
            sdlSoapBindingFunctionFaultPtr fb = fault->bindingAttributes;
            use = fb->use;
            if (fault_ns.empty()) {
              fault_ns = fb->ns;
            }
          }
        }
      }
    } else if (function && function->faults.size() == 1) {
      fault = function->faults[0];
      if (function->binding &&
          function->binding->bindingType == BINDING_SOAP &&
          fault->bindingAttributes) {
        sdlSoapBindingFunctionFaultPtr fb = fault->bindingAttributes;
        use = fb->use;
        if (fault_ns.empty()) {
          fault_ns = fb->ns;
        }
      }
    }

    if (fault_ns.empty() && fault && fault->details.size() == 1) {
      sdlParamPtr sparam = fault->details[0];
      if (sparam->element) {
        fault_ns = sparam->element->namens;
      }
    }

    if (version == SOAP_1_1) {
      if (!obj->o_get("faultcode").toString().empty()) {
        xmlNodePtr node = xmlNewNode(NULL, BAD_CAST("faultcode"));
        String str = StringUtil::HtmlEncode(obj->o_get("faultcode"),
                                            StringUtil::QuoteStyle::Double,
                                            "UTF-8", true, true);
        xmlAddChild(param, node);
        if (!fault_ns.empty()) {
          xmlNsPtr nsptr = encode_add_ns(node, fault_ns.c_str());
          xmlChar *code = xmlBuildQName(BAD_CAST(str.data()), nsptr->prefix,
                                        NULL, 0);
          xmlNodeSetContent(node, code);
          xmlFree(code);
        } else {
          xmlNodeSetContentLen(node, BAD_CAST(str.data()), str.size());
        }
      }
      if (!obj->o_get("faultstring").toString().empty()) {
        xmlNodePtr node = master_to_xml(get_conversion(KindOfString),
                                        obj->o_get("faultstring"), SOAP_LITERAL,
                                        param);
        xmlNodeSetName(node, BAD_CAST("faultstring"));
      }
      if (!obj->o_get("faultactor").toString().empty()) {
        xmlNodePtr node = master_to_xml(get_conversion(KindOfString),
                                        obj->o_get("faultactor"), SOAP_LITERAL,
                                        param);
        xmlNodeSetName(node, BAD_CAST("faultactor"));
      }
      detail_name = "detail";
    } else {
      if (!obj->o_get("faultcode").toString().empty()) {
        xmlNodePtr node = xmlNewChild(param, ns, BAD_CAST("Code"), NULL);
        String str = StringUtil::HtmlEncode(obj->o_get("faultcode"),
                                            StringUtil::QuoteStyle::Double,
                                            "UTF-8", true, true);
        node = xmlNewChild(node, ns, BAD_CAST("Value"), NULL);
        if (!fault_ns.empty()) {
          xmlNsPtr nsptr = encode_add_ns(node, fault_ns.c_str());
          xmlChar *code = xmlBuildQName(BAD_CAST(str.data()), nsptr->prefix,
                                        NULL, 0);
          xmlNodeSetContent(node, code);
          xmlFree(code);
        } else {
          xmlNodeSetContentLen(node, BAD_CAST(str.data()), str.size());
        }
      }
      if (!obj->o_get("faultstring").toString().empty()) {
        xmlNodePtr node = xmlNewChild(param, ns, BAD_CAST("Reason"), NULL);
        node = master_to_xml(get_conversion(KindOfString), obj->o_get("faultstring"),
                             SOAP_LITERAL, node);
        xmlNodeSetName(node, BAD_CAST("Text"));
        xmlSetNs(node, ns);
      }
      detail_name = SOAP_1_2_ENV_NS_PREFIX":Detail";
    }
    if (fault && fault->details.size() == 1) {
      xmlNodePtr node;
      Variant detail;
      sdlParamPtr sparam;
      xmlNodePtr x;

      if (!obj->o_get("detail").isNull()) {
        detail = obj->o_get("detail");
      }
      node = xmlNewNode(NULL, BAD_CAST(detail_name));
      xmlAddChild(param, node);

      sparam = fault->details[0];

      if (detail.isObject() && sparam->element) {
        Variant prop = detail.toObject()->o_get(sparam->element->name.c_str());
        if (!prop.isNull()) {
          detail = prop;
        }
      }

      x = serialize_parameter(sparam, detail, 1, NULL, use, node);

      if (function &&
          function->binding &&
          function->binding->bindingType == BINDING_SOAP &&
          function->bindingAttributes) {
        sdlSoapBindingFunctionPtr fnb = function->bindingAttributes;
        if (fnb->style == SOAP_RPC && !sparam->element) {
          if (fault->bindingAttributes) {
            sdlSoapBindingFunctionFaultPtr fb = fault->bindingAttributes;
            if (!fb->ns.empty()) {
              xmlNsPtr ns = encode_add_ns(x, fb->ns.c_str());
              xmlSetNs(x, ns);
            }
          }
        } else {
          if (sparam->element) {
            xmlNsPtr ns = encode_add_ns(x, sparam->element->namens.c_str());
            xmlNodeSetName(x, BAD_CAST(sparam->element->name.c_str()));
            xmlSetNs(x, ns);
          }
        }
      }
      if (use == SOAP_ENCODED && version == SOAP_1_2) {
        xmlSetNsProp(x, envelope->ns, BAD_CAST("encodingStyle"),
                     BAD_CAST(SOAP_1_2_ENC_NAMESPACE));
      }
    } else if (!obj->o_get("detail").isNull()) {
      serialize_zval(obj->o_get("detail"), sdlParamPtr(), detail_name, use, param);
    }
  } else {

    if (!headers.empty()) {
      head = xmlNewChild(envelope, ns, BAD_CAST("Header"), NULL);
      for (ArrayIter iter(headers); iter; ++iter) {
        soapHeader *h = iter.second().toResource().getTyped<soapHeader>();
        if (!h->retval.isNull()) {
          encodePtr hdr_enc;
          int hdr_use = SOAP_LITERAL;
          Variant &hdr_ret = h->retval;
          const char *hdr_ns   = h->hdr ? h->hdr->ns.c_str() : NULL;
          const char *hdr_name = h->function_name.data();

          if (h->retval.isObject() &&
              h->retval.toObject().is<c_SoapHeader>()) {
            c_SoapHeader *ht = h->retval.toObject().getTyped<c_SoapHeader>();
            string key;
            if (!ht->m_namespace.empty()) {
              key += ht->m_namespace.data();
              key += ':';
              hdr_ns = ht->m_namespace.data();
            }
            if (!ht->m_name.empty()) {
              key += ht->m_name.data();
              hdr_name = ht->m_name.data();
            }
            if (function && function->binding &&
                function->binding->bindingType == BINDING_SOAP) {
              sdlSoapBindingFunctionPtr fnb = function->bindingAttributes;
              sdlSoapBindingFunctionHeaderMap::iterator iter =
                fnb->output.headers.find(key);
              if (iter != fnb->output.headers.end()) {
                hdr_enc = iter->second->encode;
                hdr_use = iter->second->use;
              }
            }
            hdr_ret = ht->m_data;
          }

          if (h->function) {
            if (serialize_response_call2(head, h->function,
                                         h->function_name.data(), uri, hdr_ret,
                                         version, 0) == SOAP_ENCODED) {
              use = SOAP_ENCODED;
            }
          } else {
            xmlNodePtr xmlHdr = master_to_xml(hdr_enc, hdr_ret, hdr_use, head);
            if (hdr_name) {
              xmlNodeSetName(xmlHdr, BAD_CAST(hdr_name));
            }
            if (hdr_ns) {
              xmlNsPtr nsptr = encode_add_ns(xmlHdr,hdr_ns);
              xmlSetNs(xmlHdr, nsptr);
            }
          }
        }
      }

      if (head->children == NULL) {
        xmlUnlinkNode(head);
        xmlFreeNode(head);
      }
    }

    body = xmlNewChild(envelope, ns, BAD_CAST("Body"), NULL);

    if (serialize_response_call2(body, function.get(), function_name, uri, ret,
                                 version, 1) == SOAP_ENCODED) {
      use = SOAP_ENCODED;
    }
  }

  if (use == SOAP_ENCODED) {
    xmlNewNs(envelope, BAD_CAST(XSD_NAMESPACE), BAD_CAST(XSD_NS_PREFIX));
    if (version == SOAP_1_1) {
      xmlNewNs(envelope, BAD_CAST(SOAP_1_1_ENC_NAMESPACE),
               BAD_CAST(SOAP_1_1_ENC_NS_PREFIX));
      xmlSetNsProp(envelope, envelope->ns, BAD_CAST("encodingStyle"),
                   BAD_CAST(SOAP_1_1_ENC_NAMESPACE));
    } else if (version == SOAP_1_2) {
      xmlNewNs(envelope, BAD_CAST(SOAP_1_2_ENC_NAMESPACE),
               BAD_CAST(SOAP_1_2_ENC_NS_PREFIX));
    }
  }

  encode_finish();

  if (function && function->responseName.empty() &&
      body->children == NULL && head == NULL) {
    xmlFreeDoc(doc);
    return NULL;
  }
  return doc;
}

static void function_to_string(std::shared_ptr<sdlFunction> function,
                               StringBuffer &sb) {
  sdlParamPtr param;
  sdlParamVec &res = function->responseParameters;
  if (!res.empty()) {
    if (res.size() == 1) {
      param = res[0];
      if (param->encode && !param->encode->details.type_str.empty()) {
        sb.append(param->encode->details.type_str);
        sb.append(' ');
      } else {
        sb.append("UNKNOWN ");
      }
    } else {
      sb.append("list(");
      bool started = false;
      for (unsigned int i = 0; i < res.size(); i++) {
        param = res[i];
        if (started) {
          sb.append(", ");
        } else {
          started = true;
        }
        if (param->encode && !param->encode->details.type_str.empty()) {
          sb.append(param->encode->details.type_str);
        } else {
          sb.append("UNKNOWN");
        }
        sb.append(" $");
        sb.append(param->paramName);
      }
      sb.append(") ");
    }
  } else {
    sb.append("void ");
  }

  sb.append(function->functionName);

  sb.append('(');
  sdlParamVec &req = function->requestParameters;
  bool started = false;
  for (unsigned int i = 0; i < req.size(); i++) {
    param = req[i];
    if (started) {
      sb.append(", ");
    } else {
      started = true;
    }

    if (param->encode && !param->encode->details.type_str.empty()) {
      sb.append(param->encode->details.type_str);
    } else {
      sb.append("UNKNOWN");
    }
    sb.append(" $");
    sb.append(param->paramName);
  }
  sb.append(')');
}

static void type_to_string(sdlType *type, StringBuffer &buf, int level) {
  StringBuffer sbspaces;
  for (int i = 0; i < level;i++) {
    sbspaces.append(' ');
  }
  String spaces = sbspaces.detach();
  buf.append(spaces);

  switch (type->kind) {
  case XSD_TYPEKIND_SIMPLE:
    if (type->encode) {
      buf.append(type->encode->details.type_str);
      buf.append(' ');
    } else {
      buf.append("anyType ");
    }
    buf.append(type->name);
    break;
  case XSD_TYPEKIND_LIST:
    buf.append("list ");
    buf.append(type->name);
    if (!type->elements.empty()) {
      buf.append(" {");
      buf.append(type->elements[0]->name);
      buf.append('}');
    }
    break;
  case XSD_TYPEKIND_UNION:
    buf.append("union ");
    buf.append(type->name);
    if (!type->elements.empty()) {
      bool first = true;
      buf.append(" {");
      for (unsigned int i = 0; i < type->elements.size(); i++) {
        if (!first) {
          buf.append(',');
          first = false;
        }
        buf.append(type->elements[i]->name);
      }
      buf.append('}');
    }
    break;
  case XSD_TYPEKIND_COMPLEX:
  case XSD_TYPEKIND_RESTRICTION:
  case XSD_TYPEKIND_EXTENSION:
    if (type->encode &&
        (type->encode->details.type == KindOfArray ||
         type->encode->details.type == SOAP_ENC_ARRAY)) {
      sdlAttributeMap::iterator iter;
      sdlExtraAttributeMap::iterator iterExtra;
      if (!type->attributes.empty() &&
          ((iter = type->attributes.find(SOAP_1_1_ENC_NAMESPACE":arrayType"))
           != type->attributes.end())) {
        if ((iterExtra = iter->second->extraAttributes.find
             (WSDL_NAMESPACE":arrayType"))
            != iter->second->extraAttributes.end()) {
          auto ext = iterExtra->second;
          const char *end = strchr(ext->val.c_str(), '[');
          int len;
          if (end == NULL) {
            len = ext->val.size();
          } else {
            len = end- ext->val.c_str();
          }
          if (len == 0) {
            buf.append("anyType");
          } else {
            buf.append(ext->val.c_str(), len);
          }
          buf.append(' ');
          buf.append(type->name);
          if (end) {
            buf.append(end);
          }
        }
      } else {
        sdlTypePtr elementType;
        sdlAttributeMap::iterator iter;
        sdlExtraAttributeMap::iterator iterExtra;
        if (!type->attributes.empty() &&
            ((iter = type->attributes.find(SOAP_1_2_ENC_NAMESPACE":itemType"))
             != type->attributes.end())) {
          if ((iterExtra = iter->second->extraAttributes.find
               (WSDL_NAMESPACE":itemType"))
              != iter->second->extraAttributes.end()) {
            auto ext = iterExtra->second;
            buf.append(ext->val);
            buf.append(' ');
          }
        } else if (type->elements.size() == 1 &&
                   (elementType = type->elements[0]) != NULL &&
                   elementType->encode &&
                   !elementType->encode->details.type_str.empty()) {
          buf.append(elementType->encode->details.type_str);
          buf.append(' ');
        } else {
          buf.append("anyType ");
        }
        buf.append(type->name);
        if (!type->attributes.empty() &&
            ((iter = type->attributes.find(SOAP_1_2_ENC_NAMESPACE":arraySize"))
             != type->attributes.end())) {
          if ((iterExtra = iter->second->extraAttributes.find
               (WSDL_NAMESPACE":itemType"))
              != iter->second->extraAttributes.end()) {
            auto ext = iterExtra->second;
            buf.append('[');
            buf.append(ext->val);
            buf.append(']');
          }
        } else {
          buf.append("[]");
        }
      }
    } else {
      buf.append("struct ");
      buf.append(type->name);
      buf.append(' ');
      buf.append("{\n");
      if ((type->kind == XSD_TYPEKIND_RESTRICTION ||
           type->kind == XSD_TYPEKIND_EXTENSION) && type->encode) {
        encodePtr enc = type->encode;
        while (enc && enc->details.sdl_type &&
               enc != enc->details.sdl_type->encode &&
               enc->details.sdl_type->kind != XSD_TYPEKIND_SIMPLE &&
               enc->details.sdl_type->kind != XSD_TYPEKIND_LIST &&
               enc->details.sdl_type->kind != XSD_TYPEKIND_UNION) {
          enc = enc->details.sdl_type->encode;
        }
        if (enc) {
          buf.append(spaces);
          buf.append(' ');
          buf.append(type->encode->details.type_str);
          buf.append(" _;\n");
        }
      }
      if (type->model) {
        model_to_string(type->model, buf, level+1);
      }
      if (!type->attributes.empty()) {
        for (sdlAttributeMap::iterator iter = type->attributes.begin();
             iter != type->attributes.end(); ++iter) {
          sdlAttributePtr attr = iter->second;
          buf.append(spaces);
          buf.append(' ');
          if (attr->encode && !attr->encode->details.type_str.empty()) {
            buf.append(attr->encode->details.type_str);
            buf.append(' ');
          } else {
            buf.append("UNKNOWN ");
          }
          buf.append(attr->name);
          buf.append(";\n");
        }
      }
      buf.append(spaces);
      buf.append('}');
    }
    break;
  default:
    break;
  }
}

static void model_to_string(sdlContentModelPtr model, StringBuffer &buf,
                            int level) {
  int i;
  switch (model->kind) {
  case XSD_CONTENT_ELEMENT:
    type_to_string(model->u_element, buf, level);
    buf.append(";\n");
    break;
  case XSD_CONTENT_ANY:
    for (i = 0;i < level;i++) {
      buf.append(' ');
    }
    buf.append("<anyXML> any;\n");
    break;
  case XSD_CONTENT_SEQUENCE:
  case XSD_CONTENT_ALL:
  case XSD_CONTENT_CHOICE: {
    for (unsigned int i = 0; i < model->u_content.size(); i++) {
      sdlContentModelPtr tmp = model->u_content[i];
      model_to_string(tmp, buf, level);
    }
    break;
  }
  case XSD_CONTENT_GROUP:
    model_to_string(model->u_group->model, buf, level);
  default:
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// soap fault functions

const StaticString
  s_HTTP_USER_AGENT("HTTP_USER_AGENT"),
  s__SERVER("_SERVER"),
  s_Shockwave_Flash("Shockwave Flash");

static void send_soap_server_fault(
    std::shared_ptr<sdlFunction> function,
    Variant fault,
    soapHeader *hdr) {
  USE_SOAP_GLOBAL;
  bool use_http_error_status = true;
  GlobalVariables *g = get_global_variables();
  if (g->get(s__SERVER).toArray()[s_HTTP_USER_AGENT].toString() ==
      s_Shockwave_Flash) {
    use_http_error_status = false;
  }
  if (use_http_error_status) {
    f_header("HTTP/1.1 500 Internal Service Error");
  }
  output_xml_header(SOAP_GLOBAL(soap_version));

  Array headers;
  if (hdr) headers.append(Resource(hdr));
  xmlDocPtr doc_return = serialize_response_call
    (function, NULL, NULL, fault, headers, SOAP_GLOBAL(soap_version));

  f_ob_end_clean(); // dump all buffered output

  xmlChar *buf; int size;
  xmlDocDumpMemory(doc_return, &buf, &size);
  if (buf) {
    echo(String((const char *)buf, size, CopyString));
    xmlFree(buf);
  }
  xmlFreeDoc(doc_return);
}

static void send_soap_server_fault(
    std::shared_ptr<sdlFunction> function,
    Exception &e,
    soapHeader *hdr) {
  USE_SOAP_GLOBAL;
  if (SOAP_GLOBAL(use_soap_error_handler)) {
    send_soap_server_fault(
      std::shared_ptr<sdlFunction>(), create_soap_fault(e), NULL);
  } else {
    throw create_soap_fault(e); // assuming we are in "catch"
  }
}

static void throw_soap_server_fault(litstr code, litstr fault) {
  send_soap_server_fault(
    std::shared_ptr<sdlFunction>(), create_soap_fault(code, fault),
                         NULL);
  throw ExitException(1);
}

bool f_use_soap_error_handler(bool handler /* = true */) {
  USE_SOAP_GLOBAL;
  bool old = SOAP_GLOBAL(use_soap_error_handler);
  SOAP_GLOBAL(use_soap_error_handler) = handler;
  return old;
}

bool f_is_soap_fault(const Variant& fault) {
  return fault.isObject() &&
    fault.getObjectData()->instanceof(SystemLib::s_SoapFaultClass);
}

int64_t f__soap_active_version() {
  USE_SOAP_GLOBAL;
  return SOAP_GLOBAL(soap_version);
}

///////////////////////////////////////////////////////////////////////////////
// class SoapServer

c_SoapServer::c_SoapServer(Class* cb) :
    ExtObjectData(cb),
    m_type(SOAP_FUNCTIONS),
    m_version(SOAP_1_1),
    m_sdl(NULL),
    m_encoding(NULL),
    m_typemap(NULL),
    m_features(0),
    m_send_errors(1) {
}

c_SoapServer::~c_SoapServer() {
}

const StaticString
  s_soap_version("soap_version"),
  s_uri("uri"),
  s_actor("actor"),
  s_encoding("encoding"),
  s_classmap("classmap"),
  s_typemap("typemap"),
  s_features("features"),
  s_cache_wsdl("cache_wsdl"),
  s_send_errors("send_errors"),
  s_location("location"),
  s_style("style"),
  s_use("use"),
  s_stream_context("stream_context"),
  s_login("login"),
  s_password("password"),
  s_authentication("authentication"),
  s_proxy_host("proxy_host"),
  s_proxy_port("proxy_port"),
  s_proxy_login("proxy_login"),
  s_proxy_password("proxy_password"),
  s_trace("trace"),
  s_exceptions("exceptions"),
  s_compression("compression"),
  s_connection_timeout("connection_timeout"),
  s_user_agent("user_agent"),
  s_soapaction("soapaction");

void c_SoapServer::t___construct(const Variant& wsdl,
                                 const Array& options /* = null_array */) {
  USE_SOAP_GLOBAL;
  SoapServerScope ss(this);

  try {

  if (!wsdl.isString() && !wsdl.isNull()) {
    throw SoapException("Invalid parameters");
  }

  m_send_errors = 1;

  int64_t cache_wsdl = SOAP_GLOBAL(cache);

  int version = SOAP_1_1;
  Array typemap_ht;
  if (!options.empty()) {
    if (options[s_soap_version].isInteger()) {
      int64_t tmp = options[s_soap_version].toInt64();
      if (tmp == SOAP_1_1 || tmp == SOAP_1_2) {
        version = tmp;
      }
    }

    if (options[s_uri].isString()) {
      m_uri = options[s_uri].toString();
    } else if (wsdl.isNull()) {
      throw SoapException("'uri' option is required in nonWSDL mode");
    }

    if (options[s_actor].isString()) {
      m_actor = options[s_actor].toString();
    }

    if (options[s_encoding].isString()) {
      String tmp = options[s_encoding].toString();
      m_encoding = xmlFindCharEncodingHandler(tmp.data());
      if (m_encoding == NULL) {
        throw SoapException("Invalid 'encoding' option - '%s'", tmp.data());
      }
      s_soap_data->register_encoding(m_encoding);
    }

    if (options[s_classmap].isArray()) {
      m_classmap = options[s_classmap].toArray();
    }

    if (options[s_typemap].isArray()) {
      typemap_ht = options[s_typemap].toArray();
    }

    if (options[s_features].isInteger()) {
      m_features = options[s_features].toInt64();
    }

    if (options[s_cache_wsdl].isInteger()) {
      cache_wsdl = options[s_cache_wsdl].toInt64();
    }

    if (options[s_send_errors].isInteger() ||
        options[s_send_errors].is(KindOfBoolean)) {
      m_send_errors = options[s_send_errors].toInt64();
    }

  } else if (wsdl.isNull()) {
    throw SoapException("'uri' option is required in nonWSDL mode");
  }

  m_version = version;
  m_type = SOAP_FUNCTIONS;
  m_soap_functions.functions_all = false;

  if (!wsdl.isNull()) {
    m_sdl = s_soap_data->get_sdl(wsdl.toString().data(), cache_wsdl);
    if (m_uri.isNull()) {
      if (!m_sdl->target_ns.empty()) {
        m_uri = String(m_sdl->target_ns);
      } else {
        /*FIXME*/
        m_uri = String("http://unknown-uri/");
      }
    }
  }

  if (!typemap_ht.empty()) {
    m_typemap = soap_create_typemap(m_sdl, typemap_ht);
  }

  } catch (Exception &e) {
    throw create_soap_fault(e);
  }
}

void c_SoapServer::t_setclass(int _argc, const String& name,
                              const Array& _argv /* = null_array */) {
  SoapServerScope ss(this);
  if (f_class_exists(name, true)) {
    m_type = SOAP_CLASS;
    m_soap_class.name = name;
    m_soap_class.argv = _argv;
    m_soap_class.persistance = SOAP_PERSISTENCE_REQUEST;
  } else {
    raise_warning("Tried to set a non existant class (%s)", name.data());
  }
}

void c_SoapServer::t_setobject(const Object& obj) {
  SoapServerScope ss(this);
  m_type = SOAP_OBJECT;
  m_soap_object = obj;
}

void c_SoapServer::t_addfunction(const Variant& func) {
  SoapServerScope ss(this);

  Array funcs;
  if (func.isString()) {
    funcs.append(func);
  } else if (func.isArray()) {
    funcs = func.toArray();
  } else if (func.isInteger()) {
    if (func.toInt64() == SOAP_FUNCTIONS_ALL) {
      m_soap_functions.ft.clear();
      m_soap_functions.ftOriginal.clear();
      m_soap_functions.functions_all = true;
    } else {
      raise_warning("Invalid value passed");
    }
    return;
  }

  if (m_type == SOAP_FUNCTIONS) {
    for (ArrayIter iter(funcs); iter; ++iter) {
      if (!iter.second().isString()) {
        raise_warning("Tried to add a function that isn't a string");
        return;
      }
      String function_name = iter.second().toString();
      if (!f_function_exists(function_name)) {
        raise_warning("Tried to add a non existant function '%s'",
                        function_name.data());
        return;
      }
      m_soap_functions.ft.set(f_strtolower(function_name), 1);
      m_soap_functions.ftOriginal.set(function_name, 1);
    }
  }
}

Variant c_SoapServer::t_getfunctions() {
  SoapServerScope ss(this);

  String class_name;
  if (m_type == SOAP_OBJECT) {
    class_name = m_soap_object->o_getClassName();
  } else if (m_type == SOAP_CLASS) {
    class_name = m_soap_class.name;
  } else if (m_soap_functions.functions_all) {
    return Unit::getSystemFunctions() + Unit::getUserFunctions();
  } else if (!m_soap_functions.ft.empty()) {
    return f_array_keys(m_soap_functions.ftOriginal);
  }

  ClassInfo::MethodVec methods;
  ClassInfo::GetClassMethods(methods, class_name.data());
  Array ret = Array::Create();
  for (unsigned int i = 0; i < methods.size(); i++) {
    ClassInfo::MethodInfo *info = methods[i];
    if (info->attribute & ClassInfo::IsPublic) {
      ret.append(info->name);
    }
  }
  return ret;
}

static bool valid_function(c_SoapServer *server, Object &soap_obj,
                           const String& fn_name) {
  HPHP::Class* cls = nullptr;
  if (server->m_type == SOAP_OBJECT || server->m_type == SOAP_CLASS) {
    cls = server->m_soap_object->getVMClass();
  } else if (server->m_soap_functions.functions_all) {
    return f_function_exists(fn_name);
  } else if (!server->m_soap_functions.ft.empty()) {
    return server->m_soap_functions.ft.exists(f_strtolower(fn_name));
  }
  HPHP::Func* f = cls ? cls->lookupMethod(fn_name.get()) : nullptr;
  return (f && f->isPublic());
}

const StaticString
  s_HTTP_CONTENT_ENCODING("HTTP_CONTENT_ENCODING"),
  s_gzip("gzip"),
  s_xgzip("x-gzip"),
  s_deflate("deflate");

void c_SoapServer::t_handle(const String& request /* = null_string */) {
  USE_SOAP_GLOBAL;
  SoapServerScope ss(this);

  SOAP_GLOBAL(soap_version) = m_version;

  // 0. serving WSDL
  Transport *transport = g_context->getTransport();
  if (transport && transport->getMethod() == Transport::Method::GET &&
      transport->getCommand() == "wsdl") {
    if (!m_sdl) {
      throw_soap_server_fault("Server", "WSDL generation is not supported");
    }

    f_header("Content-Type: text/xml; charset=utf-8");
    Variant ret = f_readfile(m_sdl->source.c_str());
    if (same(ret, false)) {
      throw_soap_server_fault("Server", "Couldn't find WSDL");
    }
    return;
  }

  if (!f_ob_start()) {
    throw SoapException("ob_start failed");
  }

  // 1. process request
  String req;
  if (!request.isNull()) {
    req = request;
  } else {
    int size;
    const char *data = NULL;
    if (transport) {
      data = (const char *)transport->getPostData(size);
    }
    if (!data || !*data || !size) {
      return;
    }
    req = String(data, size, CopyString);

    GlobalVariables *g = get_global_variables();
    if (g->get(s__SERVER).toArray().exists(s_HTTP_CONTENT_ENCODING)) {
      String encoding = g->get(s__SERVER)
        .toArray()[s_HTTP_CONTENT_ENCODING].toString();
      Variant ret;
      if (encoding == s_gzip || encoding == s_xgzip) {
        ret = HHVM_FN(gzinflate)(String(data, size, CopyString));
      } else if (encoding == s_deflate) {
        ret = HHVM_FN(gzuncompress)(String(data, size, CopyString));
      } else {
        raise_warning("Request is encoded with unknown compression '%s'",
                        encoding.data());
        return;
      }
      if (!ret.isString()) {
        raise_warning("Can't uncompress compressed request");
        return;
      }
      req = ret.toString();
    }
  }
  xmlDocPtr doc_request = soap_xmlParseMemory(req.data(), req.size());
  if (doc_request == NULL) {
    throw_soap_server_fault("Client", "Bad Request");
  }
  if (xmlGetIntSubset(doc_request) != NULL) {
    xmlNodePtr env = get_node(doc_request->children,"Envelope");
    if (env && env->ns) {
      if (strcmp((char*)env->ns->href, SOAP_1_1_ENV_NAMESPACE) == 0) {
        SOAP_GLOBAL(soap_version) = SOAP_1_1;
      } else if (strcmp((char*)env->ns->href,SOAP_1_2_ENV_NAMESPACE) == 0) {
        SOAP_GLOBAL(soap_version) = SOAP_1_2;
      }
    }
    xmlFreeDoc(doc_request);
    throw_soap_server_fault("Server", "DTD are not supported by SOAP");
  }

  // 2. find out which PHP function to call with what params
  SoapServiceScope sss(this);
  String function_name;
  Array params;
  int soap_version = 0;
  std::shared_ptr<sdlFunction> function;
  try {
    function = deserialize_function_call(m_sdl, doc_request, m_actor.c_str(),
                                         function_name, params, soap_version,
                                         m_soap_headers);
  } catch (Exception &e) {
    xmlFreeDoc(doc_request);
    send_soap_server_fault(function, e, NULL);
    return;
  }
  xmlFreeDoc(doc_request);

  // 3. we may need an object
  Object soap_obj;
  if (m_type == SOAP_OBJECT) {
    soap_obj = m_soap_object;
  } else if (m_type == SOAP_CLASS) {
    try {
      soap_obj = create_object(m_soap_class.name,
                               m_soap_class.argv);
    } catch (Exception &e) {
      send_soap_server_fault(function, e, NULL);
      return;
    }
  }

  // 4. process soap headers
  for (ArrayIter iter(m_soap_headers); iter; ++iter) {
    soapHeader *h = iter.second().toResource().getTyped<soapHeader>();
    if (m_sdl && !h->function && !h->hdr) {
      if (h->mustUnderstand) {
        throw_soap_server_fault("MustUnderstand","Header not understood");
      }
      continue;
    }

    String fn_name = h->function_name;
    if (valid_function(this, soap_obj, fn_name)) {
      try {
        if (m_type == SOAP_CLASS || m_type == SOAP_OBJECT) {
          h->retval = vm_call_user_func
            (make_packed_array(soap_obj, fn_name), h->parameters);
        } else {
          h->retval = vm_call_user_func(fn_name, h->parameters);
        }
      } catch (Exception &e) {
        send_soap_server_fault(function, e, h);
        return;
      }
      if (h->retval.isObject() &&
          h->retval.getObjectData()->instanceof(SystemLib::s_SoapFaultClass)) {
        send_soap_server_fault(function, h->retval, h);
        return;
      }
    } else if (h->mustUnderstand) {
      throw_soap_server_fault("MustUnderstand","Header not understood");
    }
  }

  // 5. main call
  String fn_name = function_name;
  Variant retval;
  if (valid_function(this, soap_obj, fn_name)) {
    try {
      if (m_type == SOAP_CLASS || m_type == SOAP_OBJECT) {
        retval = vm_call_user_func
          (make_packed_array(soap_obj, fn_name), params);
      } else {
        retval = vm_call_user_func(fn_name, params);
      }
    } catch (Exception &e) {
      send_soap_server_fault(function, e, NULL);
      return;
    }
    if (retval.isObject() &&
        retval.toObject()->instanceof(SystemLib::s_SoapFaultClass)) {
      send_soap_server_fault(function, retval, NULL);
      return;
    }
  } else {
    throw SoapException("Function '%s' doesn't exist", fn_name.data());
  }

  // 6. serialize response
  String response_name;
  if (function && !function->responseName.empty()) {
    response_name = function->responseName;
  } else {
    response_name = function_name + "Response";
  }
  xmlDocPtr doc_return = NULL;
  try {
    doc_return = serialize_response_call(function, response_name.data(),
                                         m_uri.c_str(), retval, m_soap_headers,
                                         soap_version);
  } catch (Exception &e) {
    send_soap_server_fault(function, e, NULL);
    return;
  }

  // 7. throw away all buffered output so far, so we can send back a clean
  //    soap resposne
  f_ob_end_clean();

  // 8. special case
  if (doc_return == NULL) {
    f_header("HTTP/1.1 202 Accepted");
    f_header("Content-Length: 0");
    return;
  }

  // 9. XML response
  xmlChar *buf; int size;
  xmlDocDumpMemory(doc_return, &buf, &size);
  xmlFreeDoc(doc_return);
  if (buf == NULL || size == 0) {
    if (buf) xmlFree(buf);
    throw SoapException("Dump memory failed");
  }
  output_xml_header(soap_version);
  if (buf) {
    echo(String((char*)buf, size, CopyString));
    xmlFree(buf);
  }
}

void c_SoapServer::t_setpersistence(int64_t mode) {
  SoapServerScope ss(this);
  if (m_type == SOAP_CLASS) {
    if (mode == SOAP_PERSISTENCE_SESSION || mode == SOAP_PERSISTENCE_REQUEST) {
      m_soap_class.persistance = mode;
    } else {
      raise_warning("Tried to set persistence with bogus value (%" PRId64 ")",
                    mode);
    }
  } else {
    raise_warning("Tried to set persistence when you are using you "
                    "SOAP SERVER in function mode, no persistence needed");
  }
}

void c_SoapServer::t_fault(const Variant& code, const String& fault,
                           const String& actor /* = null_string */,
                           const Variant& detail /* = null */,
                           const String& name /* = null_string */) {
  SoapServerScope ss(this);
  Object obj(SystemLib::AllocSoapFaultObject(code, fault, actor, detail, name));
  send_soap_server_fault(std::shared_ptr<sdlFunction>(), obj, NULL);
}

void c_SoapServer::t_addsoapheader(const Object& fault) {
  SoapServerScope ss(this);
  soapHeader *p = NEWOBJ(soapHeader)();
  Resource obj(p);
  p->function = NULL;
  p->mustUnderstand = false;
  p->retval = fault;
  p->hdr = NULL;
  m_soap_headers.append(obj);
}

///////////////////////////////////////////////////////////////////////////////
// class SoapClient

c_SoapClient::c_SoapClient(Class* cb) :
    ExtObjectDataFlags<ObjectData::HasCall>(cb),
    m_soap_version(SOAP_1_1),
    m_sdl(NULL),
    m_encoding(NULL),
    m_typemap(NULL),
    m_features(0),
    m_style(SOAP_RPC),
    m_use(SOAP_LITERAL),
    m_authentication(SOAP_AUTHENTICATION_BASIC),
    m_proxy_port(0),
    m_connection_timeout(0),
    m_max_redirect(0),
    m_use11(true),
    m_compression(false),
    m_exceptions(true),
    m_trace(false) {
}

c_SoapClient::~c_SoapClient() {
}

void c_SoapClient::t___construct(const Variant& wsdl,
                                 const Array& options /* = null_array */) {
  USE_SOAP_GLOBAL;
  SoapClientScope ss(this);

  try {

  if (!wsdl.isString() && !wsdl.isNull()) {
    throw SoapException("wsdl must be string or null");
  }

  int64_t cache_wsdl = SOAP_GLOBAL(cache);
  if (!options.empty()) {
    m_location = options[s_location];

    if (wsdl.isNull()) {
      /* Fetching non-WSDL mode options */
      m_uri   = options[s_uri];
      m_style = options[s_style].toInt32(); // SOAP_RPC || SOAP_DOCUMENT
      m_use   = options[s_use].toInt32(); // SOAP_LITERAL || SOAP_ENCODED

      if (m_uri.empty()) {
        throw SoapException("'uri' option is required in nonWSDL mode");
      }
      if (m_location.empty()) {
        throw SoapException("'location' option is required in nonWSDL mode");
      }
    }

    if (options.exists(s_stream_context)) {
      StreamContext *sc = NULL;
      if (options[s_stream_context].isResource()) {
        sc = options[s_stream_context].toResource()
                                      .getTyped<StreamContext>();
      }
      if (!sc) {
        throw SoapException("'stream_context' is not a StreamContext");
      }
      m_stream_context_options = sc->getOptions();
    }

    if (options.exists(s_soap_version)) {
      m_soap_version = options[s_soap_version].toInt32();
    }

    m_login = options[s_login].toString();
    m_password = options[s_password].toString();
    m_authentication = options[s_authentication].toInt32();

    m_proxy_host = options[s_proxy_host].toString();
    m_proxy_port = options[s_proxy_port].toInt32();
    m_proxy_login = options[s_proxy_login].toString();
    m_proxy_password = options[s_proxy_password].toString();

    m_trace = options[s_trace].toBoolean();
    if (options.exists(s_exceptions)) {
      m_exceptions = options[s_exceptions].toBoolean();
    }
    if (options.exists(s_compression)) {
      m_compression = options[s_compression].toBoolean();
    }

    String encoding = options[s_encoding].toString();
    if (!encoding.empty()) {
      m_encoding = xmlFindCharEncodingHandler(encoding.data());
      if (m_encoding == NULL) {
        throw SoapException("Invalid 'encoding' option - '%s'",
                            encoding.data());
      }
      s_soap_data->register_encoding(m_encoding);
    }
    m_classmap = options[s_classmap].toArray();
    m_features = options[s_features].toInt32();
    m_connection_timeout = options[s_connection_timeout].toInt64();
    m_user_agent = options[s_user_agent].toString();

    if (options.exists(s_cache_wsdl)) {
      cache_wsdl = options[s_cache_wsdl].toInt64();
    }

  } else if (wsdl.isNull()) {
    throw SoapException("'location' and 'uri' options are required in "
                        "nonWSDL mode");
  }

  if (!wsdl.isNull()) {
    int old_soap_version = SOAP_GLOBAL(soap_version);
    SOAP_GLOBAL(soap_version) = m_soap_version;
    String swsdl = wsdl.toString();
    if (swsdl.find("http://") == 0 || swsdl.find("https://") == 0) {
      HttpClient http(m_connection_timeout, m_max_redirect, m_use11, true);
      if (!m_proxy_host.empty() && m_proxy_port) {
        http.proxy(m_proxy_host.data(), m_proxy_port, m_proxy_login.data(),
                   m_proxy_password.data());
      }
      if (!m_login.empty()) {
        http.auth(m_login.data(), m_password.data(), !m_digest);
      }
      http.setStreamContextOptions(m_stream_context_options);
      m_sdl = s_soap_data->get_sdl(swsdl.data(), cache_wsdl, &http);
    } else {
      m_sdl = s_soap_data->get_sdl(swsdl.data(), cache_wsdl);
    }
    SOAP_GLOBAL(soap_version) = old_soap_version;
  }

  Variant v = options[s_typemap];
  if (v.isArray()) {
    Array arr = v.toArray();
    if (!arr.empty()) {
      m_typemap = soap_create_typemap(m_sdl, arr);
    }
  }

  } catch (Exception &e) {
    throw create_soap_fault(e);
  }
}

Variant c_SoapClient::t___call(Variant name, Variant args) {
  return t___soapcall(name.toString(), args.toArray());
}

Variant c_SoapClient::t___soapcall(const String& name, const Array& args,
                                   const Array& options /* = null_array */,
                                   const Variant& input_headers /* = null_variant */,
                                   VRefParam output_headers_ref /* = null */) {
  SoapClientScope ss(this);

  String location, soap_action, uri;
  if (!options.isNull()) {
    if (options[s_location].isString()) {
      location = options[s_location].toString();
      if (location.isNull()) {
        location = m_location;
      }
    }
    if (options[s_soapaction].isString()) {
      soap_action = options[s_soapaction].toString();
    }
    if (options[s_uri].isString()) {
      uri = options[s_uri].toString();
    }
  }

  Array soap_headers = Array::Create();
  if (input_headers.isNull()) {
  } else if (input_headers.isArray()) {
    Array arr = input_headers.toArray();
    verify_soap_headers_array(arr);
    soap_headers = input_headers;
  } else if (input_headers.isObject() &&
             input_headers.toObject().is<c_SoapHeader>()) {
    soap_headers = make_packed_array(input_headers);
  } else{
    raise_warning("Invalid SOAP header");
    return uninit_null();
  }
  if (!m_default_headers.isNull()) {
    soap_headers.merge(m_default_headers.toArray());
  }

  Array output_headers;
  SCOPE_EXIT {
    output_headers_ref = output_headers;
  };

  if (m_trace) {
    m_last_request = Variant();
    m_last_response = Variant();
  }

  if (location.empty()) {
    location = m_location;
  }

  m_soap_fault = Variant();

  SoapServiceScope sss(this);
  Variant return_value;
  bool ret = false;
  xmlDocPtr request = NULL;
  if (m_sdl) {
    auto fn = get_function(m_sdl, name.data());
    if (fn) {
      sdlBindingPtr binding = fn->binding;
      bool one_way = false;
      if (fn->responseName.empty() && fn->responseParameters.empty() &&
          soap_headers.empty()) {
        one_way = true;
      }
      if (location.empty()) {
        location = binding->location;
      }

      Variant response;
      try {
        if (binding->bindingType == BINDING_SOAP) {
          sdlSoapBindingFunctionPtr fnb = fn->bindingAttributes;
          request = serialize_function_call
            (this, fn, NULL, fnb->input.ns.c_str(), args, soap_headers);
          ret = do_request(this, request, location.data(),
                           fnb->soapAction.c_str(), m_soap_version, one_way,
                           response);
        }  else {
          request = serialize_function_call
            (this, fn, NULL, m_sdl->target_ns.c_str(), args, soap_headers);
          ret = do_request(this, request, location.data(), NULL,
                           m_soap_version, one_way, response);
        }
      } catch (Exception &e) {
        xmlFreeDoc(request);
        throw create_soap_fault(e);
      }
      xmlFreeDoc(request);

      if (ret && response.isString()) {
        encode_reset_ns();
        String sresponse = response.toString();
        ret = parse_packet_soap(this, sresponse.data(), sresponse.size(),
                                fn, NULL, return_value, output_headers);
        encode_finish();
      }
    } else {
      StringBuffer error;
      error.append("Function (\"");
      error.append(name.data());
      error.append("\") is not a valid method for this service");
      m_soap_fault = create_soap_fault("Client", error.detach());
    }
  } else {
    string action;
    if (m_uri.empty()) {
      m_soap_fault =
        create_soap_fault("Client", "Error finding \"uri\" property");
    } else if (location.empty()) {
      m_soap_fault =
        create_soap_fault("Client", "Error could not find \"location\" "
                          "property");
    } else {
      request = serialize_function_call
        (this, std::shared_ptr<sdlFunction>(), name.data(), m_uri.data(), args,
         soap_headers);
      if (soap_action.empty()) {
        action += m_uri.data();
        action += '#';
        action += name.data();
      } else {
        action += (std::string) soap_action;
      }
      Variant response;
      try {
        ret = do_request(this, request, location.c_str(), action.c_str(),
                         m_soap_version, 0, response);
      } catch (Exception &e) {
        xmlFreeDoc(request);
        throw create_soap_fault(e);
      }
      xmlFreeDoc(request);
      if (ret && response.isString()) {
        encode_reset_ns();
        String sresponse = response.toString();
        ret = parse_packet_soap(this, sresponse.data(), sresponse.size(),
                                std::shared_ptr<sdlFunction>(),
                                name.data(),
                                return_value,
                                output_headers);
        encode_finish();
      }
    }
  }

  if (!ret && m_soap_fault.isNull()) {
    m_soap_fault = create_soap_fault("Client", "Unknown Error");
  }
  if (!m_soap_fault.isNull()) {
    throw m_soap_fault.toObject();
  }
  return return_value;
}

Variant c_SoapClient::t___getlastrequest() {
  return m_last_request;
}

Variant c_SoapClient::t___getlastresponse() {
  return m_last_response;
}

Variant c_SoapClient::t___getlastrequestheaders() {
  return m_last_request_headers;
}

Variant c_SoapClient::t___getlastresponseheaders() {
  return m_last_response_headers;
}

Variant c_SoapClient::t___getfunctions() {
  SoapClientScope ss(this);

  if (m_sdl) {
    Array ret = Array::Create();
    for (sdlFunctionMap::iterator iter = m_sdl->functions.begin();
         iter != m_sdl->functions.end(); ++iter) {
      StringBuffer sb;
      function_to_string(iter->second, sb);
      ret.append(sb.detach());
    }
    return ret;
  }
  return uninit_null();
}

Variant c_SoapClient::t___gettypes() {
  SoapClientScope ss(this);

  if (m_sdl) {
    Array ret = Array::Create();
    for (unsigned int i = 0; i < m_sdl->types.size(); i++) {
      StringBuffer sb;
      type_to_string(m_sdl->types[i].get(), sb, 0);
      ret.append(sb.detach());
    }
    return ret;
  }
  return uninit_null();
}

Variant c_SoapClient::t___dorequest(const String& buf, const String& location, const String& action,
                                    int64_t version, bool oneway /* = false */) {
  if (location.empty()) {
    m_soap_fault =
      Object(SystemLib::AllocSoapFaultObject("HTTP", "Unable to parse URL"));
    return uninit_null();
  }

  USE_SOAP_GLOBAL;
  SoapClientScope ss(this);

  HeaderMap headers;

  String buffer(buf);

  // compression
  if (m_compression > 0) {
    if (m_compression & SOAP_COMPRESSION_ACCEPT) {
      headers["Accept-Encoding"].push_back("gzip, deflate");
    }
    int level = m_compression & 0x0f;
    if (level > 9) level = 9;
    if (level > 0) {
      Variant ret;
      if (m_compression & SOAP_COMPRESSION_DEFLATE) {
        ret = HHVM_FN(gzcompress)(buffer, level);
        headers["Content-Encoding"].push_back("deflate");
      } else {
        ret = HHVM_FN(gzencode)(buffer, level);
        headers["Content-Encoding"].push_back("gzip");
      }
      if (!ret.isString()) return uninit_null();
      buffer = ret.toString();
    }
  }

  // prepare more headers
  if (!m_user_agent.empty()) {
    headers["User-Agent"].push_back(m_user_agent.data());
  }
  string contentType;
  if (version == SOAP_1_2) {
    contentType = "application/soap+xml; charset=utf-8";
    contentType += "; action=\"";
    contentType += action.data();
    contentType += "\"";
    headers["Content-Type"].push_back(contentType);
  } else {
    contentType = "text/xml; charset=utf-8";
    headers["Content-Type"].push_back(contentType);
    headers["SOAPAction"].push_back(string("\"") + action.data() + "\"");
  }

  // post the request
  HttpClient http(m_connection_timeout, m_max_redirect, m_use11, true);
  if (!m_proxy_host.empty() && m_proxy_port) {
    http.proxy(m_proxy_host.data(), m_proxy_port, m_proxy_login.data(),
               m_proxy_password.data());
  }
  if (!m_login.empty()) {
    http.auth(m_login.data(), m_password.data(), !m_digest);
  }
  http.setStreamContextOptions(m_stream_context_options);
  StringBuffer response;
  int code = http.post(location.data(), buffer.data(), buffer.size(), response,
                       &headers);
  if (code == 0) {
    String msg = "Failed Sending HTTP Soap request";
    if (!http.getLastError().empty()) {
      msg += ": " + http.getLastError();
    }
    m_soap_fault = Object(SystemLib::AllocSoapFaultObject(
      "HTTP", msg));
    return uninit_null();
  }
  if (code != 200) {
    String msg = response.detach();
    if (msg.empty()) {
      msg = HttpProtocol::GetReasonString(code);
    }
    m_soap_fault = Object(SystemLib::AllocSoapFaultObject("HTTP", msg));
    return uninit_null();
  }

  // return response
  if (SOAP_GLOBAL(features) & SOAP_WAIT_ONE_WAY_CALLS) {
    oneway = false;
  }
  if (oneway) {
    return "";
  }
  return response.detach();
}

Variant c_SoapClient::t___setcookie(const String& name,
                                    const String& value /* = null_string */) {
  if (!value.isNull()) {
    m_cookies.set(name, make_packed_array(value));
  } else {
    const Variant* t = o_realProp("_cookies", RealPropUnchecked);
    if (t && t->isInitialized()) {
      m_cookies.remove(name);
    }
  }
  return uninit_null();
}

Variant c_SoapClient::t___setlocation(const String& new_location /* = null_string */) {
  Variant ret = m_location;
  m_location = new_location;
  return ret;
}

bool c_SoapClient::t___setsoapheaders(const Variant& headers /* = null_variant */) {
  if (headers.isNull()) {
    m_default_headers = uninit_null();
  } else if (headers.isArray()) {
    Array arr = headers.toArray();
    verify_soap_headers_array(arr);
    m_default_headers = arr;
  } else if (headers.isObject() && headers.toObject().is<c_SoapHeader>()) {
    m_default_headers = make_packed_array(headers);
  } else {
    raise_warning("Invalid SOAP header");
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// class SoapVar

c_SoapVar::c_SoapVar(Class* cb) : ExtObjectData(cb) {
}

c_SoapVar::~c_SoapVar() {
}

void c_SoapVar::t___construct(const Variant& data, const Variant& type,
                              const String& type_name /* = null_string */,
                              const String& type_namespace /* = null_string */,
                              const String& node_name /* = null_string */,
                              const String& node_namespace /* = null_string */) {
  USE_SOAP_GLOBAL;
  if (type.isNull()) {
    m_type = UNKNOWN_TYPE;
  } else {
    std::map<int, encodePtr> &defEncIndex = SOAP_GLOBAL(defEncIndex);
    int64_t ntype = type.toInt64();
    if (defEncIndex.find(ntype) != defEncIndex.end()) {
      m_type = ntype;
    } else {
      raise_warning("Invalid type ID");
      return;
    }
  }

  if (data.toBoolean())        m_value  = data;
  if (!type_name.empty())      m_stype  = type_name;
  if (!type_namespace.empty()) m_ns     = type_namespace;
  if (!node_name.empty())      m_name   = node_name;
  if (!node_namespace.empty()) m_namens = node_namespace;
}

///////////////////////////////////////////////////////////////////////////////
// class SoapParam

c_SoapParam::c_SoapParam(Class* cb) : ExtObjectData(cb) {
}

c_SoapParam::~c_SoapParam() {
}

void c_SoapParam::t___construct(const Variant& data, const String& name) {
  if (name.empty()) {
    raise_warning("Invalid parameter name");
    return;
  }
  m_name = name;
  m_data = data;
}

///////////////////////////////////////////////////////////////////////////////
// class SoapHeader

c_SoapHeader::c_SoapHeader(Class* cb) :
    ExtObjectData(cb) {
}

c_SoapHeader::~c_SoapHeader() {
}

void c_SoapHeader::t___construct(const String& ns, const String& name,
                                 const Variant& data /* = null */,
                                 bool mustunderstand /* = false */,
                                 const Variant& actor /* = null */) {
  if (ns.empty()) {
    raise_warning("Invalid namespace");
    return;
  }
  if (name.empty()) {
    raise_warning("Invalid header name");
    return;
  }

  m_namespace = ns;
  m_name = name;
  m_data = data;
  m_mustUnderstand = mustunderstand;

  if (actor.isInteger() &&
      (actor.toInt64() == SOAP_ACTOR_NEXT ||
       actor.toInt64() == SOAP_ACTOR_NONE ||
       actor.toInt64() == SOAP_ACTOR_UNLIMATERECEIVER)) {
    m_actor = actor.toInt64();
  } else if (actor.isString() && !actor.toString().empty()) {
    m_actor = actor.toString();
  } else if (!actor.isNull()) {
    raise_warning("Invalid actor");
  }
}

///////////////////////////////////////////////////////////////////////////////
}
