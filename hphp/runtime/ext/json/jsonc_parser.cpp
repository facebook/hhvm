/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
   | Authors: Omar Kilani <omar@php.net>                                  |
   |          Remi Collet <remi@php.net>                                  |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_JSONC

#include <cstdint>

#ifdef JSONC_INCLUDE_WITH_C
#include <json-c/json.h>
#else
#include <json/json.h>
#endif

#include <folly/CppAttributes.h>

#include "hphp/runtime/ext/collections/ext_collections-map.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"
#include "hphp/runtime/ext/json/JSON_parser.h"
#include "hphp/runtime/ext/json/ext_json.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/utf8-decode.h"
#include "hphp/runtime/base/variable-serializer.h"

namespace HPHP {

const StaticString
    s_empty("_empty_");

struct json_state {
    json_error_codes error_code;
    json_tokener_error parser_code;
};

THREAD_LOCAL(json_state, s_json_state);

json_error_codes json_get_last_error_code() {
    return s_json_state->error_code;
}

void json_set_last_error_code(json_error_codes error_code) {
    s_json_state->error_code = error_code;
    s_json_state->parser_code = json_tokener_success;
}

void json_set_last_error_code(json_error_codes error_code,
                              json_tokener_error parser_code) {
    s_json_state->error_code = error_code;
    s_json_state->parser_code = parser_code;
}

const char *json_get_last_error_msg() {
  switch (s_json_state->error_code) {
  case json_error_codes::JSON_ERROR_NONE:
    return "No error";
  case json_error_codes::JSON_ERROR_DEPTH:
    return "Maximum stack depth exceeded";
  case json_error_codes::JSON_ERROR_STATE_MISMATCH:
    return "State mismatch (invalid or malformed JSON)";
  case json_error_codes::JSON_ERROR_CTRL_CHAR:
    return "Control character error, possibly incorrectly encoded";
  case json_error_codes::JSON_ERROR_SYNTAX:
    return "Syntax error: @todo more info";
    return json_tokener_error_desc(s_json_state->parser_code);
  case json_error_codes::JSON_ERROR_UTF8:
    return "Malformed UTF-8 characters, possibly incorrectly encoded";
  case json_error_codes::JSON_ERROR_RECURSION:
    return "Recursion detected";
  case json_error_codes::JSON_ERROR_INF_OR_NAN:
    return "Inf and NaN cannot be JSON encoded";
  case json_error_codes::JSON_ERROR_UNSUPPORTED_TYPE:
    return "Type is not supported";
  default:
    return "Unknown error";
  }
}

///////////////////////////////////////////////////////////////////////////////

Variant json_object_to_variant(json_object *new_obj, const bool assoc,
                               const JSONContainerType container_type);

Variant json_type_array_to_variant(json_object *new_obj, const bool assoc,
                                   const JSONContainerType container_type) {
  int i, nb;
  Variant var, tmpvar;
  nb = json_object_array_length(new_obj);
  switch (container_type) {
    case JSONContainerType::COLLECTIONS:
      var = req::make<c_Vector>();
      break;
    case JSONContainerType::HACK_ARRAYS:
      var = Array::CreateVec();
      break;
    case JSONContainerType::PHP_ARRAYS:
      var = Array::Create();
      break;
  }

  for (i=0; i<nb; i++) {
    tmpvar = json_object_to_variant(json_object_array_get_idx(new_obj, i),
                                    assoc, container_type);
    if (container_type == JSONContainerType::COLLECTIONS) {
      collections::append(var.getObjectData(), tmpvar.asCell());
    } else {
      var.asArrRef().append(tmpvar);
    }
  }
  return var;
}

Variant json_type_object_to_variant(json_object *new_obj, const bool assoc,
                                    const JSONContainerType container_type) {
    struct json_object_iterator it, itEnd;
    json_object  *jobj;
    Variant       var, tmpvar;

  if (!assoc) {
    var = SystemLib::AllocStdClassObject();
  } else {
    switch (container_type) {
      case JSONContainerType::COLLECTIONS:
        var = req::make<c_Map>();
        break;
      case JSONContainerType::HACK_ARRAYS:
        var = Array::CreateDict();
        break;
      case JSONContainerType::PHP_ARRAYS:
        var = Array::Create();
        break;
    }
  }

  it = json_object_iter_begin(new_obj);
  itEnd = json_object_iter_end(new_obj);

  while (!json_object_iter_equal(&it, &itEnd)) {
    String key(json_object_iter_peek_name(&it), CopyString);
    jobj = json_object_iter_peek_value(&it);
    tmpvar = json_object_to_variant(jobj, assoc, container_type);

    if (!assoc) {
      if (key.empty()) {
        var.getObjectData()->setProp(nullptr, s_empty.get(), *tmpvar.asCell());
      } else {
        var.getObjectData()->o_set(key, tmpvar);
      }
    } else {
      switch (container_type) {
        case JSONContainerType::COLLECTIONS: {
          auto keyTV = make_tv<KindOfString>(key.get());
          collections::set(var.getObjectData(), &keyTV, tmpvar.asCell());
          break;
        }
        case JSONContainerType::HACK_ARRAYS:
          forceToDict(var).set(key, tmpvar);
          break;
        case JSONContainerType::PHP_ARRAYS:
          forceToArray(var).set(key, tmpvar);
          break;
      }
    }
    json_object_iter_next(&it);
  }
  return var;
}

Variant json_object_to_variant(json_object *new_obj, const bool assoc,
                               const JSONContainerType container_type) {
    json_type type;
    int64_t i64;

    if (!new_obj) {
        return Variant(Variant::NullInit());
    }

    type = json_object_get_type(new_obj);
    switch (type) {
    case json_type_int:
        i64 = json_object_get_int64(new_obj);
        if (!(i64==INT64_MAX || i64==INT64_MIN)) {
          return Variant(i64);
        }
        FOLLY_FALLTHROUGH;

    case json_type_double:
        return Variant(json_object_get_double(new_obj));

    case json_type_string:
        return Variant(String(json_object_get_string(new_obj),
                              json_object_get_string_len(new_obj),
                              CopyString));

    case json_type_boolean:
        if (json_object_get_boolean(new_obj)) {
            return Variant(true);
        } else {
            return Variant(false);
        }

    case json_type_null:
        return Variant(Variant::NullInit());

    case json_type_array:
        return json_type_array_to_variant(new_obj, assoc, container_type);

    case json_type_object:
        return json_type_object_to_variant(new_obj, assoc, container_type);

    default:
        // warning type <type> not yet implemented
        return Variant(Variant::NullInit());
    }
}

bool JSON_parser(Variant &return_value, const char *data, int data_len,
                 bool assoc, int depth, int64_t options) {
    json_tokener *tok;
    json_object *new_obj;
    bool retval = false;

#if JSON_C_MINOR_VERSION >= 11
    tok = json_tokener_new_ex(depth);
#else
    tok = json_tokener_new();
#endif
    if (!tok) {
        return retval;
    }

    //if (!(options & k_JSON_FB_LOOSE)) {
    //    json_tokener_set_flags(tok, JSON_TOKENER_STRICT);
    //}
    JSONContainerType container_type = JSONContainerType::PHP_ARRAYS;
    if (options & (k_JSON_FB_STABLE_MAPS | k_JSON_FB_COLLECTIONS)) {
      assoc = true;
      container_type = JSONContainerType::COLLECTIONS;
    } else if (options & k_JSON_FB_HACK_ARRAYS) {
      container_type = JSONContainerType::HACK_ARRAYS;
    }

    new_obj = json_tokener_parse_ex(tok, data, data_len);
    if (json_tokener_get_error(tok)==json_tokener_continue) {
        new_obj = json_tokener_parse_ex(tok, "", -1);
    }

    if (new_obj) {
        return_value = json_object_to_variant(new_obj, assoc, container_type);
        json_object_put(new_obj);
        retval = true;
    } else {
        switch (json_tokener_get_error(tok)) {
        case json_tokener_success:
            retval = true;
            break;

        case json_tokener_error_depth:
            json_set_last_error_code(json_error_codes::JSON_ERROR_DEPTH);
            break;

        default:
            json_set_last_error_code(json_error_codes::JSON_ERROR_SYNTAX,
                                     json_tokener_get_error(tok));
        }
    }

    json_tokener_free(tok);
    return retval;
}

void json_parser_init() {
    // Nop
}

void json_parser_flush_caches() {
    // Nop
}

}

#endif /* HAVE_JSONC */
