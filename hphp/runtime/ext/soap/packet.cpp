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
#include "hphp/runtime/ext/soap/packet.h"

#include <memory>
#include "hphp/runtime/ext/soap/ext_soap.h"
#include "hphp/util/hash-map-typedefs.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static void add_soap_fault(SoapClient *client, const String& code,
                           const String& fault) {
  client->m_soap_fault =
    SystemLib::AllocSoapFaultObject(String(code, CopyString), fault);
}

/* SOAP client calls this function to parse response from SOAP server */
bool parse_packet_soap(SoapClient *obj, const char *buffer,
                       int buffer_size,
                       std::shared_ptr<sdlFunction> fn, const char *fn_name,
                       Variant &return_value, Array& soap_headers) {
  char* envelope_ns = nullptr;
  xmlNodePtr trav, env, head, body, resp, cur, fault;
  xmlAttrPtr attr;
  int param_count = 0;
  int soap_version = SOAP_1_1;
  sdlSoapBindingFunctionHeaderMap *hdrs = nullptr;

  assert(return_value.asTypedValue()->m_type == KindOfUninit);
  return_value.asTypedValue()->m_type = KindOfNull;

  /* Response for one-way opearation */
  if (buffer_size == 0) {
    return true;
  }

  /* Parse XML packet */
  xmlDocPtr response = soap_xmlParseMemory(buffer, buffer_size);
  if (!response) {
    add_soap_fault(obj, "Client", "looks like we got no XML document");
    return false;
  }
  if (xmlGetIntSubset(response) != nullptr) {
    add_soap_fault(obj, "Client", "DTD are not supported by SOAP");
    xmlFreeDoc(response);
    return false;
  }

  /* Get <Envelope> element */
  env = nullptr;
  trav = response->children;
  while (trav != nullptr) {
    if (trav->type == XML_ELEMENT_NODE) {
      if (!env && node_is_equal_ex(trav,"Envelope", SOAP_1_1_ENV_NAMESPACE)) {
        env = trav;
        envelope_ns = SOAP_1_1_ENV_NAMESPACE;
        soap_version = SOAP_1_1;
      } else if (!env &&
                 node_is_equal_ex(trav, "Envelope", SOAP_1_2_ENV_NAMESPACE)) {
        env = trav;
        envelope_ns = SOAP_1_2_ENV_NAMESPACE;
        soap_version = SOAP_1_2;
      } else {
        add_soap_fault(obj, "VersionMismatch", "Wrong Version");
        xmlFreeDoc(response);
        return false;
      }
    }
    trav = trav->next;
  }
  if (env == nullptr) {
    add_soap_fault(obj, "Client",
                   "looks like we got XML without \"Envelope\" element");
    xmlFreeDoc(response);
    return false;
  }

  attr = env->properties;
  while (attr != nullptr) {
    if (attr->ns == nullptr) {
      add_soap_fault(obj, "Client",
                     "A SOAP Envelope element cannot have non Namespace "
                     "qualified attributes");
      xmlFreeDoc(response);
      return false;
    }
    if (attr_is_equal_ex(attr, "encodingStyle", SOAP_1_2_ENV_NAMESPACE)) {
      if (soap_version == SOAP_1_2) {
        add_soap_fault(obj, "Client",
                       "encodingStyle cannot be specified on the Envelope");
        xmlFreeDoc(response);
        return false;
      }
      if (strcmp((char*)attr->children->content, SOAP_1_1_ENC_NAMESPACE)) {
        add_soap_fault(obj, "Client", "Unknown data encoding style");
        xmlFreeDoc(response);
        return false;
      }
    }
    attr = attr->next;
  }

  /* Get <Header> element */
  head = nullptr;
  trav = env->children;
  while (trav != nullptr && trav->type != XML_ELEMENT_NODE) {
    trav = trav->next;
  }
  if (trav != nullptr && node_is_equal_ex(trav,"Header",envelope_ns)) {
    head = trav;
    trav = trav->next;
  }

  /* Get <Body> element */
  body = nullptr;
  while (trav != nullptr && trav->type != XML_ELEMENT_NODE) {
    trav = trav->next;
  }
  if (trav != nullptr && node_is_equal_ex(trav,"Body",envelope_ns)) {
    body = trav;
    trav = trav->next;
  }
  while (trav != nullptr && trav->type != XML_ELEMENT_NODE) {
    trav = trav->next;
  }
  if (body == nullptr) {
    add_soap_fault(obj, "Client", "Body must be present in a SOAP envelope");
    xmlFreeDoc(response);
    return false;
  }
  attr = body->properties;
  while (attr != nullptr) {
    if (attr->ns == nullptr) {
      if (soap_version == SOAP_1_2) {
        add_soap_fault(obj, "Client",
                       "A SOAP Body element cannot have non Namespace "
                       "qualified attributes");
        xmlFreeDoc(response);
        return false;
      }
    } else if (attr_is_equal_ex(attr,"encodingStyle",SOAP_1_2_ENV_NAMESPACE)) {
      if (soap_version == SOAP_1_2) {
        add_soap_fault(obj, "Client",
                       "encodingStyle cannot be specified on the Body");
        xmlFreeDoc(response);
        return false;
      }
      if (strcmp((char*)attr->children->content, SOAP_1_1_ENC_NAMESPACE)) {
        add_soap_fault(obj, "Client", "Unknown data encoding style");
        xmlFreeDoc(response);
        return false;
      }
    }
    attr = attr->next;
  }
  if (trav != nullptr && soap_version == SOAP_1_2) {
    add_soap_fault(obj, "Client",
                   "A SOAP 1.2 envelope can contain only Header and Body");
    xmlFreeDoc(response);
    return false;
  }

  if (head != nullptr) {
    attr = head->properties;
    while (attr != nullptr) {
      if (attr->ns == nullptr) {
        add_soap_fault(obj, "Client",
                       "A SOAP Header element cannot have non Namespace "
                       "qualified attributes");
        xmlFreeDoc(response);
        return false;
      }
      if (attr_is_equal_ex(attr, "encodingStyle", SOAP_1_2_ENV_NAMESPACE)) {
        if (soap_version == SOAP_1_2) {
          add_soap_fault(obj, "Client",
                         "encodingStyle cannot be specified on the Header");
          xmlFreeDoc(response);
          return false;
        }
        if (strcmp((char*)attr->children->content, SOAP_1_1_ENC_NAMESPACE)) {
          add_soap_fault(obj, "Client", "Unknown data encoding style");
          xmlFreeDoc(response);
          return false;
        }
      }
      attr = attr->next;
    }
  }

  /* Check if <Body> contains <Fault> element */
  fault = get_node_ex(body->children,"Fault",envelope_ns);
  if (fault != nullptr) {
    char *faultcode = nullptr;
    String faultstring, faultactor;
    Variant details;
    xmlNodePtr tmp;

    if (soap_version == SOAP_1_1) {
      tmp = get_node(fault->children, "faultcode");
      if (tmp != nullptr && tmp->children != nullptr) {
        faultcode = (char*)tmp->children->content;
      }

      tmp = get_node(fault->children, "faultstring");
      if (tmp != nullptr && tmp->children != nullptr) {
        Variant zv =
          master_to_zval(get_conversion(dataTypeToSoap(KindOfString)), tmp);
        faultstring = zv.toString();
      }

      tmp = get_node(fault->children, "faultactor");
      if (tmp != nullptr && tmp->children != nullptr) {
        Variant zv =
          master_to_zval(get_conversion(dataTypeToSoap(KindOfString)), tmp);
        faultactor = zv.toString();
      }

      tmp = get_node(fault->children, "detail");
      if (tmp != nullptr) {
        details = master_to_zval(encodePtr(), tmp);
      }
    } else {
      tmp = get_node(fault->children, "Code");
      if (tmp != nullptr && tmp->children != nullptr) {
        tmp = get_node(tmp->children, "Value");
        if (tmp != nullptr && tmp->children != nullptr) {
          faultcode = (char*)tmp->children->content;
        }
      }

      tmp = get_node(fault->children,"Reason");
      if (tmp != nullptr && tmp->children != nullptr) {
        /* TODO: lang attribute */
        tmp = get_node(tmp->children,"Text");
        if (tmp != nullptr && tmp->children != nullptr) {
          Variant zv =
            master_to_zval(get_conversion(dataTypeToSoap(KindOfString)), tmp);
          faultstring = zv.toString();
        }
      }

      tmp = get_node(fault->children,"Detail");
      if (tmp != nullptr) {
        details = master_to_zval(encodePtr(), tmp);
      }
    }
    obj->m_soap_fault =
        SystemLib::AllocSoapFaultObject(String(faultcode, CopyString),
                                        faultstring,
                                        faultactor,
                                        details);
    xmlFreeDoc(response);
    return false;
  }

  /* Parse content of <Body> element */
  return_value = Array::Create();
  resp = body->children;
  while (resp != nullptr && resp->type != XML_ELEMENT_NODE) {
    resp = resp->next;
  }
  if (resp != nullptr) {
    if (fn && fn->binding && fn->binding->bindingType == BINDING_SOAP) {
      /* Function has WSDL description */
      sdlParamPtr h_param, param;
      xmlNodePtr val = nullptr;
      const char *name, *ns = nullptr;
      Variant tmp(Variant::NullInit{});
      sdlSoapBindingFunctionPtr fnb =
        (sdlSoapBindingFunctionPtr)fn->bindingAttributes;
      int res_count;

      hdrs = &fnb->output.headers;

      if (!fn->responseParameters.empty()) {
        res_count = fn->responseParameters.size();
        for (unsigned int i = 0; i < fn->responseParameters.size(); i++) {
          h_param = fn->responseParameters[i];
          param = h_param;
          if (fnb->style == SOAP_DOCUMENT) {
            if (param->element) {
              name = param->element->name.c_str();
              ns = param->element->namens.c_str();
/*
              name = param->encode->details.type_str;
              ns = param->encode->details.ns;
*/
            } else {
              name = param->paramName.c_str();
            }
          } else {
            name = fn->responseName.c_str();
            /* ns = ? */
          }

          /* Get value of parameter */
          cur = get_node_ex(resp, (char*)name, (char*)ns);
          if (!cur) {
            cur = get_node(resp, (char*)name);
            /* TODO: produce warning invalid ns */
          }
          if (!cur && fnb->style == SOAP_RPC) {
            cur = resp;
          }
          if (cur) {
            if (fnb->style == SOAP_DOCUMENT) {
              val = cur;
            } else {
              val = get_node(cur->children, (char*)param->paramName.c_str());
              if (res_count == 1) {
                if (val == nullptr) {
                  val = get_node(cur->children, "return");
                }
                if (val == nullptr) {
                  val = get_node(cur->children, "result");
                }
                if (val == nullptr && cur->children && !cur->children->next) {
                  val = cur->children;
                }
              }
            }
          }

          if (!val) {
            /* TODO: may be "nil" is not OK? */
/*
            add_soap_fault(obj, "Client", "Can't find response data");
            xmlFreeDoc(response);
            return false;
*/
          } else {
            /* Decoding value of parameter */
            if (param != nullptr) {
              tmp = master_to_zval(param->encode, val);
            } else {
              tmp = master_to_zval(encodePtr(), val);
            }
          }
          return_value.toArrRef().set(String(param->paramName), tmp);
          param_count++;
        }
      }
    } else {
      /* Function hasn't WSDL description */
      xmlNodePtr val;
      val = resp->children;
      while (val != nullptr) {
        while (val && val->type != XML_ELEMENT_NODE) {
          val = val->next;
        }
        if (val != nullptr) {
          if (!node_is_equal_ex(val,"result",RPC_SOAP12_NAMESPACE)) {
            Variant tmp = master_to_zval(encodePtr(), val);
            if (val->name) {
              String key((char*)val->name, CopyString);
              if (return_value.toCArrRef().exists(key)) {
                auto& lval = return_value.toArrRef().lvalAt(key);
                if (!lval.isArray()) lval = lval.toArray();
                lval.toArrRef().append(tmp);
              } else if (val->next && get_node(val->next, (char*)val->name)) {
                Array arr = Array::Create();
                arr.append(tmp);
                return_value.toArrRef().set(key, arr);
              } else {
                return_value.toArrRef().set(key, tmp);
              }
            } else {
              return_value.toArrRef().append(tmp);
            }
            ++param_count;
          }
          val = val->next;
        }
      }
    }
  }

  if (return_value.isArray()) {
    if (param_count == 0) {
      return_value = init_null();
    } else if (param_count == 1) {
      Array arr = return_value.toArray();
      ArrayIter iter(arr);
      return_value = iter.second();
    }
  }

  if (head) {
    trav = head->children;
    while (trav) {
      if (trav->type == XML_ELEMENT_NODE) {
        encodePtr enc;
        if (hdrs && !hdrs->empty()) {
          std::string key;
          if (trav->ns) {
            key += (char*)trav->ns->href;
            key += ':';
          }
          key += (char*)trav->name;
          sdlSoapBindingFunctionHeaderMap::const_iterator iter =
            hdrs->find(key);
          if (iter != hdrs->end()) {
            enc = iter->second->encode;
          }
        }
        soap_headers.set(String((char*)trav->name, CopyString),
                         master_to_zval(enc, trav));
      }
      trav = trav->next;
    }
  }

  xmlFreeDoc(response);
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
