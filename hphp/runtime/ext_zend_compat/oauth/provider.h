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

/* $Id: php_oauth.h,v 1.18 2009/05/10 06:40:59 jawed Exp $ */
#ifndef PHP_OAUTH_P_H
#define PHP_OAUTH_P_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define OAUTH_PROVIDER_COPY_ZVAL_FROM_PZVAL(dest, src) \
  MAKE_STD_ZVAL(dest); \
  *dest = *src; \
  zval_copy_ctor(dest);

#define OAUTH_PROVIDER_COPY_HASH_FROM_PZVAL(dest, src) \
  if(dest) { zval_ptr_dtor(&dest); } \
  OAUTH_PROVIDER_COPY_ZVAL_FROM_PZVAL(dest, src, 0)

#define OAUTH_PROVIDER_CALL_CB(pt, m) \
  zval *retval = NULL; \
  retval = oauth_provider_call_cb(pt, m); \
  COPY_PZVAL_TO_ZVAL(*return_value, retval); \
  zval_ptr_dtor(&retval);


#define OAUTH_PROVIDER_FREE_FCALL_INFO(o) \
  if(o) { \
    if(o->fcall_info->function_name) { zval_ptr_dtor(&o->fcall_info->function_name); } \
    efree(o->fcall_info); \
    efree(o); \
  }

#define OAUTH_PROVIDER_FREE_STRING(a) \
  if(a) { \
    efree(a); \
    a = NULL; \
  }

#define OAUTH_PROVIDER_FREE_CB(c) \
  if(c) { \
    OAUTH_PROVIDER_FREE_FCALL_INFO(c); \
  } \

#define OAUTH_PROVIDER_CHECK_PARAMS(s, r) \
  if(oauth_provider_check_sapi(s, TRUE TSRMLS_CC)) { \
    if(r) {\
      RETURN_FALSE\
    } \
  } \

#define OAUTH_PROVIDER_SET_PARAM(tgt_param, param, exp, val) \
  if(!strncasecmp(param, exp, strlen(exp))) { \
    tgt_param = val;\
    return SUCCESS;\
  }

#define OAUTH_PROVIDER_REQ_PARAM(a,b) \
  if(!a) { \
    oauth_provider_add_missing_param(sop,a,b);\
  }

#define OAUTH_PROVIDER_SET_PARAM_VALUE(ht,k,m,v) \
  zend_hash_update(ht, k, strlen(k) + 1, (void**)v, Z_STRLEN_PP(v) + 1, NULL)

#define OAUTH_PROVIDER_SET_STD_PARAM(h,k,m) \
  if(zend_hash_find(h, k, sizeof(k), (void **) &dest_entry)!=FAILURE) { \
    oauth_provider_set_param_member(provider_obj, m, *dest_entry TSRMLS_CC); \
  }

enum { OAUTH_PROVIDER_PATH_REQUEST, OAUTH_PROVIDER_PATH_ACCESS, OAUTH_PROVIDER_PATH_AUTH };

#define OAUTH_PROVIDER_SET_ENDPOINT(epp, path) \
  OAUTH_PROVIDER_FREE_STRING(epp)  \
  epp = estrdup(path);

typedef struct {
  zend_fcall_info *fcall_info;
  zend_fcall_info_cache fcall_info_cache;
} php_oauth_provider_fcall;

typedef struct {
  zend_object zo;
  HashTable *properties;
  HashTable *missing_params;
  /* oauth params which might be passed in requests */
  HashTable *oauth_params;
  HashTable *required_params;
  HashTable *custom_params;
  char *endpoint_paths[3];
  zval *zrequired_params;
  zval *this_ptr;
  php_oauth_provider_fcall *consumer_handler;
  php_oauth_provider_fcall *token_handler;
  php_oauth_provider_fcall *tsnonce_handler;
  unsigned int params_via_method;
  /* will ext/oauth set the proper header and error message? */
  unsigned int handle_errors;
} php_oauth_provider;

extern int oauth_provider_register_class(TSRMLS_D);

#define SOP_METHOD(func) PHP_METHOD(oauthprovider, func)
#define SOP_ME(func, arg_info, flags) PHP_ME(oauthprovider, func, arg_info, flags)
#ifndef OAUTH_PROVIDER_DEFAULT_METHODS
#define OAUTH_PROVIDER_DEFAULT_METHODS OAUTH_AUTH_TYPE_AUTHORIZATION | OAUTH_AUTH_TYPE_FORM | OAUTH_AUTH_TYPE_URI
#endif

#define OAUTH_PROVIDER_CONSUMER_CB (1<<0)
#define OAUTH_PROVIDER_TOKEN_CB (1<<1)
#define OAUTH_PROVIDER_TSNONCE_CB (1<<2)

#define OAUTH_PROVIDER_CONSUMER_KEY "consumer_key"
#define OAUTH_PROVIDER_CONSUMER_SECRET "consumer_secret"
#define OAUTH_PROVIDER_SIGNATURE "signature"
#define OAUTH_PROVIDER_SIGNATURE_METHOD "signature_method"
#define OAUTH_PROVIDER_TOKEN "token"
#define OAUTH_PROVIDER_TOKEN_SECRET "token_secret"
#define OAUTH_PROVIDER_NONCE "nonce"
#define OAUTH_PROVIDER_TIMESTAMP "timestamp"
#define OAUTH_PROVIDER_VERSION "version"
#define OAUTH_PROVIDER_CALLBACK "callback"
#define OAUTH_PROVIDER_VERIFIER "verifier"

#define OAUTH_BAD_NONCE (1<<2)
#define OAUTH_BAD_TIMESTAMP (1<<3)
#define OAUTH_CONSUMER_KEY_UNKNOWN (1<<4)
#define OAUTH_CONSUMER_KEY_REFUSED (1<<5)
#define OAUTH_INVALID_SIGNATURE (1<<6)
#define OAUTH_TOKEN_USED (1<<7)
#define OAUTH_TOKEN_EXPIRED (1<<8)
#define OAUTH_TOKEN_REVOKED (1<<9)
#define OAUTH_TOKEN_REJECTED (1<<10)
#define OAUTH_VERIFIER_INVALID (1<<11)
#define OAUTH_PARAMETER_ABSENT (1<<12)
#define OAUTH_SIGNATURE_METHOD_REJECTED (1<<13)


#endif
