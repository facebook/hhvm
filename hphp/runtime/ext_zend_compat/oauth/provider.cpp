/*
+----------------------------------------------------------------------+
| See LICENSE file for further copyright information                   |
+----------------------------------------------------------------------+
| Authors: John Jawed <jawed@php.net>                                  |
|          Felipe Pena <felipe@php.net>                                |
|          Rasmus Lerdorf <rasmus@php.net>                             |
|          Tjerk Meesters <datibbaw@php.net>                           |
+----------------------------------------------------------------------+
*/
/* $Id: oauth.c,v 1.60 2009/05/16 15:46:09 jawed Exp $ */

#include "php_oauth.h"
#include "provider.h"

#if PHP_WIN32
# include <windows.h>
# include <Wincrypt.h>
#endif

#include "fcntl.h"
#include "SAPI.h"

static zend_object_handlers oauth_provider_obj_hndlrs;
static zend_class_entry *oauthprovider;

static inline void oauth_provider_set_param_member(zval *provider_obj, char *prop_name, zval *prop TSRMLS_DC) /* {{{ */
{
  zend_update_property(Z_OBJCE_P(provider_obj), provider_obj, prop_name, strlen(prop_name), prop TSRMLS_CC);
}
/* }}} */

static inline php_oauth_provider *fetch_sop_object(zval *obj TSRMLS_DC) /* {{{ */
{
  php_oauth_provider *sop = (php_oauth_provider *)zend_object_store_get_object(obj TSRMLS_CC);
  sop->this_ptr = obj;
  return sop;
}
/* }}} */

static int oauth_provider_set_default_required_params(HashTable *ht) /* {{{ */
{
  char *required_params[] = {"oauth_consumer_key", "oauth_signature", "oauth_signature_method", "oauth_nonce", "oauth_timestamp", "oauth_token", NULL};
  unsigned int idx = 0;

  do {
    zval *tmp;
    MAKE_STD_ZVAL(tmp);
    ZVAL_NULL(tmp);
    if(zend_hash_add(ht, required_params[idx], strlen(required_params[idx]) + 1, &tmp, sizeof(zval *), NULL)==FAILURE) {
      return FAILURE;
    }
    ++idx;
  } while(required_params[idx]);

  return SUCCESS;
}
/* }}} */

static int oauth_provider_remove_required_param(HashTable *ht, char *required_param) /* {{{ */
{
  zval **dest_entry;
  char *key;
  uint key_len;
  ulong num_key;
  HashPosition hpos;

  if(zend_hash_find(ht, required_param, strlen(required_param) + 1, (void **)&dest_entry)==FAILURE) {
    return FAILURE;
  } else {
    zend_hash_internal_pointer_reset_ex(ht, &hpos);
    do {
      if(zend_hash_get_current_key_ex(ht, &key, &key_len, &num_key, 0, &hpos)!=FAILURE) {
        if(!strcmp(key, required_param)) {
          zend_hash_del(ht, key, key_len);
          return SUCCESS;
        }
      }
    } while(zend_hash_move_forward_ex(ht, &hpos)==SUCCESS);
  }
  return FAILURE;
}
/* }}} */

static int oauth_provider_add_required_param(HashTable *ht, char *required_param) /* {{{ */
{
  zval *zparam, **dest_entry;

  if(zend_hash_find(ht, required_param, strlen(required_param) + 1, (void **)&dest_entry)==FAILURE) {
    MAKE_STD_ZVAL(zparam);
    ZVAL_NULL(zparam);
    if(zend_hash_add(ht, required_param, strlen(required_param) + 1, &zparam, sizeof(zval *), NULL)==FAILURE) {
      return FAILURE;
    }
  }
  return SUCCESS;
}
/* }}} */

static void oauth_provider_apply_custom_param(HashTable *ht, HashTable *custom) /* {{{ */
{
  HashPosition custompos;
  zval **entry;
  char *key;
  uint key_len;
  ulong num_key;

  zend_hash_internal_pointer_reset_ex(custom, &custompos);
  do {
    if (SUCCESS==zend_hash_get_current_data_ex(custom, (void**)&entry, &custompos) && HASH_KEY_IS_STRING==zend_hash_get_current_key_ex(custom, &key, &key_len, &num_key, 0, &custompos)) {
      if (IS_NULL==Z_TYPE_PP(entry)) {
        zend_hash_del(ht, key, key_len);
      } else {
        zend_hash_update(ht, key, key_len, entry, sizeof(zval **), NULL);
      }
    }
  } while (SUCCESS==zend_hash_move_forward_ex(custom, &custompos));
}
/* }}} */

static int oauth_provider_token_required(zval *provider_obj, char* uri TSRMLS_DC)
{
  zval *is_req_token_api;

  is_req_token_api = zend_read_property(Z_OBJCE_P(provider_obj), provider_obj, "request_token_endpoint", sizeof("request_token_endpoint") - 1, 1 TSRMLS_CC);

  if (!Z_BVAL_P(is_req_token_api)) {
    php_oauth_provider *sop;

    sop = fetch_sop_object(provider_obj TSRMLS_CC);
    /* do uri matching on the relative path */
    if (sop->endpoint_paths[OAUTH_PROVIDER_PATH_REQUEST]) {
      const char *reqtoken_path = sop->endpoint_paths[OAUTH_PROVIDER_PATH_REQUEST];
      int uri_matched = 0;

      if (reqtoken_path[0]=='/') {
        /* match against relative url */
        php_url *urlparts = php_url_parse_ex(uri, strlen(uri));
        uri_matched = urlparts && 0==strncmp(urlparts->path, reqtoken_path, strlen(reqtoken_path));
        php_url_free(urlparts);
      } else {
        /* match against full uri */
        uri_matched = 0==strncmp(uri, reqtoken_path, strlen(reqtoken_path));
      }

      /* token required if no match was found */
      if (uri_matched) {
        ZVAL_BOOL(is_req_token_api, 1);
        return 0;
      }
    }

    /* no matches, token required */
    return 1;
  }
  return 0;
}

static void oauth_provider_check_required_params(HashTable *required_params, HashTable *params, HashTable *missing_params TSRMLS_DC) /* {{{ */
{
  HashPosition hpos, reqhpos, paramhpos;
  zval **dest_entry, *param;
  char *key;
  ulong num_key;
  uint key_len;

  zend_hash_internal_pointer_reset_ex(required_params, &hpos);
  zend_hash_internal_pointer_reset_ex(params, &reqhpos);
  zend_hash_internal_pointer_reset_ex(missing_params, &paramhpos);
  do {
    if(zend_hash_get_current_key_ex(required_params, &key, &key_len, &num_key, 0, &hpos)==HASH_KEY_IS_STRING) {
      if(zend_hash_find(params, key, key_len, (void **)&dest_entry)==FAILURE) {
        MAKE_STD_ZVAL(param);
        ZVAL_STRING(param, key, 1);
        zend_hash_next_index_insert(missing_params, &param, sizeof(zval *), NULL);
      }
    }
  } while(zend_hash_move_forward_ex(required_params, &hpos)==SUCCESS);
}
/* }}} */

static void oauth_provider_set_std_params(zval *provider_obj, HashTable *sbs_vars TSRMLS_DC) /* {{{ */
{
  zval **dest_entry;

  if(!provider_obj || !sbs_vars) {
    return;
  }

  OAUTH_PROVIDER_SET_STD_PARAM(sbs_vars, OAUTH_PARAM_CONSUMER_KEY, OAUTH_PROVIDER_CONSUMER_KEY);
  OAUTH_PROVIDER_SET_STD_PARAM(sbs_vars, OAUTH_PARAM_TOKEN, OAUTH_PROVIDER_TOKEN);
  OAUTH_PROVIDER_SET_STD_PARAM(sbs_vars, OAUTH_PARAM_SIGNATURE, OAUTH_PROVIDER_SIGNATURE);
  OAUTH_PROVIDER_SET_STD_PARAM(sbs_vars, OAUTH_PARAM_NONCE, OAUTH_PROVIDER_NONCE);
  OAUTH_PROVIDER_SET_STD_PARAM(sbs_vars, OAUTH_PARAM_TIMESTAMP, OAUTH_PROVIDER_TIMESTAMP);
  OAUTH_PROVIDER_SET_STD_PARAM(sbs_vars, OAUTH_PARAM_VERSION, OAUTH_PROVIDER_VERSION);
  OAUTH_PROVIDER_SET_STD_PARAM(sbs_vars, OAUTH_PARAM_SIGNATURE_METHOD, OAUTH_PROVIDER_SIGNATURE_METHOD);
  OAUTH_PROVIDER_SET_STD_PARAM(sbs_vars, OAUTH_PARAM_CALLBACK, OAUTH_PROVIDER_CALLBACK);
  OAUTH_PROVIDER_SET_STD_PARAM(sbs_vars, OAUTH_PARAM_VERIFIER, OAUTH_PROVIDER_VERIFIER);
}
/* }}} */

static inline int oauth_provider_set_param_value(HashTable *ht, const char *key, zval **val) /* {{{ */
{
  ulong h;
  ulong key_len = 0;

  key_len = strlen(key);
  h = zend_hash_func(key, key_len+1);
  Z_ADDREF_P(*val);
  return zend_hash_quick_update(ht, key, key_len+1, h, val, sizeof(zval **), NULL);
}
/* }}} */

static int oauth_provider_parse_auth_header(php_oauth_provider *sop, char *auth_header TSRMLS_DC) /* {{{ */
{
  pcre_cache_entry *pce;
  zval *subpats = NULL, *return_value = NULL, **item_param = NULL, **current_param = NULL, **current_val = NULL;
  HashPosition hpos;
  /* the following regex is also used at http://oauth.googlecode.com/svn/code/php/OAuth.php to help ensure uniform behavior between libs, credit goes to the original author(s) */
  char *regex = "/(oauth_[a-z_-]*)=(?:\"([^\"]*)\"|([^,]*))/";

  if(!auth_header || strncasecmp(auth_header, "oauth", 4) || !sop) {
    return FAILURE;
  }
  /* pass "OAuth " */
  auth_header += 5;

  if ((pce = pcre_get_compiled_regex_cache(regex, sizeof(regex)-1 TSRMLS_CC)) == NULL) {
    return FAILURE;
  }

  MAKE_STD_ZVAL(return_value);
  ALLOC_INIT_ZVAL(subpats);

  php_pcre_match_impl(
    pce,
    auth_header,
    strlen(auth_header),
    return_value,
    subpats,
    1, /* global */
    1, /* use flags */
    2, /* PREG_SET_ORDER */
    0
    TSRMLS_CC
  );

  if (0==Z_LVAL_P(return_value)) {
    return FAILURE;
  }

  zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(subpats), &hpos);
  /* walk the oauth param names */
  do {
    if (SUCCESS==zend_hash_get_current_data_ex(Z_ARRVAL_P(subpats), (void **)&item_param, &hpos)) {
      zval *decoded_val;
      char *tmp;
      int decoded_len;
      /*
       * item = array(
       *   1 => param name
       *  2 => quoted value
       *  3 => unquoted value (defined if matched)
       * )
       */
      zend_hash_index_find(Z_ARRVAL_PP(item_param), 1, (void **)&current_param);

      if (FAILURE==zend_hash_index_find(Z_ARRVAL_PP(item_param), 3, (void**)&current_val)) {
        zend_hash_index_find(Z_ARRVAL_PP(item_param), 2, (void**)&current_val);
      }

      tmp = estrndup(Z_STRVAL_PP(current_val), Z_STRLEN_PP(current_val));
      decoded_len = php_url_decode(tmp, Z_STRLEN_PP(current_val));
      MAKE_STD_ZVAL(decoded_val);
      ZVAL_STRINGL(decoded_val, tmp, decoded_len, 0);

      if (oauth_provider_set_param_value(sop->oauth_params, Z_STRVAL_PP(current_param), &decoded_val)==FAILURE) {
        return FAILURE;
      }
      Z_DELREF_P(decoded_val);
    }
  } while (SUCCESS==zend_hash_move_forward_ex(Z_ARRVAL_P(subpats), &hpos));

  zval_ptr_dtor(&return_value);
  zval_ptr_dtor(&subpats);

  return SUCCESS;
}
/* }}} */

static void oauth_provider_register_cb(INTERNAL_FUNCTION_PARAMETERS, int type) /* {{{ */
{
  zend_fcall_info fci;
  zend_fcall_info_cache fci_cache;
  php_oauth_provider *sop;
  php_oauth_provider_fcall *cb;
  php_oauth_provider_fcall **tgt_cb;

  if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f", &fci, &fci_cache)==FAILURE) {
    return;
  }

  sop = fetch_sop_object(getThis() TSRMLS_CC);

  cb = (php_oauth_provider_fcall*) emalloc(sizeof(php_oauth_provider_fcall));
  cb->fcall_info = (zend_fcall_info*) emalloc(sizeof(zend_fcall_info));
  memcpy(cb->fcall_info, &fci, sizeof(zend_fcall_info));
  cb->fcall_info_cache = fci_cache;

  Z_ADDREF_P(cb->fcall_info->function_name);

  switch(type) {
    case OAUTH_PROVIDER_CONSUMER_CB:
      tgt_cb = &sop->consumer_handler;
      break;
    case OAUTH_PROVIDER_TOKEN_CB:
      tgt_cb = &sop->token_handler;
      break;
    case OAUTH_PROVIDER_TSNONCE_CB:
      tgt_cb = &sop->tsnonce_handler;
      break;
    default:
      php_error_docref(NULL TSRMLS_CC, E_ERROR, "Invalid callback type for OAuthProvider");
      return;
  }
  OAUTH_PROVIDER_FREE_CB((*tgt_cb));
  (*tgt_cb) = cb;
}
/* }}} */

static zval *oauth_provider_call_cb(INTERNAL_FUNCTION_PARAMETERS, int type) /* {{{ */
{
  php_oauth_provider *sop;
  php_oauth_provider_fcall *cb = NULL;
  zval *retval = NULL, *args, *pthis;
  char *errstr = "", *callable_name = NULL;
  zend_bool is_callable;

  pthis = getThis();
  sop = fetch_sop_object(pthis TSRMLS_CC);

  switch(type) {
    case OAUTH_PROVIDER_CONSUMER_CB:
      cb = sop->consumer_handler;
      errstr = "Consumer key/secret handler not specified, did you set a valid callback via OAuthProvider::consumerHandler()?";
      break;
    case OAUTH_PROVIDER_TOKEN_CB:
      cb = sop->token_handler;
      errstr = "Token handler not specified, did you set a valid callback via OAuthProvider::tokenHandler()?";
      break;
    case OAUTH_PROVIDER_TSNONCE_CB:
      cb = sop->tsnonce_handler;
      errstr = "Timestamp/nonce handler not specified, did you set a valid callback via OAuthProvider::timestampNonceHandler()?";
      break;
    default:
      php_error_docref(NULL TSRMLS_CC, E_ERROR, "Invalid callback type for OAuthProvider");
      return NULL;
  }

  if(!cb) {
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "%s", errstr);
    return NULL;
  }

  MAKE_STD_ZVAL(args);
  array_init(args);
  add_next_index_zval(args, pthis);
  Z_ADDREF_P(pthis);
  Z_ADDREF_P(args);

  errstr = NULL;
#if PHP_VERSION_ID < 50300
  is_callable = zend_is_callable_ex(cb->fcall_info->function_name, 0, &callable_name, NULL, NULL, NULL, NULL TSRMLS_CC);
#else
  is_callable = zend_is_callable_ex(cb->fcall_info->function_name, cb->fcall_info->object_ptr, IS_CALLABLE_CHECK_SILENT, &callable_name, NULL, &cb->fcall_info_cache, &errstr TSRMLS_CC);
#endif

  if (!is_callable) {
    if (errstr) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid callback %s, %s", callable_name, errstr);
      efree(errstr);
    }
  } else if (errstr) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", errstr);
    efree(errstr);
  }

  if (zend_fcall_info_call(cb->fcall_info, &cb->fcall_info_cache, &retval, args TSRMLS_CC)!=SUCCESS) {
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "Failed calling callback %s", callable_name);
  }
  zval_ptr_dtor(&args);
  efree(callable_name);

  return retval;
}
/* }}} */

static const char *oauth_provider_get_http_verb(TSRMLS_D) /* {{{ */
{
  zval **tmp;

  zend_is_auto_global("_SERVER", sizeof("_SERVER")-1 TSRMLS_CC);

  if(PG(http_globals)[TRACK_VARS_SERVER]) {
    if(zend_hash_find(HASH_OF(PG(http_globals)[TRACK_VARS_SERVER]), "REQUEST_METHOD", sizeof("REQUEST_METHOD"), (void **) &tmp)!=FAILURE || zend_hash_find(HASH_OF(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_METHOD", sizeof("HTTP_METHOD"), (void **) &tmp)!=FAILURE) {
      return Z_STRVAL_PP(tmp);
    }
  }
  return NULL;
}
/* }}} */

static char *oauth_provider_get_current_uri(TSRMLS_D)
{
  zval **host = NULL, **port = NULL, **uri = NULL, **proto = NULL, **https=NULL;

  zend_is_auto_global("_SERVER", sizeof("_SERVER")-1 TSRMLS_CC);

  zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_HOST", sizeof("HTTP_HOST"), (void**)&host);
  zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "SERVER_PORT", sizeof("SERVER_PORT"), (void**)&port);
  zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "REQUEST_URI", sizeof("REQUEST_URI"), (void**)&uri);
  zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_X_FORWARDED_PROTO", sizeof("HTTP_X_FORWARDED_PROTO"), (void **)&proto);
  zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "HTTPS", sizeof("HTTPS"), (void **)&https);

  if (host && port && uri)
  {
    char *tmp,*hostname,*colon_in_hostname;

    spprintf(&hostname, 0, "%s", Z_STRVAL_PP(host));
    colon_in_hostname=strrchr(hostname,':');
    if(colon_in_hostname && ((https && Z_LVAL_PP(port)==443) || (!https && Z_LVAL_PP(port)==80)))
    {
      *colon_in_hostname=0;
    }
    if(proto && Z_STRLEN_PP(proto))
    {
      spprintf(&tmp, 0, "%s://%s%s", Z_STRVAL_PP(proto), hostname, Z_STRVAL_PP(uri));
    }
    else if(https && strcasecmp(Z_STRVAL_PP(https),"off")!=0)
    {
      spprintf(&tmp, 0, "https://%s%s", hostname, Z_STRVAL_PP(uri));
    }
    else
    {
      spprintf(&tmp, 0, "http://%s%s", hostname, Z_STRVAL_PP(uri));
    }
    efree(hostname);
    return tmp;
  }

  return NULL;
}

/* {{{ proto void OAuthProvider::__construct()
   Instantiate a new OAuthProvider object */
SOP_METHOD(__construct)
{
  php_oauth_provider *sop;
  zval *params = NULL, *pthis = NULL, *auth_header = NULL, *apache_get_headers = NULL, *retval = NULL, **tmpzval = NULL, **item_param = NULL;
  char *authorization_header = NULL, *key = NULL;
  ulong num_key = 0, param_count = 0;
  HashPosition hpos;

  pthis = getThis();

  sop = fetch_sop_object(pthis TSRMLS_CC);

  /* XXX throw E_NOTICE if filter!='unsafe_raw' */
  if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|z", &params)==FAILURE) {
    soo_handle_error(NULL, OAUTH_ERR_INTERNAL_ERROR, "Failed to instantiate OAuthProvider", NULL, NULL TSRMLS_CC);
    return;
  }

  if (params && Z_TYPE_P(params)==IS_ARRAY) {
    param_count = zend_hash_num_elements(Z_ARRVAL_P(params));
  } else {
    param_count = 0;
  }
  if(!strcasecmp("cli", sapi_module.name) && !param_count) {
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "For the CLI sapi parameters must be set first via OAuthProvider::__construct(array(\"oauth_param\" => \"value\", ...))");
    return;
  }

  /* hashes for storing parameter info/checks */
  ALLOC_HASHTABLE(sop->oauth_params);
  zend_hash_init(sop->oauth_params, 0, NULL, ZVAL_PTR_DTOR, 0);
  ALLOC_HASHTABLE(sop->missing_params);
  zend_hash_init(sop->missing_params, 0, NULL, ZVAL_PTR_DTOR, 0);
  ALLOC_HASHTABLE(sop->required_params);
  zend_hash_init(sop->required_params, 0, NULL, ZVAL_PTR_DTOR, 0);
  ALLOC_HASHTABLE(sop->custom_params);
  zend_hash_init(sop->custom_params, 0, NULL, ZVAL_PTR_DTOR, 0);
  memset(sop->endpoint_paths, 0, sizeof(sop->endpoint_paths));

  sop->consumer_handler = NULL;
  sop->token_handler = NULL;
  sop->tsnonce_handler = NULL;
  sop->handle_errors = 1;

  oauth_provider_set_default_required_params(sop->required_params);

  zend_update_property_null(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_CONSUMER_KEY, sizeof(OAUTH_PROVIDER_CONSUMER_KEY)-1 TSRMLS_CC);
  zend_update_property_null(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_CONSUMER_SECRET, sizeof(OAUTH_PROVIDER_CONSUMER_SECRET)-1 TSRMLS_CC);
  zend_update_property_null(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_NONCE, sizeof(OAUTH_PROVIDER_NONCE)-1 TSRMLS_CC);
  zend_update_property_null(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_TOKEN, sizeof(OAUTH_PROVIDER_TOKEN)-1 TSRMLS_CC);
  zend_update_property_null(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_TOKEN_SECRET, sizeof(OAUTH_PROVIDER_TOKEN_SECRET)-1 TSRMLS_CC);
  zend_update_property_null(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_TIMESTAMP, sizeof(OAUTH_PROVIDER_TIMESTAMP)-1 TSRMLS_CC);
  zend_update_property_null(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_VERSION, sizeof(OAUTH_PROVIDER_VERSION)-1 TSRMLS_CC);
  zend_update_property_null(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_SIGNATURE_METHOD, sizeof(OAUTH_PROVIDER_SIGNATURE_METHOD)-1 TSRMLS_CC);
  zend_update_property_null(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_CALLBACK, sizeof(OAUTH_PROVIDER_CALLBACK)-1 TSRMLS_CC);

  zend_update_property_bool(Z_OBJCE_P(pthis), pthis, "request_token_endpoint", sizeof("request_token_endpoint")-1, 0 TSRMLS_CC);

  if(!param_count) {
    /* TODO: support NSAPI */
    /* mod_php */
    if(!strncasecmp(sapi_module.name, "apache", sizeof("apache") - 1)) {
      MAKE_STD_ZVAL(apache_get_headers);
      MAKE_STD_ZVAL(retval);
      ZVAL_STRING(apache_get_headers, "apache_request_headers", 0);

      if(zend_is_callable(apache_get_headers, 0, NULL OAUTH_IS_CALLABLE_CC)) {
        if(call_user_function(EG(function_table), NULL, apache_get_headers, retval, 0, NULL TSRMLS_CC)) {
          php_error_docref(NULL TSRMLS_CC, E_ERROR, "Failed to get HTTP Request headers");
        }
        if(SUCCESS == zend_hash_find(HASH_OF(retval), "Authorization", sizeof("Authorization"), (void **) &tmpzval)) {
          auth_header = *tmpzval;
          authorization_header = estrdup(Z_STRVAL_P(auth_header));
        } else if (SUCCESS==zend_hash_find(HASH_OF(retval), "authorization", sizeof("authorization"), (void **) &tmpzval)) {
          auth_header = *tmpzval;
          authorization_header = estrdup(Z_STRVAL_P(auth_header));
        } else {
          /* search one by one */
          zend_hash_internal_pointer_reset_ex(HASH_OF(retval), &hpos);
          do {
            uint key_len;

            if (FAILURE!=zend_hash_get_current_key_ex(HASH_OF(retval), &key, &key_len, &num_key, 0, &hpos) && key_len==sizeof("authorization") && 0==strcasecmp(key, "authorization") && SUCCESS==zend_hash_get_current_data_ex(HASH_OF(retval), (void**)&tmpzval, &hpos)) {
              auth_header = *tmpzval;
              authorization_header = estrdup(Z_STRVAL_P(auth_header));
              break;
            }
          } while (SUCCESS==zend_hash_move_forward_ex(HASH_OF(retval), &hpos));
        }
      } else {
        php_error_docref(NULL TSRMLS_CC, E_ERROR, "Failed to call apache_request_headers while running under the Apache SAPI");
      }
      FREE_ZVAL(apache_get_headers);
      zval_ptr_dtor(&retval);
    } else { /* not mod_php, look in _SERVER and _ENV for Authorization header */
      if(!zend_is_auto_global("_SERVER", sizeof("_SERVER") - 1 TSRMLS_CC) && !zend_is_auto_global("_ENV", sizeof("_ENV") - 1 TSRMLS_CC)) {
        return;
      }

      /* first look in _SERVER */
      if (!PG(http_globals)[TRACK_VARS_SERVER]
          || zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_AUTHORIZATION", sizeof("HTTP_AUTHORIZATION"), (void **) &tmpzval)==FAILURE) {
        /* well that didn't work out, so let's check out _ENV */
        if (!PG(http_globals)[TRACK_VARS_ENV]
            || zend_hash_find(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_ENV]), "HTTP_AUTHORIZATION", sizeof("HTTP_AUTHORIZATION"), (void **) &tmpzval)==FAILURE) {
          /* not found, [bf]ail */
          return;
        }
      }
      auth_header = *tmpzval;
      authorization_header = estrdup(Z_STRVAL_P(auth_header));
    }
    if (authorization_header) {
      int ret = oauth_provider_parse_auth_header(sop, authorization_header TSRMLS_CC);

      efree(authorization_header);

      if (FAILURE==ret) {
        soo_handle_error(NULL, OAUTH_SIGNATURE_METHOD_REJECTED, "Unknown signature method", NULL, NULL TSRMLS_CC);
        return;
      }
    }
  }
  /* let constructor params override any values that may have been found in auth headers */
  if (param_count) {
    zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(params), &hpos);
    do {
      if(zend_hash_get_current_key_ex(Z_ARRVAL_P(params), &key, NULL, &num_key, 0, &hpos)==HASH_KEY_IS_STRING) {
        if(zend_hash_get_current_data_ex(Z_ARRVAL_P(params), (void **)&item_param, &hpos)!=FAILURE) {
          if(oauth_provider_set_param_value(sop->oauth_params, key, item_param)==FAILURE) {
            return;
          }
        }
      }
    } while(zend_hash_move_forward_ex(Z_ARRVAL_P(params), &hpos)==SUCCESS);
  }
}
/* }}} */

/* {{{ proto void OAuthProvider::callConsumerHandler()
   calls the registered consumer key handler function */
SOP_METHOD(callconsumerHandler)
{
  OAUTH_PROVIDER_CALL_CB(INTERNAL_FUNCTION_PARAM_PASSTHRU, OAUTH_PROVIDER_CONSUMER_CB);
}
/* }}} */

/* {{{ proto void OAuthProvider::callTokenHandler()
   calls the registered token handler function */
SOP_METHOD(calltokenHandler)
{
  OAUTH_PROVIDER_CALL_CB(INTERNAL_FUNCTION_PARAM_PASSTHRU, OAUTH_PROVIDER_TOKEN_CB);
}
/* }}} */

/* {{{ proto void OAuthProvider::callTokenHandler()
   calls the registered token handler function */
SOP_METHOD(callTimestampNonceHandler)
{
  OAUTH_PROVIDER_CALL_CB(INTERNAL_FUNCTION_PARAM_PASSTHRU, OAUTH_PROVIDER_TSNONCE_CB);
}
/* }}} */

/* {{{ proto void OAuthProvider::consumerHandler(callback cb) */
SOP_METHOD(consumerHandler)
{
  oauth_provider_register_cb(INTERNAL_FUNCTION_PARAM_PASSTHRU, OAUTH_PROVIDER_CONSUMER_CB);
}
/* }}} */

/* {{{ proto void OAuthProvider::tokenHandler(callback cb) */
SOP_METHOD(tokenHandler)
{
  oauth_provider_register_cb(INTERNAL_FUNCTION_PARAM_PASSTHRU, OAUTH_PROVIDER_TOKEN_CB);
}
/* }}} */

/* {{{ proto void OAuthProvider::timestampNonceHandler(callback cb) */
SOP_METHOD(timestampNonceHandler)
{
  oauth_provider_register_cb(INTERNAL_FUNCTION_PARAM_PASSTHRU, OAUTH_PROVIDER_TSNONCE_CB);
}
/* }}} */

/* {{{ proto void OAuthProvider::isRequestTokenEndpoint(bool will_issue_request_token) */
SOP_METHOD(isRequestTokenEndpoint)
{
  zend_bool req_api = 0;
  zval *pthis;

  if(zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ob", &pthis, oauthprovider, &req_api)==FAILURE) {
    return;
  }

  fetch_sop_object(pthis TSRMLS_CC);

  zend_update_property_bool(Z_OBJCE_P(pthis), pthis, "request_token_endpoint", sizeof("request_token_endpoint") - 1, req_api TSRMLS_CC);
}
/* }}} */

/* {{{ proto void OAuthProvider::is2LeggedEndpoint(bool will_issue_request_token) */
SOP_METHOD(is2LeggedEndpoint)
{
  zend_bool req_api = 0;
  zval *pthis;

  if(zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Ob", &pthis, oauthprovider, &req_api)==FAILURE) {
    return;
  }

  fetch_sop_object(pthis TSRMLS_CC);

  zend_update_property_bool(Z_OBJCE_P(pthis), pthis, "request_token_endpoint", sizeof("request_token_endpoint") - 1, req_api TSRMLS_CC);
}
/* }}} */

SOP_METHOD(setRequestTokenPath)
{
  zval *pthis;
  php_oauth_provider *sop;
  char *path;
  int path_len;

  if (FAILURE==zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &pthis, oauthprovider, &path, &path_len)) {
    return;
  }

  sop = fetch_sop_object(pthis TSRMLS_CC);

  OAUTH_PROVIDER_SET_ENDPOINT(sop->endpoint_paths[OAUTH_PROVIDER_PATH_REQUEST], path)

  RETURN_TRUE;
}

/* {{{ proto void OAuthProvider::checkOAuthRequest([string url [, string request_method]]) */
SOP_METHOD(checkOAuthRequest)
{
  zval *retval = NULL, **param, *pthis, *token_secret = NULL, *consumer_secret, *req_signature, *sig_method;
  oauth_sig_context *sig_ctx = NULL;
  php_oauth_provider *sop;
  ulong missing_param_count = 0, mp_count = 1;
  char additional_info[512] = "", *uri = NULL, *sbs = NULL, *signature = NULL, *current_uri = NULL;
  const char *http_verb = NULL;
  HashPosition hpos;
  HashTable *sbs_vars = NULL;
  int http_verb_len = 0, uri_len = 0, is_token_required = 0;

  if(zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "O|ss", &pthis, oauthprovider, &uri, &uri_len, &http_verb, &http_verb_len)==FAILURE) {
    return;
  }

  sop = fetch_sop_object(pthis TSRMLS_CC);

  if(!http_verb_len) {
    http_verb = oauth_provider_get_http_verb(TSRMLS_C);
  }


  if(!http_verb) {
    php_error_docref(NULL TSRMLS_CC, E_ERROR, "Failed to detect HTTP method, set a HTTP method via OAuthProvider::checkOAuthRequest()");
    return;
  }

  ALLOC_HASHTABLE(sbs_vars);
  zend_hash_init(sbs_vars, 0, NULL, ZVAL_PTR_DTOR, 0);

  if(PG(http_globals)[TRACK_VARS_GET]) {
    zval *tmp_copy;
    zend_hash_merge(sbs_vars, HASH_OF(PG(http_globals)[TRACK_VARS_GET]), (copy_ctor_func_t)zval_add_ref, (void *)&tmp_copy, sizeof(zval *), 0);
  }
  if(PG(http_globals)[TRACK_VARS_POST]) {
    zval *tmp_copy;
    zend_hash_merge(sbs_vars, HASH_OF(PG(http_globals)[TRACK_VARS_POST]), (copy_ctor_func_t)zval_add_ref, (void *)&tmp_copy, sizeof(zval *), 0);
  }
  if(zend_hash_num_elements(sop->oauth_params)) {
    zval *tmp_copy;
    zend_hash_merge(sbs_vars, sop->oauth_params, (copy_ctor_func_t)zval_add_ref, (void *)&tmp_copy, sizeof(zval *), 0);
  }

  if (zend_hash_num_elements(sop->custom_params)) {
    /* apply custom params */
    oauth_provider_apply_custom_param(sbs_vars, sop->custom_params);
  }

  zend_hash_internal_pointer_reset_ex(sbs_vars, &hpos);

  /* set the standard stuff present in every request if its found in sbs_vars, IE if we find oauth_consumer_key, set $oauth->consumer_key */
  oauth_provider_set_std_params(pthis, sbs_vars TSRMLS_CC);

  if (!uri) {
    /* get current uri */
    uri = current_uri = oauth_provider_get_current_uri(TSRMLS_C);
  }

  /* if we are in an API which issues a request token, there are is no token handler called */
  if (!(is_token_required=oauth_provider_token_required(pthis, uri TSRMLS_CC))) {
    /* by default, oauth_token is required; remove from the required list */
    oauth_provider_remove_required_param(sop->required_params, "oauth_token");
  }

  oauth_provider_check_required_params(sop->required_params, sbs_vars, sop->missing_params TSRMLS_CC);

  missing_param_count = zend_hash_num_elements(sop->missing_params);
  if(missing_param_count) {
    zend_hash_internal_pointer_reset_ex(sop->missing_params, &hpos);
    do {
      if(zend_hash_get_current_data_ex(sop->missing_params, (void **)&param, &hpos)==SUCCESS) {
        snprintf(additional_info, 512, "%s%s%s", additional_info, Z_STRVAL_PP(param), (missing_param_count > 1 && missing_param_count!=mp_count++) ? "%26" : "");
      }
    } while(zend_hash_move_forward_ex(sop->missing_params, &hpos)==SUCCESS);
    soo_handle_error(NULL, OAUTH_PARAMETER_ABSENT, "Missing required parameters", NULL, additional_info TSRMLS_CC);
    FREE_ARGS_HASH(sbs_vars);
    OAUTH_PROVIDER_FREE_STRING(current_uri);
    return;
  }

  sig_method = zend_read_property(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_SIGNATURE_METHOD, sizeof(OAUTH_PROVIDER_SIGNATURE_METHOD) - 1, 1 TSRMLS_CC);
  do {
    if (sig_method && Z_STRLEN_P(sig_method)) {
      sig_ctx = oauth_create_sig_context(Z_STRVAL_P(sig_method));
      if (OAUTH_SIGCTX_TYPE_NONE!=sig_ctx->type) {
        break;
      }
      OAUTH_SIGCTX_FREE(sig_ctx);
    }
    soo_handle_error(NULL, OAUTH_SIGNATURE_METHOD_REJECTED, "Unknown signature method", NULL, NULL TSRMLS_CC);
    FREE_ARGS_HASH(sbs_vars);
    OAUTH_PROVIDER_FREE_STRING(current_uri);
    return;
  } while (0);

  do {
    long cb_res;

    retval = oauth_provider_call_cb(INTERNAL_FUNCTION_PARAM_PASSTHRU, OAUTH_PROVIDER_CONSUMER_CB);
    if (retval) {
      convert_to_long(retval);
      cb_res = Z_LVAL_P(retval);
      zval_ptr_dtor(&retval);

      if (OAUTH_OK!=cb_res) {
        soo_handle_error(NULL, cb_res, "Invalid consumer key", NULL, additional_info TSRMLS_CC);
        break;
      }
    } else if (EG(exception)) {
      /* pass exceptions */
      break;
    }

    if (is_token_required) {
      retval = oauth_provider_call_cb(INTERNAL_FUNCTION_PARAM_PASSTHRU, OAUTH_PROVIDER_TOKEN_CB);
      if (retval) {
        convert_to_long(retval);
        cb_res = Z_LVAL_P(retval);
        zval_ptr_dtor(&retval);

        if (OAUTH_OK!=cb_res) {
          soo_handle_error(NULL, cb_res, "Invalid token", NULL, additional_info TSRMLS_CC);
          break;
        }
      } else if (EG(exception)) {
        /* pass exceptions */
        break;
      }
    }

    retval = oauth_provider_call_cb(INTERNAL_FUNCTION_PARAM_PASSTHRU, OAUTH_PROVIDER_TSNONCE_CB);
    if (retval) {
      convert_to_long(retval);
      cb_res = Z_LVAL_P(retval);
      zval_ptr_dtor(&retval);

      if (OAUTH_OK!=cb_res) {
        soo_handle_error(NULL, cb_res, "Invalid nonce/timestamp combination", NULL, additional_info TSRMLS_CC);
        break;
      }
    } else if (EG(exception)) {
      /* pass exceptions */
      break;
    }

    /* now for the signature stuff */
    sbs = oauth_generate_sig_base(NULL, http_verb, uri, sbs_vars, NULL TSRMLS_CC);

    if (sbs) {
      consumer_secret = zend_read_property(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_CONSUMER_SECRET, sizeof(OAUTH_PROVIDER_CONSUMER_SECRET) - 1, 1 TSRMLS_CC);
      convert_to_string_ex(&consumer_secret);
      if (is_token_required) {
        token_secret = zend_read_property(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_TOKEN_SECRET, sizeof(OAUTH_PROVIDER_TOKEN_SECRET) - 1, 1 TSRMLS_CC);
        convert_to_string_ex(&token_secret);
      }
      signature = soo_sign(NULL, sbs, consumer_secret, token_secret, sig_ctx TSRMLS_CC);
    }

    req_signature = zend_read_property(Z_OBJCE_P(pthis), pthis, OAUTH_PROVIDER_SIGNATURE, sizeof(OAUTH_PROVIDER_SIGNATURE) - 1, 1 TSRMLS_CC);
    if (!signature || !Z_STRLEN_P(req_signature) || strcmp(signature, Z_STRVAL_P(req_signature))) {
      soo_handle_error(NULL, OAUTH_INVALID_SIGNATURE, "Signatures do not match", NULL, sbs TSRMLS_CC);
    }

    OAUTH_PROVIDER_FREE_STRING(sbs);
    OAUTH_PROVIDER_FREE_STRING(signature);
  } while (0);

  OAUTH_SIGCTX_FREE(sig_ctx);
  OAUTH_PROVIDER_FREE_STRING(current_uri);
  FREE_ARGS_HASH(sbs_vars);
}
/* }}} */

/* {{{ proto void OAuthProvider::addRequiredParameter(string $required_param) */
SOP_METHOD(addRequiredParameter)
{
  zval *pthis;
  char *required_param;
  php_oauth_provider *sop;
  ulong req_param_len;

  if(zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &pthis, oauthprovider, &required_param, &req_param_len)==FAILURE) {
    return;
  }

  sop = fetch_sop_object(pthis TSRMLS_CC);

  if(oauth_provider_add_required_param(sop->required_params, required_param)==SUCCESS) {
    RETURN_TRUE;
  }

  RETURN_FALSE;
}
/* }}} */

/* {{{ proto void OAuthProvider::setParam(string $key, mixed $val) */
SOP_METHOD(setParam)
{
  zval *pthis, *param_val = NULL;
  char *param_key;
  ulong param_key_len;
  php_oauth_provider *sop;

  if (FAILURE==zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os|z/", &pthis, oauthprovider, &param_key, &param_key_len, &param_val)) {
    return;
  }

  sop = fetch_sop_object(pthis TSRMLS_CC);

  if (!param_val) {
    RETURN_BOOL(SUCCESS==zend_hash_del(sop->custom_params, param_key, param_key_len+1));
  } else {
    Z_ADDREF_P(param_val);

    RETURN_BOOL(SUCCESS==zend_hash_add(sop->custom_params, param_key, param_key_len+1, &param_val, sizeof(zval **), NULL));
  }
}
/* }}} */

/* {{{ proto void OAuthProvider::removeRequiredParameter(string $required_param) */
SOP_METHOD(removeRequiredParameter)
{
  zval *pthis;
  char *required_param;
  php_oauth_provider *sop;
  ulong req_param_len;

  if(zend_parse_method_parameters(ZEND_NUM_ARGS() TSRMLS_CC, getThis(), "Os", &pthis, oauthprovider, &required_param, &req_param_len)==FAILURE) {
    return;
  }

  sop = fetch_sop_object(pthis TSRMLS_CC);

  if(oauth_provider_remove_required_param(sop->required_params, required_param)==SUCCESS) {
    RETURN_TRUE;
  }

  RETURN_FALSE;
}
/* }}} */

/* {{{ proto string OAuthProvider::generateToken(int $size[, bool $string = false]) */
SOP_METHOD(generateToken)
{
  long size, reaped = 0;
  int strong = 0;
  char *iv = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|b", &size, &strong)==FAILURE) {
    return;
  }

  if (size < 1 || size > INT_MAX) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot generate token with a size of less than 1 or greater than %d", INT_MAX);
    return;
  }

  iv = (char*) ecalloc(size+1, 1);

  do {
#if PHP_WIN32
/*
 * The Windows port has been ripped from the mcrypt extension; thanks guys! ;-)
 */
    HCRYPTPROV hCryptProv;
    BYTE *iv_b = (BYTE *) iv;

    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
      break;
    }
    if (!CryptGenRandom(hCryptProv, size, iv_b)) {
      break;
    }
    reaped = size;
#else
    int fd;

    fd = open(strong?"/dev/random":"/dev/urandom", O_RDONLY);
    if (fd < 0) {
      break;
    }
    while (reaped < size) {
      register int n;
      n = read(fd, iv + reaped, size - reaped);
      if (n < 0) {
        break;
      }
      reaped += n;
    }
    close(fd);
#endif
  } while (0);

  if (reaped < size) {
    if (strong) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not gather enough random data, falling back on rand()");
    }
    while (reaped < size) {
      iv[reaped++] = (char) (255.0 * php_rand(TSRMLS_C) / RAND_MAX);
    }
  }

  RETURN_STRINGL(iv, size, 0);
}
/* }}} */

/* {{{ proto void OAuthProvider::reportProblem(Exception $e) */
SOP_METHOD(reportProblem)
{
  zval *exception, *code, *sbs, *missing_params;
  zend_class_entry *ex_ce;
  zend_bool out_malloced = 0;
  char *out, *tmp_out, *http_header_line;
  size_t pr_len;
  ulong lcode;
  uint http_code;
  sapi_header_line ctr = {0};
  zend_bool send_headers = 1;

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 2)
  ex_ce = zend_exception_get_default();
#else
  ex_ce = zend_exception_get_default(TSRMLS_C);
#endif

  if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|b", &exception, ex_ce, &send_headers)==FAILURE) {
    return;
  }

  /* XXX good candidate for refactoring */
  code = zend_read_property(Z_OBJCE_P(exception), exception, "code", sizeof("code") - 1, 1 TSRMLS_CC);
  lcode = Z_LVAL_P(code);

  switch(lcode) {
    case OAUTH_BAD_TIMESTAMP:
      http_code = OAUTH_ERR_BAD_AUTH;
      out = "oauth_problem=timestamp_refused"; break;
    case OAUTH_BAD_NONCE:
      http_code = OAUTH_ERR_BAD_AUTH;
      out = "oauth_problem=nonce_used"; break;
    case OAUTH_CONSUMER_KEY_UNKNOWN:
      http_code = OAUTH_ERR_BAD_AUTH;
      out = "oauth_problem=consumer_key_unknown"; break;
    case OAUTH_CONSUMER_KEY_REFUSED:
      http_code = OAUTH_ERR_BAD_AUTH;
      out = "oauth_problem=consumer_key_refused"; break;
    case OAUTH_TOKEN_USED:
      http_code = OAUTH_ERR_BAD_AUTH;
      out = "oauth_problem=token_used"; break;
    case OAUTH_TOKEN_EXPIRED:
      http_code = OAUTH_ERR_BAD_AUTH;
      out = "oauth_problem=token_expired"; break;
    case OAUTH_TOKEN_REVOKED:
      http_code = OAUTH_ERR_BAD_AUTH;
      out = "oauth_problem=token_revoked"; break;
    case OAUTH_TOKEN_REJECTED:
      http_code = OAUTH_ERR_BAD_AUTH;
      out = "oauth_problem=token_rejected"; break;
    case OAUTH_VERIFIER_INVALID:
      http_code = OAUTH_ERR_BAD_AUTH;
      out = "oauth_problem=verifier_invalid"; break;
    case OAUTH_INVALID_SIGNATURE:
      http_code = OAUTH_ERR_BAD_AUTH;
      out = "oauth_problem=signature_invalid";
      sbs = zend_read_property(Z_OBJCE_P(exception), exception, "additionalInfo", sizeof("additionalInfo") - 1, 1 TSRMLS_CC);
      if (sbs && IS_NULL!=Z_TYPE_P(sbs)) {
        convert_to_string_ex(&sbs);
        if(Z_STRLEN_P(sbs)) {
          pr_len = Z_STRLEN_P(sbs) + strlen(out) + sizeof("&debug_sbs=");
          tmp_out = (char*) emalloc(pr_len);
          /* sbs url encoded so XSS shouldn't be an issue here */
          snprintf(tmp_out, pr_len, "%s&debug_sbs=%s", out, Z_STRVAL_P(sbs));
          out = tmp_out;
          out_malloced = 1;
        }
      }
      break;
    case OAUTH_SIGNATURE_METHOD_REJECTED:
      http_code = OAUTH_ERR_BAD_REQUEST;
      out = "oauth_problem=signature_method_rejected"; break;
    case OAUTH_PARAMETER_ABSENT:
      http_code = OAUTH_ERR_BAD_REQUEST;
      out = "oauth_problem=parameter_absent";
      missing_params = zend_read_property(Z_OBJCE_P(exception), exception, "additionalInfo", sizeof("additionalInfo") - 1, 1 TSRMLS_CC);
      if(missing_params) {
        convert_to_string_ex(&missing_params);
        if(Z_STRLEN_P(missing_params)) {
          pr_len = Z_STRLEN_P(missing_params) + strlen(out) + sizeof("&oauth_parameters_absent=");
          tmp_out = (char*) emalloc(pr_len);
          snprintf(tmp_out, pr_len, "%s&oauth_parameters_absent=%s", out, Z_STRVAL_P(missing_params));
          out = tmp_out;
          out_malloced = 1;
        }
      }
      break;
    default:
      http_code = OAUTH_ERR_INTERNAL_ERROR;
      out = (char*) emalloc(48);
      snprintf(out, 48, "oauth_problem=unknown_problem&code=%d", lcode);
      out_malloced = 1;
  }

  ZVAL_STRINGL(return_value, out, strlen(out), 1);

  if(send_headers) {
    if(http_code==OAUTH_ERR_BAD_REQUEST) {
      http_header_line = "HTTP/1.1 400 Bad Request";
    } else {
      http_header_line = "HTTP/1.1 401 Unauthorized";
    }

    ctr.line = http_header_line;
    ctr.line_len = strlen(http_header_line);
    ctr.response_code = http_code;

    sapi_header_op(SAPI_HEADER_REPLACE, &ctr TSRMLS_CC);
  }

  if(out_malloced) {
    efree(out);
  }
}
/* }}} */

static void oauth_provider_free_storage(void *obj TSRMLS_DC) /* {{{ */
{
  php_oauth_provider *sop;

  sop = (php_oauth_provider *)obj;

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3)
  if (sop->zo.guards) {
    zend_hash_destroy(sop->zo.guards);
    FREE_HASHTABLE(sop->zo.guards);
  }
  if (sop->zo.properties) {
    zend_hash_destroy(sop->zo.properties);
    FREE_HASHTABLE(sop->zo.properties);
  }
#else
  zend_object_std_dtor(&sop->zo TSRMLS_CC);
#endif

  OAUTH_PROVIDER_FREE_FCALL_INFO(sop->consumer_handler);
  OAUTH_PROVIDER_FREE_FCALL_INFO(sop->token_handler);
  OAUTH_PROVIDER_FREE_FCALL_INFO(sop->tsnonce_handler);
  FREE_ARGS_HASH(sop->missing_params);
  FREE_ARGS_HASH(sop->oauth_params);
  FREE_ARGS_HASH(sop->required_params);
  FREE_ARGS_HASH(sop->custom_params);

  OAUTH_PROVIDER_FREE_STRING(sop->endpoint_paths[OAUTH_PROVIDER_PATH_REQUEST]);
  OAUTH_PROVIDER_FREE_STRING(sop->endpoint_paths[OAUTH_PROVIDER_PATH_ACCESS]);
  OAUTH_PROVIDER_FREE_STRING(sop->endpoint_paths[OAUTH_PROVIDER_PATH_AUTH]);

  efree(sop);
}
/* }}} */

static zend_object_value oauth_provider_register(php_oauth_provider *soo TSRMLS_DC) /* {{{ */
{
  zend_object_value rv;

  rv.handle = zend_objects_store_put(soo, (zend_objects_store_dtor_t)zend_objects_destroy_object, oauth_provider_free_storage, NULL TSRMLS_CC);
  rv.handlers = (zend_object_handlers *)&oauth_provider_obj_hndlrs;
  return rv;
}

static php_oauth_provider* oauth_provider_new(zend_class_entry *ce TSRMLS_DC) /* {{{ */
{
  php_oauth_provider *nos;
#ifndef ZEND_ENGINE_2_4
  zval *tmp;
#endif

  nos = (php_oauth_provider*) ecalloc(1, sizeof(php_oauth_provider));

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3)
  ALLOC_HASHTABLE(nos->zo.properties);
  zend_hash_init(nos->zo.properties, 0, NULL, ZVAL_PTR_DTOR, 0);

  nos->zo.ce = ce;
  nos->zo.guards = NULL;
#else
  zend_object_std_init(&nos->zo, ce TSRMLS_CC);
#ifdef ZEND_ENGINE_2_4
  object_properties_init(&nos->zo, ce);
#else
  zend_hash_copy(nos->zo.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
#endif
#endif

  return nos;
}

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_provider__construct, 0, 0, 0)
ZEND_ARG_INFO(0, params_array)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_provider_noparams, 0, 0, 0)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_provider_check, 0, 0, 0)
ZEND_ARG_INFO(0, uri)
ZEND_ARG_INFO(0, method)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_provider_handler, 0, 0, 1)
ZEND_ARG_INFO(0, function_name)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_provider_reportproblem, 0, 0, 1)
ZEND_ARG_INFO(0, oauthexception)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_provider_req_token, 0, 0, 1)
ZEND_ARG_INFO(0, params_array)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_provider_set_req_param, 0, 0, 1)
ZEND_ARG_INFO(0, req_params)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_provider_set_param, 0, 0, 1)
ZEND_ARG_INFO(0, param_key)
ZEND_ARG_INFO(0, param_val)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_provider_set_path, 0, 0, 1)
ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_provider_generate_token, 0, 0, 1)
ZEND_ARG_INFO(0, size)
ZEND_ARG_INFO(0, strong)
ZEND_END_ARG_INFO()

static zend_function_entry oauth_provider_methods[] = { /* {{{ */
    SOP_ME(__construct,      arginfo_oauth_provider__construct,    ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    SOP_ME(consumerHandler,  arginfo_oauth_provider_handler,    ZEND_ACC_PUBLIC)
    SOP_ME(tokenHandler,  arginfo_oauth_provider_handler,    ZEND_ACC_PUBLIC)
    SOP_ME(timestampNonceHandler,  arginfo_oauth_provider_handler,    ZEND_ACC_PUBLIC)
    SOP_ME(callconsumerHandler,  arginfo_oauth_provider_noparams,    ZEND_ACC_PUBLIC)
    SOP_ME(calltokenHandler,  arginfo_oauth_provider_noparams,    ZEND_ACC_PUBLIC)
    SOP_ME(callTimestampNonceHandler,  arginfo_oauth_provider_noparams,    ZEND_ACC_PUBLIC)
    SOP_ME(checkOAuthRequest,  arginfo_oauth_provider_check,    ZEND_ACC_PUBLIC)
    SOP_ME(isRequestTokenEndpoint,  arginfo_oauth_provider_req_token,    ZEND_ACC_PUBLIC)
    SOP_ME(setRequestTokenPath,  arginfo_oauth_provider_set_path,  ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    SOP_ME(addRequiredParameter,  arginfo_oauth_provider_set_req_param,    ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    SOP_ME(reportProblem,  arginfo_oauth_provider_reportproblem,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
    SOP_ME(setParam,     arginfo_oauth_provider_set_param,    ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    SOP_ME(removeRequiredParameter,  arginfo_oauth_provider_set_req_param,    ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
    SOP_ME(generateToken,    arginfo_oauth_provider_generate_token,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
    PHP_MALIAS(oauthprovider,  is2LeggedEndpoint, isRequestTokenEndpoint, arginfo_oauth_provider_req_token, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

static zend_object_value oauth_provider_create_object(zend_class_entry *ce TSRMLS_DC) /* {{{ */
{
  php_oauth_provider *oprovider;

  oprovider = oauth_provider_new(ce TSRMLS_CC);
  return oauth_provider_register(oprovider TSRMLS_CC);
}
/* }}} */

extern int oauth_provider_register_class(TSRMLS_D) /* {{{ */
{
  zend_class_entry osce;

  INIT_CLASS_ENTRY(osce, "OAuthProvider", oauth_provider_methods);
  osce.create_object = oauth_provider_create_object;

  oauthprovider = zend_register_internal_class(&osce TSRMLS_CC);
  memcpy(&oauth_provider_obj_hndlrs, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  return SUCCESS;
}
/* }}} */

/**
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: t
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4 noexpandtab
 */
