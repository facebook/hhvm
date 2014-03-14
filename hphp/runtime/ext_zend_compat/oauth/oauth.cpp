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

/* $Id: oauth.c 325799 2012-05-24 21:07:51Z jawed $ */

#include "php_oauth.h"
#include "provider.h"

static zend_class_entry *soo_class_entry;
static zend_class_entry *soo_exception_ce;
static zend_object_handlers so_object_handlers;

static zend_object_value oauth_clone_obj(zval *this_ptr TSRMLS_DC);
static php_so_object* php_so_object_new(zend_class_entry *ce TSRMLS_DC);
static zend_object_value php_so_register_object(php_so_object *soo TSRMLS_DC);

static zend_object_value oauth_clone_obj(zval *this_ptr TSRMLS_DC) /* {{{ */
{
  php_so_object *old_obj = (php_so_object *)zend_object_store_get_object(this_ptr TSRMLS_CC);
  php_so_object *new_obj = php_so_object_new(old_obj->zo.ce TSRMLS_CC);
  zend_object_value new_ov = php_so_register_object(new_obj TSRMLS_CC);

  zend_objects_clone_members(&new_obj->zo, new_ov, &old_obj->zo, Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

  return new_ov;
}
/* }}} */

static int oauth_parse_str(char *params, zval *dest_array TSRMLS_DC) /* {{{ */
{
  char *res = NULL, *var, *val, *separator = NULL;
  char *strtok_buf = NULL;

  if (!params) {
    return FAILURE;
  }

  res = params;
  separator = (char *) estrdup(PG(arg_separator).input);
  var = php_strtok_r(res, separator, &strtok_buf);
  while (var) {
    val = strchr(var, '=');

    if (val) { /* have a value */
      int val_len;

      *val++ = '\0';
      php_url_decode(var, strlen(var));
      val_len = php_url_decode(val, strlen(val));
      val = estrndup(val, val_len);
    } else {
      int val_len;

      php_url_decode(var, strlen(var));
      val_len = 0;
      val = estrndup("", val_len);
    }
#if (PHP_MAJOR_VERSION >= 6)
    add_ascii_assoc_string(dest_array, var, val, 1);
#else
    add_assoc_string(dest_array, var, val, 1);
#endif
    efree(val);
    var = php_strtok_r(NULL, separator, &strtok_buf);
  }

  efree(separator);
  return SUCCESS;
}
/* }}} */

static inline php_so_object *fetch_so_object(zval *obj TSRMLS_DC) /* {{{ */
{
  php_so_object *soo = (php_so_object *)zend_object_store_get_object(obj TSRMLS_CC);

  soo->this_ptr = obj;

  return soo;
}
/* }}} */

static int so_set_response_args(HashTable *hasht, zval *data, zval *retarray TSRMLS_DC) /* {{{ */
{
  if (data && Z_TYPE_P(data) == IS_STRING) {
    ulong h = zend_hash_func(OAUTH_RAW_LAST_RES, sizeof(OAUTH_RAW_LAST_RES));

#if jawed_0
    /* don't need this till we fully implement error reporting ... */
    if (!onlyraw) {
      zend_hash_quick_update(hasht, OAUTH_ATTR_LAST_RES, sizeof(OAUTH_ATTR_LAST_RES), h, &arrayArg, sizeof(zval *), NULL);
    } else {
      zend_hash_quick_update(hasht, OAUTH_ATTR_LAST_RES, sizeof(OAUTH_ATTR_LAST_RES), h, &rawval, sizeof(zval *), NULL);

      h = zend_hash_func(OAUTH_RAW_LAST_RES, sizeof(OAUTH_RAW_LAST_RES));
      zend_hash_quick_update(hasht, OAUTH_RAW_LAST_RES, sizeof(OAUTH_RAW_LAST_RES), h, &rawval, sizeof(zval *), NULL);
    }
    return data;
#endif
    if (retarray) {
      char *res = NULL;

      res = estrndup(Z_STRVAL_P(data), Z_STRLEN_P(data));
      /* do not use oauth_parse_str here, we want the result to pass through input filters */
      sapi_module.treat_data(PARSE_STRING, res, retarray TSRMLS_CC);
    }

    return zend_hash_quick_update(hasht, OAUTH_RAW_LAST_RES, sizeof(OAUTH_RAW_LAST_RES), h, &data, sizeof(zval *), NULL);
  }
  return FAILURE;
}
/* }}} */

static zval *so_set_response_info(HashTable *hasht, zval *info) /* {{{ */
{
  ulong h = zend_hash_func(OAUTH_ATTR_LAST_RES_INFO, sizeof(OAUTH_ATTR_LAST_RES_INFO));

  if (zend_hash_quick_update(hasht, OAUTH_ATTR_LAST_RES_INFO, sizeof(OAUTH_ATTR_LAST_RES_INFO), h, &info, sizeof(zval *), NULL) != SUCCESS) {
    return NULL;
  }
  return info;
}
/* }}} */

static void so_object_free_storage(void *obj TSRMLS_DC) /* {{{ */
{
  php_so_object *soo;

  soo = (php_so_object *) obj;

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3)
  if (soo->zo.guards) {
    zend_hash_destroy(soo->zo.guards);
    FREE_HASHTABLE(soo->zo.guards);
  }
  if (soo->zo.properties) {
    zend_hash_destroy(soo->zo.properties);
    FREE_HASHTABLE(soo->zo.properties);
  }
#else
  zend_object_std_dtor(&soo->zo TSRMLS_CC);
#endif

  if (soo->lastresponse.c) {
    smart_str_free(&soo->lastresponse);
  }
  if (soo->headers_in.c) {
    smart_str_free(&soo->headers_in);
  }
  if (soo->headers_out.c) {
    smart_str_free(&soo->headers_out);
  }
  efree(obj);
}
/* }}} */

static zend_object_value php_so_register_object(php_so_object *soo TSRMLS_DC) /* {{{ */
{
  zend_object_value rv;

  rv.handle = zend_objects_store_put(soo, (zend_objects_store_dtor_t)zend_objects_destroy_object, so_object_free_storage, NULL TSRMLS_CC);
  rv.handlers = (zend_object_handlers *) &so_object_handlers;
  return rv;
}
/* }}} */

static php_so_object* php_so_object_new(zend_class_entry *ce TSRMLS_DC) /* {{{ */
{
  php_so_object *nos;
#ifndef ZEND_ENGINE_2_4
  zval *tmp;
#endif

  nos = (php_so_object*) ecalloc(1, sizeof(php_so_object));
  nos->signature = NULL;
  nos->timeout = 0;

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
/* }}} */

static zend_object_value new_so_object(zend_class_entry *ce TSRMLS_DC) /* {{{ */
{
  php_so_object *soo;

  soo = php_so_object_new(ce TSRMLS_CC);
  return php_so_register_object(soo TSRMLS_CC);
}
/* }}} */

void soo_handle_error(php_so_object *soo, long errorCode, char *msg, char *response, char *additional_info TSRMLS_DC) /* {{{ */
{
  zval *ex;
#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 2)
  zend_class_entry *dex = zend_exception_get_default(), *soox = soo_exception_ce;
#else
  zend_class_entry *dex = zend_exception_get_default(TSRMLS_C), *soox = soo_exception_ce;
#endif

  MAKE_STD_ZVAL(ex);
  object_init_ex(ex, soox);

  if (!errorCode) {
    php_error(E_WARNING, "caller did not pass an errorcode!");
  } else {
    zend_update_property_long(dex, ex, "code", sizeof("code")-1, errorCode TSRMLS_CC);
  }
  if (response) {
    zend_update_property_string(dex, ex, "lastResponse", sizeof("lastResponse")-1, response TSRMLS_CC);
  }
  if(soo && soo->debug && soo->debugArr) {
    zend_update_property(dex, ex, "debugInfo", sizeof("debugInfo") - 1, soo->debugArr TSRMLS_CC);
  }

  if(additional_info) {
    zend_update_property_string(dex, ex, "additionalInfo", sizeof("additionalInfo")-1, additional_info TSRMLS_CC);
  }

  zend_update_property_string(dex, ex, "message", sizeof("message")-1, msg TSRMLS_CC);
  zend_throw_exception_object(ex TSRMLS_CC);
}
/* }}} */

static void oauth_prop_hash_dtor(php_so_object *soo TSRMLS_DC) /* {{{ */
{
  HashTable *ht;

  ht = soo->properties;

  FREE_ARGS_HASH(ht);
}
/* }}} */

static char *soo_sign_hmac(php_so_object *soo, char *message, const char *cs, const char *ts, const oauth_sig_context *ctx TSRMLS_DC) /* {{{ */
{
  zval *args[4],*retval,*func;
  char *tret;
  int retlen;
  unsigned char *result;

  MAKE_STD_ZVAL(func);
  ZVAL_STRING(func, "hash_hmac", 2);

  if (!zend_is_callable(func, 0, NULL OAUTH_IS_CALLABLE_CC)) {
    FREE_ZVAL(func);
    soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "HMAC signature generation failed, is ext/hash installed?", NULL, NULL TSRMLS_CC);
    return NULL;
  }

  /* cs and ts would at best be empty, so this should be safe ;-) */
  spprintf(&tret, 0, "%s&%s", cs, ts);

  MAKE_STD_ZVAL(retval);
  MAKE_STD_ZVAL(args[0]);
  MAKE_STD_ZVAL(args[1]);
  MAKE_STD_ZVAL(args[2]);
  MAKE_STD_ZVAL(args[3]);

  ZVAL_STRING(args[0], ctx->hash_algo, 2);
  ZVAL_STRING(args[1], message, 0);
  ZVAL_STRING(args[2], tret, 2);
  ZVAL_BOOL(args[3], 1);

  call_user_function(EG(function_table), NULL, func, retval, 4, args TSRMLS_CC);
  result = php_base64_encode((unsigned char *)Z_STRVAL_P(retval), Z_STRLEN_P(retval), &retlen);

  efree(tret);
  zval_ptr_dtor(&retval);
  FREE_ZVAL(func);
  FREE_ZVAL(args[0]);
  FREE_ZVAL(args[1]);
  FREE_ZVAL(args[2]);
  FREE_ZVAL(args[3]);

  return (char *)result;
}
/* }}} */

static char *soo_sign_rsa(php_so_object *soo, char *message, const oauth_sig_context *ctx TSRMLS_DC)
{
  zval *args[3], *func, *retval;
  unsigned char *result;
  int retlen;

  /* check for empty private key */
  if (!ctx->privatekey) {
    return NULL;
  }

  MAKE_STD_ZVAL(func);
  ZVAL_STRING(func, "openssl_sign", 2);

  MAKE_STD_ZVAL(retval);
  MAKE_STD_ZVAL(args[0]);
  MAKE_STD_ZVAL(args[1]);

  /* TODO: add support for other algorithms instead of OPENSSL_ALGO_SHA1 */

  ZVAL_STRING(args[0], message, 0);
  /* Bug 17545 - segfault when zval_dtor is attempted on this argument */
  ZVAL_EMPTY_STRING(args[1]);
  /* args[1] is filled by function */
  args[2] = ctx->privatekey;

  call_user_function(EG(function_table), NULL, func, retval, 3, args TSRMLS_CC);

  if (Z_BVAL_P(retval)) {
    result = php_base64_encode((unsigned char *)Z_STRVAL_P(args[1]), Z_STRLEN_P(args[1]), &retlen);
    zval_ptr_dtor(&args[1]);
  } else {
    result = NULL;
  }

  zval_ptr_dtor(&retval);
  FREE_ZVAL(func);
  FREE_ZVAL(args[0]);

  return (char *)result;
}
/* }}} */

static char *soo_sign_plain(php_so_object *soo, const char *cs, const char *ts TSRMLS_DC) /* {{{ */
{
  char *tret;
  spprintf(&tret, 0, "%s&%s", cs, ts);
  return tret;
}
/* }}} */

oauth_sig_context *oauth_create_sig_context(const char *sigmethod)
{
  oauth_sig_context *ctx;

  OAUTH_SIGCTX_INIT(ctx);
  if (0==strcmp(sigmethod, OAUTH_SIG_METHOD_HMACSHA1)) {
    OAUTH_SIGCTX_HMAC(ctx, "sha1");
  } else if (0==strcmp(sigmethod, OAUTH_SIG_METHOD_HMACSHA256)) {
    OAUTH_SIGCTX_HMAC(ctx, "sha256");
  } else if (0==strcmp(sigmethod, OAUTH_SIG_METHOD_RSASHA1)) {
    OAUTH_SIGCTX_RSA(ctx, "sha1");
  } else if (0==strcmp(sigmethod, OAUTH_SIG_METHOD_PLAINTEXT)) {
    OAUTH_SIGCTX_PLAIN(ctx);
  }

  return ctx;
}

char *soo_sign(php_so_object *soo, char *message, zval *cs, zval *ts, const oauth_sig_context *ctx TSRMLS_DC)
{
  const char *csec = cs?Z_STRVAL_P(cs):"", *tsec = ts?Z_STRVAL_P(ts):"";

  if (OAUTH_SIGCTX_TYPE_HMAC==ctx->type) {
    return soo_sign_hmac(soo, message, csec, tsec, ctx TSRMLS_CC);
  } else if (OAUTH_SIGCTX_TYPE_RSA==ctx->type) {
    return soo_sign_rsa(soo, message, ctx TSRMLS_CC);
  } else if(OAUTH_SIGCTX_TYPE_PLAIN==ctx->type) {
    return soo_sign_plain(soo, csec, tsec TSRMLS_CC);
  }
  return NULL;
}

static inline zval **soo_get_property(php_so_object *soo, char *prop_name TSRMLS_DC) /* {{{ */
{
  size_t prop_len = 0;
  void *data_ptr;
  ulong h;

  prop_len = strlen(prop_name);
  h = zend_hash_func(prop_name, prop_len+1);

  if (zend_hash_quick_find(soo->properties, prop_name, prop_len+1, h, (void **)&data_ptr) == SUCCESS) {
    return (zval **)data_ptr;
  }
  return NULL;
}
/* }}} */

/* XXX for auth type, need to make sure that the auth type is actually supported before setting */
static inline int soo_set_property(php_so_object *soo, zval *prop, char *prop_name TSRMLS_DC) /* {{{ */
{
  size_t prop_len = 0;
  ulong h;

  prop_len = strlen(prop_name);
  h = zend_hash_func(prop_name, prop_len+1);

  return zend_hash_quick_update(soo->properties, prop_name, prop_len+1, h, (void *)&prop, sizeof(zval *), NULL);
}
/* }}} */

char *oauth_url_encode(const char *url, int url_len) /* {{{ */
{
  char *urlencoded = NULL, *ret;
  int out_len, ret_len;

  if (url) {
    if (url_len < 0) {
      url_len = strlen(url);
    }
    urlencoded = php_raw_url_encode(url, url_len, &out_len);
  }

  if (urlencoded) {
    ret = php_str_to_str_ex(urlencoded, out_len, "%7E", sizeof("%7E")-1, "~", sizeof("~")-1, &ret_len, 0, NULL);
    efree(urlencoded);
    return ret;
  }
  return NULL;
}
/* }}} */

char* oauth_http_encode_value(zval **v TSRMLS_DC)
{
  char *param_value = NULL;

  switch (Z_TYPE_PP(v)) {
    case IS_STRING:
      param_value = oauth_url_encode(Z_STRVAL_PP(v), Z_STRLEN_PP(v));
      break;
#if (PHP_MAJOR_VERSION >= 6)
    case IS_UNICODE:
      {
        char *temp;
        int temp_len;

        zend_unicode_to_string(UG(utf8_conv), &temp, &temp_len, Z_USTRVAL_PP(v), Z_USTRLEN_PP(v) TSRMLS_CC);
        param_value = oauth_url_encode(temp, temp_len);
        efree(temp);
      }
      break;
#endif
    default:
      SEPARATE_ZVAL(v);
      convert_to_string_ex(v);
      param_value = oauth_url_encode(Z_STRVAL_PP(v), Z_STRLEN_PP(v));
  }

  return param_value;
}

static int oauth_strcmp(zval *first, zval *second TSRMLS_DC)
{
  zval result;

  if (FAILURE==string_compare_function(&result, first, second TSRMLS_CC)) {
    return 0;
  }

  if (Z_LVAL(result) < 0) {
    return -1;
  } else if (Z_LVAL(result) > 0) {
    return 1;
  }

  return 0;
}

static int oauth_compare_value(const void *a, const void *b TSRMLS_DC)
{
  Bucket *f, *s;
  zval *first, *second;

  f = *(Bucket **)a;
  s = *(Bucket **)b;

  first = *(zval **)f->pData;
  second = *(zval **)s->pData;

  return oauth_strcmp(first, second TSRMLS_CC);
}

static int oauth_compare_key(const void *a, const void *b TSRMLS_DC)
{
  Bucket *f, *s;
  zval first, second;

  f = *(Bucket **)a;
  s = *(Bucket **)b;

  if (f->nKeyLength == 0) {
    ZVAL_LONG(&first, f->h);
  } else {
    ZVAL_STRINGL(&first, f->arKey, f->nKeyLength - 1, 0);
  }

  if (s->nKeyLength == 0) {
    ZVAL_LONG(&first, s->h);
  } else {
    ZVAL_STRINGL(&first, s->arKey, s->nKeyLength - 1, 0);
  }

  return oauth_strcmp(&first, &second TSRMLS_CC);
}

/* build url-encoded string from args, optionally starting with & */
int oauth_http_build_query(php_so_object *soo, smart_str *s, HashTable *args, zend_bool prepend_amp TSRMLS_DC)
{
  zval **cur_val;
  char *arg_key = NULL, *param_value;
  zend_hash_key_type cur_key;
  uint cur_key_len;
  int numargs = 0, hash_key_type, skip_append = 0, i, found;
  ulong num_index;
  HashPosition pos;
  smart_str keyname;

  smart_str_0(s);
  if (args) {
    if (soo && !soo->is_multipart) {
      for (zend_hash_internal_pointer_reset_ex(args, &pos);
         HASH_KEY_NON_EXISTANT!=(hash_key_type=zend_hash_get_current_key_ex(args, &cur_key, &cur_key_len, &num_index, 0, &pos));
         zend_hash_move_forward_ex(args, &pos)) {
        zend_hash_get_current_data_ex(args, (void **)&cur_val, &pos);
        if (hash_key_type == HASH_KEY_IS_STRING &&
          (*(ZEND_HASH_KEY_STRVAL(cur_key))=='@' && *(Z_STRVAL_PP(cur_val))=='@')) {
          soo->is_multipart = 1;
          break;
        }
      }
    }

    for (zend_hash_internal_pointer_reset_ex(args, &pos);
        HASH_KEY_NON_EXISTANT!=(hash_key_type=zend_hash_get_current_key_ex(args, &cur_key, &cur_key_len, &num_index, 0, &pos));
        zend_hash_move_forward_ex(args, &pos)) {
      zend_hash_get_current_data_ex(args, (void **)&cur_val, &pos);

      skip_append = 0;

      switch (hash_key_type) {
        case HASH_KEY_IS_STRING:
          if (soo && soo->is_multipart && strncmp(ZEND_HASH_KEY_STRVAL(cur_key), "oauth_", 6) != 0) {
            found = 0;
            for (i=0; i<soo->multipart_files_num; ++i) {
              if (0 == strcmp(soo->multipart_params[i], ZEND_HASH_KEY_STRVAL(cur_key))) {
                found = 1;
                break;
              }
            }

            if (found) {
              continue;
            }

            soo->multipart_files = (const char**) erealloc(soo->multipart_files, sizeof(char *) * (soo->multipart_files_num + 1));
            soo->multipart_params = (const char**) erealloc(soo->multipart_params, sizeof(char *) * (soo->multipart_files_num + 1));

            convert_to_string_ex(cur_val);
            soo->multipart_files[soo->multipart_files_num] = Z_STRVAL_PP(cur_val);
            soo->multipart_params[soo->multipart_files_num] = ZEND_HASH_KEY_STRVAL(cur_key);

            ++soo->multipart_files_num;
            /* we don't add multipart files to the params */
            skip_append = 1;
          } else {
            arg_key = oauth_url_encode(ZEND_HASH_KEY_STRVAL(cur_key), cur_key_len-1);
          }
          break;

        case HASH_KEY_IS_LONG:
          /* take value of num_index instead */
          arg_key = NULL;
          break;

#if (PHP_MAJOR_VERSION >= 6)
        case HASH_KEY_IS_UNICODE:
          {
            char *temp;
            int temp_len;

            zend_unicode_to_string(UG(utf8_conv), &temp, &temp_len, cur_key.u, cur_key_len-1 TSRMLS_CC);
            arg_key = oauth_url_encode(temp, temp_len);
            efree(temp);
          }
          break;
#endif
        default:
          continue;
      }

      if (skip_append) {
        continue;
      }

      INIT_SMART_STR(keyname);
      if (arg_key) {
        smart_str_appends(&keyname, arg_key);
        efree(arg_key);
      } else {
        smart_str_append_unsigned(&keyname, num_index);
      }
      if (IS_ARRAY==Z_TYPE_PP(cur_val)) {
        HashPosition val_pos;
        zval **val_cur_val;

        /* make shallow copy */
        SEPARATE_ZVAL(cur_val);
        /* sort array based on string comparison */
        zend_hash_sort(Z_ARRVAL_PP(cur_val), zend_qsort, oauth_compare_value, 1 TSRMLS_CC);

        /* traverse array */
        zend_hash_internal_pointer_reset_ex(Z_ARRVAL_PP(cur_val), &val_pos);
        while (SUCCESS==zend_hash_get_current_data_ex(Z_ARRVAL_PP(cur_val), (void **)&val_cur_val, &val_pos)) {
          if (prepend_amp) {
            smart_str_appendc(s, '&');
          }

          smart_str_append(s, &keyname);
          param_value = oauth_http_encode_value(val_cur_val TSRMLS_CC);
          if (param_value) {
            smart_str_appendc(s, '=');
            smart_str_appends(s, param_value);
            efree(param_value);
          }
          prepend_amp = TRUE;
          ++numargs;
          zend_hash_move_forward_ex(Z_ARRVAL_PP(cur_val), &val_pos);
        }
        /* clean up */
      } else {
        if (prepend_amp) {
          smart_str_appendc(s, '&');
        }
        smart_str_append(s, &keyname);
        param_value = oauth_http_encode_value(cur_val TSRMLS_CC);
        if (param_value) {
          smart_str_appendc(s, '=');
          smart_str_appends(s, param_value);
          efree(param_value);
        }
        prepend_amp = TRUE;
        ++numargs;
      }
      smart_str_free(&keyname);

      smart_str_0(s);
    }
  }
  return numargs;
}

/* retrieves parameter value from the _GET or _POST superglobal */
void get_request_param(char *arg_name, const char **return_val, int *return_len TSRMLS_DC)
{
  zval **ptr;
  if (
      (PG(http_globals)[TRACK_VARS_GET] && SUCCESS==zend_hash_find(HASH_OF(PG(http_globals)[TRACK_VARS_GET]), arg_name, strlen(arg_name)+1, (void**)&ptr) && IS_STRING==Z_TYPE_PP(ptr)) ||
      (PG(http_globals)[TRACK_VARS_POST] && SUCCESS==zend_hash_find(HASH_OF(PG(http_globals)[TRACK_VARS_POST]), arg_name, strlen(arg_name)+1, (void**)&ptr) && IS_STRING==Z_TYPE_PP(ptr))
     ) {
    *return_val = Z_STRVAL_PP(ptr);
    *return_len = Z_STRLEN_PP(ptr);
    return;
  }
  *return_val = NULL;
  *return_len = 0;
}

/*
 * This function does not currently care to respect parameter precedence, in the sense that if a common param is defined
 * in POST/GET or Authorization header, the precedence is defined by: OAuth Core 1.0 section 9.1.1
 */

char *oauth_generate_sig_base(php_so_object *soo, const char *http_method, const char *uri, HashTable *post_args, HashTable *extra_args TSRMLS_DC) /* {{{ */
{
  zval *params;
  char *query;
  char *s_port = NULL, *bufz = NULL, *sbs_query_part = NULL, *sbs_scheme_part = NULL;
  php_url *urlparts;
  smart_str sbuf = {0};

  urlparts = php_url_parse_ex(uri, strlen(uri));

  if (urlparts) {
    if (!urlparts->host || !urlparts->scheme) {
      soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid url when trying to build base signature string", NULL, NULL TSRMLS_CC);
      php_url_free(urlparts);
      return NULL;
    }
    smart_str_appends(&sbuf, urlparts->scheme);
    smart_str_appends(&sbuf, "://");
    smart_str_appends(&sbuf, urlparts->host);

    if (urlparts->port && ((!strcmp("http", urlparts->scheme) && OAUTH_HTTP_PORT != urlparts->port)
          || (!strcmp("https", urlparts->scheme) && OAUTH_HTTPS_PORT != urlparts->port))) {
      spprintf(&s_port, 0, "%d", urlparts->port);
      smart_str_appendc(&sbuf, ':');
      smart_str_appends(&sbuf, s_port);
      efree(s_port);
    }

    if (urlparts->path) {
      smart_str squery = {0};
      smart_str_appends(&sbuf, urlparts->path);
      smart_str_0(&sbuf);

      MAKE_STD_ZVAL(params);
      array_init(params);

      /* merge order = oauth_args - extra_args - query */
      if (post_args) {
        zval *tmp_copy;
        zend_hash_merge(Z_ARRVAL_P(params), post_args, (copy_ctor_func_t) zval_add_ref, (void *)&tmp_copy, sizeof(zval *), 0);
      }

      if (extra_args) {
        zval *tmp_copy;
        zend_hash_merge(Z_ARRVAL_P(params), extra_args, (copy_ctor_func_t) zval_add_ref, (void *)&tmp_copy, sizeof(zval *), 0);
      }

      if (urlparts->query) {
        query = estrdup(urlparts->query);
        oauth_parse_str(query, params TSRMLS_CC);
        efree(query);
      }

      /* remove oauth_signature if it's in the hash */
#if (PHP_MAJOR_VERSION >= 6)
      zend_ascii_hash_del(Z_ARRVAL_P(params), OAUTH_PARAM_SIGNATURE, sizeof(OAUTH_PARAM_SIGNATURE));
#else
      zend_hash_del(Z_ARRVAL_P(params), OAUTH_PARAM_SIGNATURE, sizeof(OAUTH_PARAM_SIGNATURE));
#endif

      /* exret2 = uksort(&exargs2[0], "strnatcmp") */
      zend_hash_sort(Z_ARRVAL_P(params), zend_qsort, oauth_compare_key, 0 TSRMLS_CC);

      oauth_http_build_query(soo, &squery, Z_ARRVAL_P(params), FALSE TSRMLS_CC);
      smart_str_0(&squery);
      zval_ptr_dtor(&params);

      sbs_query_part = oauth_url_encode(squery.c, squery.len);
      sbs_scheme_part = oauth_url_encode(sbuf.c, sbuf.len);

      spprintf(&bufz, 0, "%s&%s&%s", http_method, sbs_scheme_part, sbs_query_part?sbs_query_part:"");
      /* TODO move this into oauth_get_http_method()
         soo_handle_error(OAUTH_ERR_INTERNAL_ERROR, "Invalid auth type", NULL TSRMLS_CC);
         */
      if(sbs_query_part) {
        efree(sbs_query_part);
      }
      if(sbs_scheme_part) {
        efree(sbs_scheme_part);
      }
      smart_str_free(&squery);
    } else {
      /* Bug 22630 - throw exception if no path given */
      soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid path (perhaps you only specified the hostname? try adding a slash at the end)", NULL, NULL TSRMLS_CC);
      return NULL;
    }

    smart_str_free(&sbuf);

    php_url_free(urlparts);

    if(soo && soo->debug) {
      if(soo->debug_info->sbs) {
        efree(soo->debug_info->sbs);
      }
      soo->debug_info->sbs = bufz?estrdup(bufz):NULL;
    }
    return bufz;
  }
  return NULL;
}
/* }}} */

static void oauth_set_debug_info(php_so_object *soo TSRMLS_DC) {
  zval *debugInfo;
  char *tmp;

  if (soo->debug_info) {
    debugInfo = soo->debugArr;

    if(!debugInfo) {
      ALLOC_INIT_ZVAL(debugInfo);
      array_init(debugInfo);
    } else {
      FREE_ARGS_HASH(HASH_OF(debugInfo));
      array_init(debugInfo);
    }

    if(soo->debug_info->sbs) {
      add_assoc_string(debugInfo, "sbs", soo->debug_info->sbs, 1);
    }

    ADD_DEBUG_INFO(debugInfo, "headers_sent", soo->debug_info->headers_out, 1);
    ADD_DEBUG_INFO(debugInfo, "headers_recv", soo->headers_in, 1);
    ADD_DEBUG_INFO(debugInfo, "body_sent", soo->debug_info->body_out, 0);
    ADD_DEBUG_INFO(debugInfo, "body_recv", soo->debug_info->body_in, 0);
    ADD_DEBUG_INFO(debugInfo, "info", soo->debug_info->curl_info, 0);

    zend_update_property(soo_class_entry, soo->this_ptr, "debugInfo", sizeof("debugInfo") - 1, debugInfo TSRMLS_CC);

    soo->debugArr = debugInfo;
  } else {
    soo->debugArr = NULL;
  }
}

static int add_arg_for_req(HashTable *ht, const char *arg, const char *val TSRMLS_DC) /* {{{ */
{
  zval *varg;
  ulong h;

  MAKE_STD_ZVAL(varg);
  ZVAL_STRING(varg, (char *)val, 1);

  h = zend_hash_func((char *)arg, strlen(arg)+1);
  zend_hash_quick_update(ht, (char *)arg, strlen(arg)+1, h, &varg, sizeof(zval *), NULL);

  return SUCCESS;
}
/* }}} */

void oauth_add_signature_header(HashTable *request_headers, HashTable *oauth_args, smart_str *header TSRMLS_DC)
{
  smart_str sheader = {0};
  zend_bool prepend_comma = FALSE;

  zval **curval;
  char *param_name, *param_val;
  zend_hash_key_type cur_key;
  uint cur_key_len;
  ulong num_key;

  smart_str_appends(&sheader, "OAuth ");

  for (zend_hash_internal_pointer_reset(oauth_args);
      zend_hash_get_current_data(oauth_args, (void **)&curval) == SUCCESS;
      zend_hash_move_forward(oauth_args)) {
    zend_hash_get_current_key_ex(oauth_args, &cur_key, &cur_key_len, &num_key, 0, NULL);

    if (prepend_comma) {
      smart_str_appendc(&sheader, ',');
    }
    param_name = oauth_url_encode(ZEND_HASH_KEY_STRVAL(cur_key), cur_key_len-1);
    param_val = oauth_url_encode(Z_STRVAL_PP(curval), Z_STRLEN_PP(curval));

    smart_str_appends(&sheader, param_name);
    smart_str_appendc(&sheader, '=');
    smart_str_appends(&sheader, "\"");
    smart_str_appends(&sheader, param_val);
    smart_str_appends(&sheader, "\"");

    efree(param_name);
    efree(param_val);
    prepend_comma = TRUE;
  }
  smart_str_0(&sheader);

  if (!header) {
    add_arg_for_req(request_headers, "Authorization", sheader.c TSRMLS_CC);
  } else {
    smart_str_appends(header, sheader.c);
  }
  smart_str_free(&sheader);
}

#define HTTP_RESPONSE_CAAS(zvalpp, header, storkey) { \
  if (0==strncasecmp(Z_STRVAL_PP(zvalpp),header,sizeof(header)-1)) { \
    CAAS(storkey, (Z_STRVAL_PP(zvalpp)+sizeof(header)-1)); \
  } \
}

#define HTTP_RESPONSE_CAAD(zvalpp, header, storkey) { \
  if (0==strncasecmp(Z_STRVAL_PP(zvalpp),header,sizeof(header)-1)) { \
    CAAD(storkey, strtoul(Z_STRVAL_PP(zvalpp)+sizeof(header)-1,NULL,10)); \
  } \
}

#define HTTP_RESPONSE_CODE(zvalpp) \
  if (response_code < 0 && 0==strncasecmp(Z_STRVAL_PP(zvalpp),"HTTP/", 5) && Z_STRLEN_PP(zvalpp)>=12) { \
    response_code = strtol(Z_STRVAL_PP(zvalpp)+9, NULL, 10); \
    CAAL("http_code", response_code); \
  }

#define HTTP_RESPONSE_LOCATION(zvalpp) \
  if (0==strncasecmp(Z_STRVAL_PP(zvalpp), "Location: ", 10)) { \
    strlcpy(soo->last_location_header, Z_STRVAL_PP(zvalpp)+10, OAUTH_MAX_HEADER_LEN); \
  }

static long make_req_streams(php_so_object *soo, const char *url, const smart_str *payload, const char *http_method, HashTable *request_headers TSRMLS_DC) /* {{{ */
{
  php_stream_context *sc;
  zval zpayload, zmethod, zredirects, zerrign;
  long response_code = -1;
  php_stream *s;
  int set_form_content_type = 0;
  php_netstream_data_t *sock;
  struct timeval tv;
  int secs = 0;

#ifdef ZEND_ENGINE_2_4
  sc = php_stream_context_alloc(TSRMLS_C);
#else
  sc = php_stream_context_alloc();
#endif

  if (payload->len) {
    smart_str_0(payload);
    ZVAL_STRINGL(&zpayload, payload->c, payload->len, 0);
    php_stream_context_set_option(sc, "http", "content", &zpayload);
    /**
     * remember to set application/x-www-form-urlencoded content-type later on
     * lest the php streams guys come and beat you up
    */
    set_form_content_type = 1;
  }

  if (request_headers) {
    zval **cur_val, zheaders;
    zend_hash_key_type cur_key;
    uint cur_key_len;
    ulong num_key;
    smart_str sheaders = {0};
    int first = 1;

    for (zend_hash_internal_pointer_reset(request_headers);
        zend_hash_get_current_data(request_headers, (void **)&cur_val) == SUCCESS;
        zend_hash_move_forward(request_headers)) {
      /* check if a string based key is used */
      smart_str sheaderline = {0};
      switch (zend_hash_get_current_key_ex(request_headers, &cur_key, &cur_key_len, &num_key, 0, NULL)) {
#if (PHP_MAJOR_VERSION >= 6)
        case HASH_KEY_IS_UNICODE:
          {
            char *temp;
            int temp_len;

            zend_unicode_to_string(UG(utf8_conv), &temp, &temp_len, cur_key.u, cur_key_len-1 TSRMLS_CC);
            smart_str_appendl(&sheaderline, temp, temp_len);
            efree(temp);
          }
          break;
#endif
        case HASH_KEY_IS_STRING:
          smart_str_appendl(&sheaderline, ZEND_HASH_KEY_STRVAL(cur_key), cur_key_len-1);
          break;
        default:
          continue;
      }
      smart_str_0(&sheaderline);
      if (!strcasecmp(sheaderline.c,"content-type")) {
        set_form_content_type = 0;
      }
      smart_str_appends(&sheaderline, ": ");
      switch (Z_TYPE_PP(cur_val)) {
        case IS_STRING:
          smart_str_appendl(&sheaderline, Z_STRVAL_PP(cur_val), Z_STRLEN_PP(cur_val));
          break;
#if (PHP_MAJOR_VERSION >= 6)
        case IS_UNICODE:
          {
            char *temp;
            int temp_len;

            zend_unicode_to_string(UG(utf8_conv), &temp, &temp_len, Z_USTRVAL_PP(cur_val), Z_USTRLEN_PP(cur_val) TSRMLS_CC);
            smart_str_appendl(&sheaderline, temp, temp_len);
            efree(temp);
          }
          break;
#endif
        default:
          smart_str_free(&sheaderline);
          continue;
      }
      if (!first) {
        smart_str_appends(&sheaders, "\r\n");
      } else {
        first = 0;
      }
      smart_str_append(&sheaders, &sheaderline);
      smart_str_free(&sheaderline);
    }
    if (set_form_content_type) {
      /* still need to add our own content-type? */
      if (!first) {
        smart_str_appends(&sheaders, "\r\n");
      }
      smart_str_appends(&sheaders, "Content-Type: application/x-www-form-urlencoded");
    }
    if (sheaders.len) {
      smart_str_0(&sheaders);
      ZVAL_STRINGL(&zheaders, sheaders.c, sheaders.len, 0);
      php_stream_context_set_option(sc, "http", "header", &zheaders);
      if (soo->debug) {
        smart_str_append(&soo->debug_info->headers_out, &sheaders);
      }
            zval_dtor(&zheaders);
    }
  }
  /* set method */
  ZVAL_STRING(&zmethod, (char*)http_method, 2);
  php_stream_context_set_option(sc, "http", "method", &zmethod);
  /* set maximum redirects; who came up with the ridiculous logic of <= 1 means no redirects ?? */
  ZVAL_LONG(&zredirects, 1L);
  php_stream_context_set_option(sc, "http", "max_redirects", &zredirects);
  /* using special extension to treat redirects as regular document (requires patch in php) */
  ZVAL_BOOL(&zerrign, TRUE);
  php_stream_context_set_option(sc, "http", "ignore_errors", &zerrign);

  smart_str_free(&soo->lastresponse);
  smart_str_free(&soo->headers_in);

  if ((s = php_stream_open_wrapper_ex((char*)url, "rb", REPORT_ERRORS | ENFORCE_SAFE_MODE, NULL, sc))) {
    zval *info;
    char *buf;
    size_t rb;

    ALLOC_INIT_ZVAL(info);
    array_init(info);

    CAAS("url", url);

    if (s->wrapperdata) {
      zval **tmp;

      zend_hash_internal_pointer_reset(Z_ARRVAL_P(s->wrapperdata));
      while (SUCCESS == zend_hash_get_current_data(Z_ARRVAL_P(s->wrapperdata), (void **)&tmp)) {
        smart_str_appendl(&soo->headers_in, Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp));
        smart_str_appends(&soo->headers_in, "\r\n");
        HTTP_RESPONSE_CODE(tmp);
        HTTP_RESPONSE_LOCATION(tmp);
        HTTP_RESPONSE_CAAS(tmp, "Content-Type: ", "content_type");
        HTTP_RESPONSE_CAAD(tmp, "Content-Length: ", "download_content_length");
        zend_hash_move_forward(Z_ARRVAL_P(s->wrapperdata));
      }
      if (HTTP_IS_REDIRECT(response_code) && soo->last_location_header) {
        CAAS("redirect_url", soo->last_location_header);
      }
    }

    if(soo->timeout) {
      sock = (php_netstream_data_t*)s->abstract;
      secs = soo->timeout / 1000;
      tv.tv_sec = secs;
      tv.tv_usec = ((soo->timeout - (secs * 1000)) * 1000) % 1000000;
      sock->timeout = tv;
    }

    if ((rb = php_stream_copy_to_mem(s, &buf, PHP_STREAM_COPY_ALL, 0)) > 0) {
      smart_str_appendl(&soo->lastresponse, buf, rb);
      pefree(buf, 0);
    }
    smart_str_0(&soo->lastresponse);
    smart_str_0(&soo->headers_in);

    CAAD("size_download", rb);
    CAAD("size_upload", payload->len);

    so_set_response_info(soo->properties, info);

    php_stream_close(s);
  } else {
    char *bufz;

    spprintf(&bufz, 0, "making the request failed (%s)", "dunno why");
    soo_handle_error(soo, -1, bufz, soo->lastresponse.c, NULL TSRMLS_CC);
    efree(bufz);
  }

  if(soo->debug) {
    smart_str_append(&soo->debug_info->body_in, &soo->lastresponse);
    smart_str_append(&soo->debug_info->body_out, payload);
  }

  return response_code;
}
/* }}} */

#if OAUTH_USE_CURL
static size_t soo_read_response(char *ptr, size_t size, size_t nmemb, void *ctx) /* {{{ */
{
  uint relsize;
  php_so_object *soo = (php_so_object *)ctx;

  relsize = size * nmemb;
  smart_str_appendl(&soo->lastresponse, ptr, relsize);

  return relsize;
}
/* }}} */

int oauth_debug_handler(CURL *ch, curl_infotype type, char *data, size_t data_len, void *ctx) /* {{{ */
{
  php_so_debug *sdbg;
  char *z_data = NULL;
  smart_str *dest;

  if(data_len > 1 && data[0]=='\r' && data[1]=='\n') { /* ignore \r\n */
    return 0;
  }

  sdbg = (php_so_debug *)ctx;
  z_data = (char*) emalloc(data_len + 2);
  memset(z_data, 0, data_len + 2);
  memcpy(z_data, data, data_len);
  z_data[data_len] = '\0';

  switch(type) {
    case CURLINFO_TEXT:
      dest = &sdbg->curl_info;
      break;
    case CURLINFO_HEADER_OUT:
      dest = &sdbg->headers_out;
      break;
    case CURLINFO_DATA_IN:
      dest = &sdbg->body_in;
      break;
    case CURLINFO_DATA_OUT:
      dest = &sdbg->body_out;
      break;
    default:
      dest = NULL;
  }

  if(dest) {
    smart_str_appends(dest, z_data);
  }
  efree(z_data);

  return 0;
}
/* }}} */

static size_t soo_read_header(void *ptr, size_t size, size_t nmemb, void *ctx)
{
  char *header;
  size_t hlen, vpos = sizeof("Location:") - 1;
  php_so_object *soo;

  header = (char *)ptr;
  hlen = nmemb * size;
  soo = (php_so_object *)ctx;

  /* handle Location header */
  if (hlen > vpos && 0==strncasecmp(header, "Location:", vpos)) {
    size_t eol = hlen;
    /* find value start */
    while (vpos != eol && ' '==header[vpos]) {
      ++vpos;
    }
    /* POST: vpos == eol OR vpos < eol => value start found */
    while (vpos != eol && strchr("\r\n\0", header[eol - 1])) {
      --eol;
    }
    /* POST: vpos == eol OR vpos < eol => value end found */
    if (vpos != eol) {
      if (eol - vpos >= OAUTH_MAX_HEADER_LEN) {
        eol = vpos + OAUTH_MAX_HEADER_LEN - 1;
      }
      /* POST: eol - vpos <= OAUTH_MAX_HEADER_LEN */
      strncpy(soo->last_location_header, header + vpos, eol - vpos);
    }
    soo->last_location_header[eol - vpos] = '\0';
  }
  if(strncasecmp(header, "\r\n", 2)) {
    smart_str_appendl(&soo->headers_in, header, hlen);
  }
  return hlen;
}

long make_req_curl(php_so_object *soo, const char *url, const smart_str *payload, const char *http_method, HashTable *request_headers TSRMLS_DC) /* {{{ */
{
  CURLcode cres, ctres, crres;
  CURL *curl;
  struct curl_slist *curl_headers = NULL;
  long l_code, response_code = -1;
  double d_code;
  zval *info, **zca_info, **zca_path, **cur_val;
  char *s_code, *content_type = NULL, *bufz = NULL;
  uint cur_key_len, sslcheck;
  ulong num_key;
  smart_str sheader = {0};
  zend_hash_key_type cur_key;

  zca_info = soo_get_property(soo, OAUTH_ATTR_CA_INFO TSRMLS_CC);
  zca_path = soo_get_property(soo, OAUTH_ATTR_CA_PATH TSRMLS_CC);
  sslcheck = soo->sslcheck;

  curl = curl_easy_init();

  if (request_headers) {
    for (zend_hash_internal_pointer_reset(request_headers);
        zend_hash_get_current_data(request_headers, (void **)&cur_val) == SUCCESS;
        zend_hash_move_forward(request_headers)) {
      /* check if a string based key is used */
      switch (zend_hash_get_current_key_ex(request_headers, &cur_key, &cur_key_len, &num_key, 0, NULL)) {
#if (PHP_MAJOR_VERSION >= 6)
        case HASH_KEY_IS_UNICODE:
          {
            char *temp;
            int temp_len;

            zend_unicode_to_string(UG(utf8_conv), &temp, &temp_len, cur_key.u, cur_key_len-1 TSRMLS_CC);
            smart_str_appendl(&sheader, temp, temp_len);
            efree(temp);
          }
          break;
#endif
        case HASH_KEY_IS_STRING:
          smart_str_appendl(&sheader, ZEND_HASH_KEY_STRVAL(cur_key), cur_key_len-1);
          break;
        default:
          continue;
      }
      smart_str_appends(&sheader, ": ");
      switch (Z_TYPE_PP(cur_val)) {
        case IS_STRING:
          smart_str_appendl(&sheader, Z_STRVAL_PP(cur_val), Z_STRLEN_PP(cur_val));
          break;
#if (PHP_MAJOR_VERSION >= 6)
        case IS_UNICODE:
        {
          char *temp;
          int temp_len;

          zend_unicode_to_string(UG(utf8_conv), &temp, &temp_len, Z_USTRVAL_PP(cur_val), Z_USTRLEN_PP(cur_val) TSRMLS_CC);
          smart_str_appendl(&sheader, temp, temp_len);
          efree(temp);
        }
        break;
#endif
        default:
          smart_str_free(&sheader);
          continue;
      }

      smart_str_0(&sheader);
      curl_headers = curl_slist_append(curl_headers, sheader.c);
      smart_str_free(&sheader);
    }
  }

  if(soo->is_multipart) {
    struct curl_httppost *ff = NULL;
    struct curl_httppost *lf = NULL;
    int i;

    for(i=0; i < soo->multipart_files_num; i++) {
      char *type, *filename, *postval;

      /* swiped from ext/curl/interface.c to help with consistency */
      postval = estrdup(soo->multipart_files[i]);

      if (postval[0] == '@' && soo->multipart_params[i][0] == '@') {
        /* :< (chomp) @ */
        ++soo->multipart_params[i];
        ++postval;

        if((type = php_memnstr(postval, ";type=", sizeof(";type=") - 1, postval + strlen(soo->multipart_files[i]) - 1))) {
          *type = '\0';
        }
        if((filename = php_memnstr(postval, ";filename=", sizeof(";filename=") - 1, postval + strlen(soo->multipart_files[i]) - 1))) {
          *filename = '\0';
        }

        /* open_basedir check */
        if(php_check_open_basedir(postval TSRMLS_CC)) {
          char *em;
          spprintf(&em, 0, "failed to open file for multipart request: %s", postval);
          soo_handle_error(soo, -1, em, NULL, NULL TSRMLS_CC);
          efree(em);
          return 1;
        }

        curl_formadd(&ff, &lf,
               CURLFORM_COPYNAME, soo->multipart_params[i],
               CURLFORM_NAMELENGTH, (long)strlen(soo->multipart_params[i]),
               CURLFORM_FILENAME, filename ? filename + sizeof(";filename=") - 1 : soo->multipart_files[i],
               CURLFORM_CONTENTTYPE, type ? type + sizeof(";type=") - 1 : "application/octet-stream",
               CURLFORM_FILE, postval,
               CURLFORM_END);
      } else {
        curl_formadd(&ff, &lf,
               CURLFORM_COPYNAME, soo->multipart_params[i],
               CURLFORM_NAMELENGTH, (long)strlen(soo->multipart_params[i]),
               CURLFORM_COPYCONTENTS, postval,
               CURLFORM_CONTENTSLENGTH, (long)strlen(postval),
               CURLFORM_END);
      }
    }

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, ff);
  } else if (payload->len) {
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload->c);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload->len);
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);

  /* the fetch method takes precedence so figure it out after we've added the OAuth params */
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, http_method);

  /* Disable sending the 100 Expect header for POST requests */
  /* Other notes: if there is a redirect the POST becomes a GET request, see curl_easy_setopt(3) and the CURLOPT_POSTREDIR option for more information */
  curl_headers = curl_slist_append(curl_headers, "Expect:");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_headers);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, OAUTH_USER_AGENT);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, soo_read_response);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, soo);
  if(sslcheck == OAUTH_SSLCHECK_NONE) {
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
  } else {
    if (!(sslcheck & OAUTH_SSLCHECK_HOST)) {
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    }
    if (!(sslcheck & OAUTH_SSLCHECK_PEER)) {
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    }
    if(zca_path && Z_STRLEN_PP(zca_path)) {
      curl_easy_setopt(curl, CURLOPT_CAPATH, Z_STRVAL_PP(zca_path));
    }
    if(zca_info && Z_STRLEN_PP(zca_info)) {
      curl_easy_setopt(curl, CURLOPT_CAINFO, Z_STRVAL_PP(zca_info));
    }
  }
  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, soo_read_header);
  curl_easy_setopt(curl, CURLOPT_WRITEHEADER, soo);
  if(soo->debug) {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
  }
#if defined(ZTS)
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
#endif

#if LIBCURL_VERSION_NUM >= 0x071304
  curl_easy_setopt(curl, CURLOPT_PROTOCOLS, OAUTH_PROTOCOLS_ALLOWED);
#endif

#if LIBCURL_VERSION_NUM > 0x071002
  if(soo->timeout) {
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, soo->timeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, soo->timeout);
  }
#endif

  smart_str_free(&soo->lastresponse);
  smart_str_free(&soo->headers_in);

  if(soo->debug) {
    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, oauth_debug_handler);
    curl_easy_setopt(curl, CURLOPT_DEBUGDATA, soo->debug_info);
  }

  cres = curl_easy_perform(curl);

  smart_str_0(&soo->lastresponse);
  smart_str_0(&soo->headers_in);

  if (curl_headers) {
    curl_slist_free_all(curl_headers);
  }

  if (CURLE_OK == cres) {
    ctres = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);
    crres = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    if (CURLE_OK == crres && ctres == CURLE_OK) {
      ALLOC_INIT_ZVAL(info);
      array_init(info);

      CAAL("http_code", response_code);

      if (HTTP_IS_REDIRECT(response_code) && soo->last_location_header) {
        CAAS("redirect_url", soo->last_location_header);
      }

      if (content_type != NULL) {
        CAAS("content_type", content_type);
      }
      if (curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &s_code) == CURLE_OK) {
        CAAS("url", s_code);
      }

      if (curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &l_code) == CURLE_OK) {
        CAAL("header_size", l_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &l_code) == CURLE_OK) {
        CAAL("request_size", l_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_FILETIME, &l_code) == CURLE_OK) {
        CAAL("filetime", l_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_SSL_VERIFYRESULT, &l_code) == CURLE_OK) {
        CAAL("ssl_verify_result", l_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_REDIRECT_COUNT, &l_code) == CURLE_OK) {
        CAAL("redirect_count", l_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME,&d_code) == CURLE_OK) {
        CAAD("total_time", d_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &d_code) == CURLE_OK) {
        CAAD("namelookup_time", d_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &d_code) == CURLE_OK) {
        CAAD("connect_time", d_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &d_code) == CURLE_OK) {
        CAAD("pretransfer_time", d_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD, &d_code) == CURLE_OK) {
        CAAD("size_upload", d_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &d_code) == CURLE_OK) {
        CAAD("size_download", d_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &d_code) == CURLE_OK) {
        CAAD("speed_download", d_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &d_code) == CURLE_OK) {
        CAAD("speed_upload", d_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &d_code) == CURLE_OK) {
        CAAD("download_content_length", d_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_UPLOAD, &d_code) == CURLE_OK) {
        CAAD("upload_content_length", d_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &d_code) == CURLE_OK) {
        CAAD("starttransfer_time", d_code);
      }
      if (curl_easy_getinfo(curl, CURLINFO_REDIRECT_TIME, &d_code) == CURLE_OK) {
        CAAD("redirect_time", d_code);
      }

      CAAS("headers_recv", soo->headers_in.c);

      so_set_response_info(soo->properties, info);
    }
  } else {
    spprintf(&bufz, 0, "making the request failed (%s)", curl_easy_strerror(cres));
    soo_handle_error(soo, -1, bufz, soo->lastresponse.c, NULL TSRMLS_CC);
    efree(bufz);
  }
  curl_easy_cleanup(curl);
  return response_code;
}
/* }}} */
#endif

static void make_standard_query(HashTable *ht, php_so_object *soo TSRMLS_DC) /* {{{ */
{
  char *ts, *nonce;

  if (soo->timestamp) {
    ts = estrdup(soo->timestamp);
  } else {
    time_t now = time(NULL);
    /* XXX allow caller to set timestamp, if none set, then default to "now" */
    spprintf(&ts, 0, "%d", (int)now);
  }

  if (soo->nonce) {
    nonce = estrdup(soo->nonce);
  } else {
    struct timeval tv;
    int sec, usec;
    /* XXX maybe find a better way to generate a nonce... */
    gettimeofday((struct timeval *) &tv, (struct timezone *) NULL);
    sec = (int) tv.tv_sec;
    usec = (int) (tv.tv_usec % 0x100000);
    spprintf(&nonce, 0, "%ld%08x%05x%.8f", php_rand(TSRMLS_C), sec, usec, php_combined_lcg(TSRMLS_C) * 10);
  }

  add_arg_for_req(ht, OAUTH_PARAM_CONSUMER_KEY, Z_STRVAL_PP(soo_get_property(soo, OAUTH_ATTR_CONSUMER_KEY TSRMLS_CC)) TSRMLS_CC);
  add_arg_for_req(ht, OAUTH_PARAM_SIGNATURE_METHOD, Z_STRVAL_PP(soo_get_property(soo, OAUTH_ATTR_SIGMETHOD TSRMLS_CC)) TSRMLS_CC);

  add_arg_for_req(ht, OAUTH_PARAM_NONCE, nonce TSRMLS_CC);

  add_arg_for_req(ht, OAUTH_PARAM_TIMESTAMP, ts TSRMLS_CC);
  add_arg_for_req(ht, OAUTH_PARAM_VERSION, Z_STRVAL_PP(soo_get_property(soo, OAUTH_ATTR_OAUTH_VERSION TSRMLS_CC)) TSRMLS_CC);

  efree(ts); efree(nonce);
}
/* }}} */

/*
Returns the default http method to use with the different auth types
*/
static const char *oauth_get_http_method(php_so_object *soo, const char *http_method TSRMLS_DC) /* {{{ */
{
  long auth_type = Z_LVAL_PP(soo_get_property(soo, OAUTH_ATTR_AUTHMETHOD TSRMLS_CC));

  if (http_method) {
    /* TODO handle conflict with FORM auth and anything but POST or PUT */
    return http_method;
  }
  /* http method not explicitly given, choose default one */
  if (OAUTH_AUTH_TYPE_FORM==auth_type) {
    return OAUTH_HTTP_METHOD_POST;
  } else {
    return OAUTH_HTTP_METHOD_GET;
  }
}
/* }}} */

/*
Modifies (and returns) passed url parameter to be used for additional parameter appending
*/
static smart_str *http_prepare_url_concat(smart_str *surl) /* {{{ */
{
  smart_str_0(surl);
  if (!strchr(surl->c, '?')) {
    smart_str_appendc(surl, '?');
  } else {
    smart_str_appendc(surl, '&');
  }
  return surl;
}
/* }}} */

/*
Modifies passed url based on the location header that was received in the response headers, depending on whether the redirection was relative or absolute
*/
static void oauth_apply_url_redirect(smart_str *surl, const char *location) /* {{{ */
{
  php_url *urlparts;

  /* determine whether location is relative */
  if ('/'==*location) {
    urlparts = php_url_parse_ex(surl->c, surl->len);

    /* rebuild url from scratch */
    smart_str_free(surl);
    if (urlparts->scheme) {
      smart_str_appends(surl, urlparts->scheme);
      smart_str_appends(surl, "://");
    }
    if (urlparts->host) {
      smart_str_appends(surl, urlparts->host);
    }
    if (urlparts->port) {
      smart_str_appendc(surl, ':');
      smart_str_append_unsigned(surl, urlparts->port);
    }
    smart_str_appends(surl, location);

    php_url_free(urlparts);
  } else {
    smart_str_free(surl);
    smart_str_appends(surl, location);
  }
}
/* }}} */

/*
Prepares the request elements to be used by make_req(); this should allow for supporting streams in the future
*/
static long oauth_fetch(php_so_object *soo, const char *url, const char *method, zval *request_params, zval *request_headers, HashTable *init_oauth_args, int fetch_flags TSRMLS_DC) /* {{{ */
{
  char *sbs = NULL, *sig = NULL, *bufz = NULL;
  const char *final_http_method;
  zval **token = NULL, **cs;
  zval *ts = NULL, **token_secret = NULL;
  zval *zret;
  HashTable *oauth_args = NULL;
  HashTable *rargs = NULL, *rheaders = NULL;
  long http_response_code, auth_type;
  smart_str surl = {0}, payload = {0}, postdata = {0};
  uint is_redirect = FALSE, follow_redirects = 0, need_to_free_rheaders = 0;

  auth_type = Z_LVAL_PP(soo_get_property(soo, OAUTH_ATTR_AUTHMETHOD TSRMLS_CC));
  if(fetch_flags & OAUTH_OVERRIDE_HTTP_METHOD) {
    final_http_method = method;
  } else {
    final_http_method = oauth_get_http_method(soo, method ? method : OAUTH_HTTP_METHOD_POST TSRMLS_CC);

    if (OAUTH_AUTH_TYPE_FORM==auth_type && strcasecmp(final_http_method, OAUTH_HTTP_METHOD_POST)) {
      soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "auth type is set to HTTP POST with a non-POST http method, use setAuthType to put OAuth parameters somewhere else in the request", NULL, NULL TSRMLS_CC);
    }
  }


  if(!final_http_method) {
    final_http_method = "GET";
  }

  follow_redirects = soo->follow_redirects;
  soo->redirects = 0;
  soo->multipart_files = NULL;
  soo->multipart_params = NULL;
  soo->multipart_files_num = 0;
  soo->is_multipart = 0;

  /* request_params can be either NULL, a string containing arbitrary text (such as XML) or an array */
  if (request_params) {
    switch (Z_TYPE_P(request_params)) {
    case IS_ARRAY:
      rargs = HASH_OF(request_params);
      oauth_http_build_query(soo, &postdata, rargs, FALSE TSRMLS_CC);
      break;
    case IS_STRING:
      smart_str_appendl(&postdata, Z_STRVAL_P(request_params), Z_STRLEN_P(request_params));
      break;
    default:
      break;
    }
  }

  /* additional http headers can be passed */
  if (!request_headers) {
    ALLOC_HASHTABLE(rheaders);
    zend_hash_init(rheaders, 0, NULL, ZVAL_PTR_DTOR, 0);
    need_to_free_rheaders = 1;
  } else {
    rheaders = HASH_OF(request_headers);
  }

  /* initialize base url */
  smart_str_appends(&surl, url);

  do {
    /* initialize response code */
    http_response_code = -1;

    /* prepare oauth arguments to be signed */
    ALLOC_HASHTABLE(oauth_args);
    zend_hash_init(oauth_args, 0, NULL, ZVAL_PTR_DTOR, 0);

    /* an array can be passed to prime special oauth parameters */
    if (init_oauth_args) {
      zval *tmp_copy;
      /* populate oauth_args with given parameters */
      zend_hash_copy(oauth_args, init_oauth_args, (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *));
    }

    /* fill in the standard set of oauth parameters */
    make_standard_query(oauth_args, soo TSRMLS_CC);

    /* use token where applicable */
    if (fetch_flags & OAUTH_FETCH_USETOKEN) {
      token = soo_get_property(soo, OAUTH_ATTR_TOKEN TSRMLS_CC);
      if (token) {
        add_arg_for_req(oauth_args, OAUTH_PARAM_TOKEN, Z_STRVAL_PP(token) TSRMLS_CC);
      }
    }

    /* generate sig base on the semi-final url */
    smart_str_0(&surl);
    sbs = oauth_generate_sig_base(soo, final_http_method, surl.c, oauth_args, rargs TSRMLS_CC);
    if (!sbs) {
      FREE_ARGS_HASH(oauth_args);
      soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid protected resource url, unable to generate signature base string", NULL, NULL TSRMLS_CC);
      break;
    }

    cs = soo_get_property(soo, OAUTH_ATTR_CONSUMER_SECRET TSRMLS_CC);
    SEPARATE_ZVAL(cs);

    /* determine whether token should be used to sign the request */
    if (fetch_flags & OAUTH_FETCH_USETOKEN) {
      token_secret = soo_get_property(soo, OAUTH_ATTR_TOKEN_SECRET TSRMLS_CC);
      if (token_secret && Z_STRLEN_PP(token_secret) > 0) {
        ts = *token_secret;
      }
    }

    if(soo->signature) {
      efree(soo->signature);
    }
    /* sign the request */
    sig = soo_sign(soo, sbs, *cs, ts, soo->sig_ctx TSRMLS_CC);
    soo->signature = sig;
    efree(sbs);

    if(fetch_flags & OAUTH_FETCH_SIGONLY) {
      FREE_ARGS_HASH(oauth_args);
      smart_str_free(&surl);
      smart_str_free(&postdata);
      if(need_to_free_rheaders) {
        FREE_ARGS_HASH(rheaders);
      }
      return SUCCESS;
    }

    if (!sig) {
      FREE_ARGS_HASH(oauth_args);
      soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Signature generation failed", NULL, NULL TSRMLS_CC);
      break;
    }

    /* and add signature to the oauth parameters */
    add_arg_for_req(oauth_args, OAUTH_PARAM_SIGNATURE, sig TSRMLS_CC);

    if(fetch_flags & OAUTH_FETCH_HEADONLY) {
      INIT_SMART_STR(soo->headers_out);
      oauth_add_signature_header(rheaders, oauth_args, &soo->headers_out TSRMLS_CC);
      smart_str_0(&payload);
      FREE_ARGS_HASH(oauth_args);
      smart_str_free(&surl);
      smart_str_free(&postdata);
      if(need_to_free_rheaders) {
        FREE_ARGS_HASH(rheaders);
      }
      return SUCCESS;
    }

    if (!strcmp(final_http_method, OAUTH_HTTP_METHOD_GET)) {
      /* GET request means to extend the url, but not for redirects obviously */
      if (!is_redirect && postdata.len) {
        smart_str_append(http_prepare_url_concat(&surl), &postdata);
      }
    } else {
      /* otherwise populate post data */
      smart_str_append(&payload, &postdata);
    }

    switch (auth_type) {
      case OAUTH_AUTH_TYPE_FORM:
        /* append/set post data with oauth parameters */
        oauth_http_build_query(soo, &payload, oauth_args, payload.len TSRMLS_CC);
        smart_str_0(&payload);
        break;
      case OAUTH_AUTH_TYPE_URI:
        /* extend url request with oauth parameters */
        if (!is_redirect) {
          oauth_http_build_query(soo, http_prepare_url_concat(&surl), oauth_args, FALSE TSRMLS_CC);
        }
        /* TODO look into merging oauth parameters if they occur in the current url */
        break;
      case OAUTH_AUTH_TYPE_AUTHORIZATION:
        /* add http header with oauth parameters */
        oauth_add_signature_header(rheaders, oauth_args, NULL TSRMLS_CC);
        break;
    }

    /* finalize endpoint url */
    smart_str_0(&surl);

    if (soo->debug) {
      if(soo->debug_info->sbs) {
        FREE_DEBUG_INFO(soo->debug_info);
      }
      INIT_DEBUG_INFO(soo->debug_info);
    }

    switch (soo->reqengine) {
      case OAUTH_REQENGINE_STREAMS:
        http_response_code = make_req_streams(soo, surl.c, &payload, final_http_method, rheaders TSRMLS_CC);
        break;
#if OAUTH_USE_CURL
      case OAUTH_REQENGINE_CURL:
        http_response_code = make_req_curl(soo, surl.c, &payload, final_http_method, rheaders TSRMLS_CC);
        if (soo->multipart_files_num) {
          efree(soo->multipart_files);
          efree(soo->multipart_params);
          soo->multipart_files_num = 0;
          soo->is_multipart = 0;
        }
        break;
#endif
    }

    is_redirect = HTTP_IS_REDIRECT(http_response_code);

    if(soo->debug) {
      oauth_set_debug_info(soo TSRMLS_CC);
    }

    FREE_ARGS_HASH(oauth_args);
    smart_str_free(&payload);

    if (is_redirect) {
      if (follow_redirects) {
        if (soo->redirects >= OAUTH_MAX_REDIRS) {
          spprintf(&bufz, 0, "max redirections exceeded (max: %ld last redirect url: %s)", OAUTH_MAX_REDIRS, soo->last_location_header);
          MAKE_STD_ZVAL(zret);
          if (soo->lastresponse.len) {
            ZVAL_STRING(zret, soo->lastresponse.c, 1);
          } else {
            ZVAL_STRING(zret, "", 1);
          }
          so_set_response_args(soo->properties, zret, NULL TSRMLS_CC);
          soo_handle_error(soo, http_response_code, bufz, soo->lastresponse.c, NULL TSRMLS_CC);
          efree(bufz);
          /* set http_response_code to error value */
          http_response_code = -1;
          break;
        } else {
          ++soo->redirects;
          oauth_apply_url_redirect(&surl, soo->last_location_header);
          smart_str_0(&surl);
/* bug 22628; keep same method when following redirects
          final_http_method = OAUTH_HTTP_METHOD_GET;
*/
        }
      }
    } else if (http_response_code < 0) {
      /* exception would have been thrown already */
    } else if (http_response_code < 200 || http_response_code > 206) {
      spprintf(&bufz, 0, "Invalid auth/bad request (got a %ld, expected HTTP/1.1 20X or a redirect)", http_response_code);
      MAKE_STD_ZVAL(zret);
      if(soo->lastresponse.c) {
        ZVAL_STRING(zret, soo->lastresponse.c, 1);
      } else {
        ZVAL_STRING(zret, "", 1);
      }
      so_set_response_args(soo->properties, zret, NULL TSRMLS_CC);
      soo_handle_error(soo, http_response_code, bufz, soo->lastresponse.c, NULL TSRMLS_CC);
      efree(bufz);
      /* set http_response_code to error value */
      http_response_code = -1;
      break;
    } else {
      /* valid response, time to get out of this loop */
    }
  } while (is_redirect && follow_redirects);

  smart_str_free(&surl);
  smart_str_free(&postdata);
  if(need_to_free_rheaders) {
    FREE_ARGS_HASH(rheaders);
  }

  return http_response_code;
}
/* }}} */

SO_METHOD(setRSACertificate)
{
  char *key;
  int key_len;
  zval *args[1], *func, *retval;

  php_so_object *soo;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
    return;
  }

  MAKE_STD_ZVAL(func);
  ZVAL_STRING(func, "openssl_get_privatekey", 2);

  MAKE_STD_ZVAL(args[0]);
  ZVAL_STRINGL(args[0], key, key_len, 2);

  MAKE_STD_ZVAL(retval);

  call_user_function(EG(function_table), NULL, func, retval, 1, args TSRMLS_CC);

  FREE_ZVAL(args[0]);
  FREE_ZVAL(func);

  if (Z_TYPE_P(retval)==IS_RESOURCE) {
    OAUTH_SIGCTX_SET_PRIVATEKEY(soo->sig_ctx, retval);
    RETURN_TRUE;
  } else {
    zval_ptr_dtor(&retval);
    soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Could not parse RSA certificate", NULL, NULL TSRMLS_CC);
    return;
  }
}

/* {{{ proto string oauth_urlencode(string uri)
   URI encoding according to RFC 3986, note: is not utf8 capable until the underlying phpapi is */
PHP_FUNCTION(oauth_urlencode)
{
  int uri_len;
  char *uri;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &uri, &uri_len) == FAILURE) {
    return;
  }

  if (uri_len < 1) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid uri length (0)");
    RETURN_FALSE;
  }
  RETURN_STRING(oauth_url_encode(uri, uri_len), 0);
}
/* }}} */

/* {{{ proto string oauth_get_sbs(string http_method, string uri, array parameters)
   Get a signature base string */
PHP_FUNCTION(oauth_get_sbs)
{
  char *uri, *http_method, *sbs;
  int uri_len, http_method_len;
  zval *req_params = NULL;
  HashTable *rparams = NULL;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|a", &http_method, &http_method_len, &uri, &uri_len, &req_params) == FAILURE) {
    return;
  }

  if (uri_len < 1) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid uri length (0)");
    RETURN_FALSE;
  }

  if (http_method_len < 1) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid http method length (0)");
    RETURN_FALSE;
  }

  if (req_params) {
    rparams = HASH_OF(req_params);
  }

  if ((sbs = oauth_generate_sig_base(NULL, http_method, uri, NULL, rparams TSRMLS_CC))) {
    RETURN_STRING(sbs, 0);
  } else {
    RETURN_FALSE;
  }
}
/* }}} */

/* only hmac-sha1 is supported at the moment (it is the most common implementation), still need to lay down the ground work for supporting plaintext and others */

/* {{{ proto void OAuth::__construct(string consumer_key, string consumer_secret [, string signature_method, [, string auth_type ]])
   Instantiate a new OAuth object */
SO_METHOD(__construct)
{
  HashTable *hasht;
  char *ck, *cs, *sig_method = NULL;
  long auth_method = 0;
  zval *zck, *zcs, *zsm, *zam, *zver, *obj;
  int ck_len, cs_len, sig_method_len = 0;
  php_so_object *soo;

  obj = getThis();

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|sl", &ck, &ck_len, &cs, &cs_len, &sig_method, &sig_method_len, &auth_method) == FAILURE) {
    ZVAL_NULL(obj);
    return;
  }

  soo = fetch_so_object(obj TSRMLS_CC);

  if(!ck_len) {
    soo_handle_error(soo, -1, "The consumer key cannot be empty", NULL, NULL TSRMLS_CC);
    return;
  }
/*
  if(!cs_len) {
    soo_handle_error(soo, -1, "The consumer secret cannot be empty", NULL, NULL TSRMLS_CC);
    php_error(E_ERROR, "the consumer secret cannot be empty");
    return;
  }
*/
  memset(soo->last_location_header, 0, OAUTH_MAX_HEADER_LEN);
  soo->redirects = 0;
  soo->debug = 0;
  soo->debug_info = (php_so_debug*) emalloc(sizeof(php_so_debug));
  soo->debug_info->sbs = NULL;
  soo->debugArr = NULL;

  soo->nonce = NULL;
  soo->timestamp = NULL;
  soo->sig_ctx = NULL;

  INIT_DEBUG_INFO(soo->debug_info);

  INIT_SMART_STR(soo->headers_in);

  /* set default class members */
  zend_update_property_null(soo_class_entry, obj, "debugInfo", sizeof("debugInfo") - 1 TSRMLS_CC);
  zend_update_property_bool(soo_class_entry, obj, "debug", sizeof("debug") - 1, soo->debug TSRMLS_CC);
  zend_update_property_long(soo_class_entry, obj, "sslChecks", sizeof("sslChecks") - 1, soo->sslcheck TSRMLS_CC);

  TSRMLS_SET_CTX(soo->thread_ctx);

  if (!sig_method_len) {
    sig_method = OAUTH_SIG_METHOD_HMACSHA1;
  }

  soo->sig_ctx = oauth_create_sig_context(sig_method);

  if (!auth_method) {
    auth_method = OAUTH_AUTH_TYPE_AUTHORIZATION;
  }

  if (soo->properties) {
    zend_hash_clean(soo->properties);
    hasht = soo->properties;
  } else {
    ALLOC_HASHTABLE(hasht);
    zend_hash_init(hasht, 0, NULL, ZVAL_PTR_DTOR, 0);
    soo->properties = hasht;
  }

  MAKE_STD_ZVAL(zck);
  ZVAL_STRING(zck, ck, 1);
  if (soo_set_property(soo, zck, OAUTH_ATTR_CONSUMER_KEY TSRMLS_CC) != SUCCESS) {
    return;
  }

  MAKE_STD_ZVAL(zcs);
  if (cs_len > 0) {
    ZVAL_STRING(zcs, oauth_url_encode(cs, cs_len), 0);
  } else {
    ZVAL_EMPTY_STRING(zcs);
  }
  if (soo_set_property(soo, zcs, OAUTH_ATTR_CONSUMER_SECRET TSRMLS_CC) != SUCCESS) {
    return;
  }

  MAKE_STD_ZVAL(zsm);
  ZVAL_STRING(zsm, sig_method, 1);
  if (soo_set_property(soo, zsm, OAUTH_ATTR_SIGMETHOD TSRMLS_CC) != SUCCESS) {
    return;
  }

  MAKE_STD_ZVAL(zam);
  ZVAL_LONG(zam, auth_method);
  if (soo_set_property(soo, zam, OAUTH_ATTR_AUTHMETHOD TSRMLS_CC) != SUCCESS) {
    return;
  }

  MAKE_STD_ZVAL(zver);
  ZVAL_STRING(zver, OAUTH_DEFAULT_VERSION, 1);
  if (soo_set_property(soo, zver, OAUTH_ATTR_OAUTH_VERSION TSRMLS_CC) != SUCCESS) {
    return;
  }

  soo->debug = 0;
  soo->sslcheck = OAUTH_SSLCHECK_BOTH;
  soo->follow_redirects = 1;

  soo->lastresponse.c = NULL;
#if OAUTH_USE_CURL
  soo->reqengine = OAUTH_REQENGINE_CURL;
#else
  soo->reqengine = OAUTH_REQENGINE_STREAMS;
#endif
}
/* }}} */

void oauth_free_privatekey(zval *privatekey TSRMLS_DC)
{
  /*
  zval *func, *retval;
  zval *args[1];

  if (Z_TYPE_P(privatekey)==IS_RESOURCE) {
    MAKE_STD_ZVAL(retval);
    MAKE_STD_ZVAL(func);

    ZVAL_STRING(func, "openssl_freekey", 2);
    args[0] = privatekey;

    call_user_function(EG(function_table), NULL, func, retval, 1, args TSRMLS_CC);

    FREE_ZVAL(func);
    FREE_ZVAL(retval);
  }
  */

  zval_ptr_dtor(&privatekey);
}

/* {{{ proto void OAuth::__destruct(void)
   clean up of OAuth object */
SO_METHOD(__destruct)
{
  php_so_object *soo;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }

  oauth_prop_hash_dtor(soo TSRMLS_CC);

  if (soo->debug_info) {
    FREE_DEBUG_INFO(soo->debug_info);
    if (soo->debug_info->sbs) {
      efree(soo->debug_info->sbs);
    }
    efree(soo->debug_info);
    soo->debug_info = NULL;
  }

  smart_str_free(&soo->headers_in);
  if (soo->headers_out.c) {
    smart_str_free(&soo->headers_out);
  }
  if(soo->debugArr) {
    zval_ptr_dtor(&soo->debugArr);
  }
  OAUTH_SIGCTX_FREE(soo->sig_ctx);
  if (soo->nonce) {
    efree(soo->nonce);
  }
  if (soo->timestamp) {
    efree(soo->timestamp);
  }
  if(soo->signature) {
    efree(soo->signature);
  }
}
/* }}} */

/* {{{ proto array OAuth::setCAPath(string ca_path, string ca_info)
   Set the Certificate Authority information */
SO_METHOD(setCAPath)
{
  php_so_object *soo;
  char *ca_path, *ca_info;
  int ca_path_len = 0, ca_info_len = 0;
  zval *zca_path, *zca_info;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ss", &ca_path, &ca_path_len, &ca_info, &ca_info_len) == FAILURE) {
    return;
  }

  if (ca_path_len) {
    MAKE_STD_ZVAL(zca_path);
    ZVAL_STRINGL(zca_path, ca_path, ca_path_len, 1);
    if (soo_set_property(soo, zca_path, OAUTH_ATTR_CA_PATH TSRMLS_CC) != SUCCESS) {
      RETURN_FALSE;
    }
  }

  if (ca_info_len) {
    MAKE_STD_ZVAL(zca_info);
    ZVAL_STRINGL(zca_info, ca_info, ca_info_len, 1);
    if (soo_set_property(soo, zca_info, OAUTH_ATTR_CA_INFO TSRMLS_CC) != SUCCESS) {
      RETURN_FALSE;
    }
  }
  RETURN_TRUE;
}
/* }}} */

/* {{{ proto array OAuth::getCAPath(void)
   Get the Certificate Authority information */
SO_METHOD(getCAPath)
{
  /* perhaps make this information available via members too? */
  php_so_object *soo;
  zval **zca_path, **zca_info;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }

  zca_info = soo_get_property(soo, OAUTH_ATTR_CA_INFO TSRMLS_CC);
  zca_path = soo_get_property(soo, OAUTH_ATTR_CA_PATH TSRMLS_CC);

  array_init(return_value);

  if (zca_info || zca_path) {
    if(zca_info) {
      add_assoc_stringl(return_value, "ca_info", Z_STRVAL_PP(zca_info), Z_STRLEN_PP(zca_info), 1);
    }

    if(zca_path) {
      add_assoc_stringl(return_value, "ca_path", Z_STRVAL_PP(zca_path), Z_STRLEN_PP(zca_path), 1);
    }
  }
}
/* }}} */

/* {{{ proto array OAuth::getRequestToken(string request_token_url [, string callback_url ])
   Get request token */
SO_METHOD(getRequestToken)
{
  php_so_object *soo;
  zval *zret = NULL, *callback_url = NULL;
  char *url, *http_method = NULL;
  int url_len = 0, http_method_len = 0;
  long retcode;
  HashTable *args = NULL;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|zs", &url, &url_len, &callback_url, &http_method, &http_method_len) == FAILURE) {
    return;
  }

  if (url_len < 1) {
    soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid request token url length", NULL, NULL TSRMLS_CC);
    RETURN_FALSE;
  }

  if (callback_url && IS_STRING==Z_TYPE_P(callback_url)) {
    ALLOC_HASHTABLE(args);
    zend_hash_init(args, 0, NULL, ZVAL_PTR_DTOR, 0);
    if (Z_STRLEN_P(callback_url) > 0) {
      add_arg_for_req(args, OAUTH_PARAM_CALLBACK, Z_STRVAL_P(callback_url) TSRMLS_CC);
    } else {
      /* empty callback url specified, treat as 1.0a */
      add_arg_for_req(args, OAUTH_PARAM_CALLBACK, OAUTH_CALLBACK_OOB TSRMLS_CC);
    }
  }

  retcode = oauth_fetch(soo, url, oauth_get_http_method(soo, http_method TSRMLS_CC), NULL, NULL, args, 0 TSRMLS_CC);

  if (args) {
    FREE_ARGS_HASH(args);
  }

  if (retcode != -1 && soo->lastresponse.c) {
    array_init(return_value);
    MAKE_STD_ZVAL(zret);
    ZVAL_STRINGL(zret, soo->lastresponse.c, soo->lastresponse.len, 1);
    so_set_response_args(soo->properties, zret, return_value TSRMLS_CC);
    return;
  }
  RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool OAuth::enableRedirects(void)
   Follow and sign redirects automatically (enabled by default) */
SO_METHOD(enableRedirects)
{
  php_so_object *soo;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }

  soo->follow_redirects = 1;

  RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool OAuth::disableRedirects(void)
   Don't follow redirects automatically, thus allowing the request to be manually redirected (enabled by default) */
SO_METHOD(disableRedirects)
{
  php_so_object *soo;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }

  soo->follow_redirects = 0;

  RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool OAuth::disableDebug(void)
   Disable debug mode */
SO_METHOD(disableDebug)
{
  php_so_object *soo;
  zval *obj;

  obj = getThis();
  soo = fetch_so_object(obj TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }

  soo->debug = 0;
  zend_update_property_bool(soo_class_entry, obj, "debug", sizeof("debug") - 1, 0 TSRMLS_CC);

  RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool OAuth::enableDebug(void)
   Enable debug mode, will verbosely output http information about requests */
SO_METHOD(enableDebug)
{
  php_so_object *soo;
  zval *obj;

  obj = getThis();
  soo = fetch_so_object(obj TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }

  soo->debug = 1;
  zend_update_property_bool(soo_class_entry, obj, "debug", sizeof("debug") - 1, 1 TSRMLS_CC);

  RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool OAuth::enableSSLChecks(void)
   Enable SSL verification for requests, enabled by default */
SO_METHOD(enableSSLChecks)
{
  php_so_object *soo;
  zval *obj;

  obj = getThis();
  soo = fetch_so_object(obj TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }

  soo->sslcheck = OAUTH_SSLCHECK_BOTH;
  zend_update_property_long(soo_class_entry, obj, "sslChecks", sizeof("sslChecks") - 1, 1 TSRMLS_CC);

  RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool OAuth::disableSSLChecks(void)
   Disable SSL verification for requests (be careful using this for production) */
SO_METHOD(disableSSLChecks)
{
  php_so_object *soo;
  zval *obj;

  obj = getThis();
  soo = fetch_so_object(obj TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }

  soo->sslcheck = OAUTH_SSLCHECK_NONE;
  zend_update_property_long(soo_class_entry, obj, "sslChecks", sizeof("sslChecks") - 1, 0 TSRMLS_CC);

  RETURN_TRUE;
}
/* }}} */


/* {{{ proto bool OAuth::setSSLChecks(long sslcheck)
   Tweak specific SSL checks for requests (be careful using this for production) */
SO_METHOD(setSSLChecks)
{
  php_so_object *soo;
  zval *obj;
  long sslcheck = OAUTH_SSLCHECK_BOTH;

  obj = getThis();
  soo = fetch_so_object(obj TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &sslcheck) == FAILURE) {
    return;
  }

  soo->sslcheck = sslcheck & OAUTH_SSLCHECK_BOTH;

  zend_update_property_long(soo_class_entry, obj, "sslChecks", sizeof("sslChecks") - 1,
      soo->sslcheck TSRMLS_CC);

  RETURN_TRUE;
}
/* }}} */


/* {{{ proto bool OAuth::setVersion(string version)
   Set oauth_version for requests (default 1.0) */
SO_METHOD(setVersion)
{
  php_so_object *soo;
  int ver_len = 0;
  char *vers;
  zval *zver;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &vers, &ver_len) == FAILURE) {
    return;
  }

  if (ver_len < 1) {
    soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid version", NULL, NULL TSRMLS_CC);
    RETURN_FALSE;
  }

  MAKE_STD_ZVAL(zver);
  ZVAL_STRING(zver, vers, 1);
  if (SUCCESS==soo_set_property(soo, zver, OAUTH_ATTR_OAUTH_VERSION TSRMLS_CC)) {
    RETURN_TRUE;
  }

  RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool OAuth::setAuthType(string auth_type)
   Set the manner in which to send oauth parameters */
SO_METHOD(setAuthType)
{
  php_so_object *soo;
  long auth;
  zval *zauth;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &auth) == FAILURE) {
    return;
  }

  switch (auth) {
    case OAUTH_AUTH_TYPE_URI:
    case OAUTH_AUTH_TYPE_FORM:
    case OAUTH_AUTH_TYPE_AUTHORIZATION:
    case OAUTH_AUTH_TYPE_NONE:
      MAKE_STD_ZVAL(zauth);
      ZVAL_LONG(zauth, auth);
      if (SUCCESS==soo_set_property(soo, zauth, OAUTH_ATTR_AUTHMETHOD TSRMLS_CC)) {
        RETURN_TRUE;
      }
    default:
      soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid auth type", NULL, NULL TSRMLS_CC);
      RETURN_FALSE;
  }

  RETURN_FALSE;
}
/* }}} */

/* {{{ proto bool OAuth::setTimeout(int milliseconds)
   Set the timeout, in milliseconds, for requests */
SO_METHOD(setTimeout)
{
  php_so_object *soo;
  long timeout;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &timeout) == FAILURE) {
    return;
  }

  if (timeout < 0) {
    soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid timeout", NULL, NULL TSRMLS_CC);
    RETURN_FALSE;
  }

  soo->timeout = timeout;

  RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool OAuth::setNonce(string nonce)
   Set oauth_nonce for subsequent requests, if none is set a random nonce will be generated using uniqid */
SO_METHOD(setNonce)
{
  php_so_object *soo;
  int nonce_len;
  char *nonce;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &nonce, &nonce_len) == FAILURE) {
    return;
  }

  if (nonce_len < 1) {
    soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid nonce", NULL, NULL TSRMLS_CC);
    RETURN_FALSE;
  }

  if (soo->nonce) {
    efree(soo->nonce);
  }
  soo->nonce = estrndup(nonce, nonce_len);

  RETURN_TRUE;
}
/* }}} */

SO_METHOD(setTimestamp)
{
  php_so_object *soo;
  int ts_len;
  char *ts;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &ts, &ts_len) == FAILURE) {
    return;
  }

  if (ts_len < 1) {
    soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid timestamp", NULL, NULL TSRMLS_CC);
    RETURN_FALSE;
  }

  if (soo->timestamp) {
    efree(soo->timestamp);
  }
  soo->timestamp = estrndup(ts, ts_len);

  RETURN_TRUE;
}

/* {{{ proto bool OAuth::setToken(string token, string token_secret)
   Set a request or access token and token secret to be used in subsequent requests */
SO_METHOD(setToken)
{
  php_so_object *soo;
  int token_len, token_secret_len;
  char *token, *token_secret;
  zval *t,*ts;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &token, &token_len, &token_secret, &token_secret_len) == FAILURE) {
    return;
  }

  MAKE_STD_ZVAL(t);
  ZVAL_STRING(t, token, 1);
  soo_set_property(soo, t, OAUTH_ATTR_TOKEN TSRMLS_CC);

  if (token_secret_len > 1) {
    MAKE_STD_ZVAL(ts);
    ZVAL_STRING(ts, oauth_url_encode(token_secret, token_secret_len), 0);
    soo_set_property(soo, ts, OAUTH_ATTR_TOKEN_SECRET TSRMLS_CC);
  }
  RETURN_TRUE;
}
/* }}} */

/* {{{ proto void OAuth::setRequestEngine(long reqengine) */
SO_METHOD(setRequestEngine)
{
  php_so_object *soo;
  long reqengine;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &reqengine) == FAILURE) {
    return;
  }
  soo = fetch_so_object(getThis() TSRMLS_CC);

  switch (reqengine) {
    case OAUTH_REQENGINE_STREAMS:
#if OAUTH_USE_CURL
    case OAUTH_REQENGINE_CURL:
#endif
      soo->reqengine = reqengine;
      break;
    default:
      soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid request engine specified", NULL, NULL TSRMLS_CC);
  }
}
/* }}} */

/* {{{ proto bool OAuth::generateSignature(string http_method, string url [, string|array extra_parameters ])
   Generate a signature based on the final HTTP method, URL and a string/array of parameters */
SO_METHOD(generateSignature)
{
  php_so_object *soo;
  int url_len, http_method_len = 0;
  char *url;
  zval *request_args = NULL;
  char *http_method = NULL;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|z", &http_method, &http_method_len, &url, &url_len, &request_args) == FAILURE) {
    return;
  }

  if (url_len < 1) {
    RETURN_BOOL(FALSE);
  }

  if (oauth_fetch(soo, url, http_method, request_args, NULL, NULL, (OAUTH_FETCH_USETOKEN | OAUTH_FETCH_SIGONLY) TSRMLS_CC) < 0) {
    RETURN_BOOL(FALSE);
  } else {
    RETURN_STRING(soo->signature, 1);
  }
}
/* }}} */

/* {{{ proto bool OAuth::fetch(string protected_resource_url [, string|array extra_parameters [, string request_type [, array request_headers]]])
   fetch a protected resource, pass in extra_parameters (array(name => value) or "custom body") */
SO_METHOD(fetch)
{
  php_so_object *soo;
  int fetchurl_len, http_method_len = 0;
  char *fetchurl;
  zval *zret = NULL, *request_args = NULL, *request_headers = NULL;
  char *http_method = NULL;
  long retcode;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|zsa", &fetchurl, &fetchurl_len, &request_args, &http_method, &http_method_len, &request_headers) == FAILURE) {
    return;
  }

  if (fetchurl_len < 1) {
    soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid protected resource url length", NULL, NULL TSRMLS_CC);
    RETURN_FALSE;
  }

  retcode = oauth_fetch(soo, fetchurl, http_method, request_args, request_headers, NULL, OAUTH_FETCH_USETOKEN | OAUTH_OVERRIDE_HTTP_METHOD TSRMLS_CC);

  MAKE_STD_ZVAL(zret);
  ZVAL_STRINGL(zret, soo->lastresponse.c, soo->lastresponse.len, 1);
  so_set_response_args(soo->properties, zret, NULL TSRMLS_CC);

  if ((retcode < 200 || retcode > 206)) {
    RETURN_FALSE;
  } else {
    RETURN_BOOL(TRUE);
  }
}
/* }}} */

/* {{{ proto array OAuth::getAccessToken(string access_token_url [, string auth_session_handle [, string auth_verifier ]])
  Get access token,
  If the server supports Scalable OAuth pass in the auth_session_handle to refresh the token (http://wiki.oauth.net/ScalableOAuth)
  For 1.0a implementation, a verifier token must be passed; this token is not passed unless a value is explicitly assigned via the function arguments or $_GET/$_POST['oauth_verifier'] is set
*/
SO_METHOD(getAccessToken)
{
  php_so_object *soo;
  int aturi_len = 0, ash_len = 0, verifier_len = 0, http_method_len = 0;
  char *aturi, *ash, *http_method = NULL;
  const char *verifier;
  zval *zret = NULL;
  HashTable *args = NULL;
  long retcode;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sss", &aturi, &aturi_len, &ash, &ash_len, &verifier, &verifier_len, &http_method, &http_method_len) == FAILURE) {
    return;
  }

  if (aturi_len < 1) {
    soo_handle_error(soo, OAUTH_ERR_INTERNAL_ERROR, "Invalid access token url length", NULL, NULL TSRMLS_CC);
    RETURN_FALSE;
  }

  if (!verifier_len) {
    /* try to get from _GET/_POST */
    get_request_param(OAUTH_PARAM_VERIFIER, &verifier, &verifier_len TSRMLS_CC);
  }

  if (ash_len > 0 || verifier_len > 0) {
    ALLOC_HASHTABLE(args);
    zend_hash_init(args, 0, NULL, ZVAL_PTR_DTOR, 0);
    if (ash_len > 0) {
      add_arg_for_req(args, OAUTH_PARAM_ASH, ash TSRMLS_CC);
    }
    if (verifier_len > 0) {
      add_arg_for_req(args, OAUTH_PARAM_VERIFIER, verifier TSRMLS_CC);
    }
  }

  retcode = oauth_fetch(soo, aturi, oauth_get_http_method(soo, http_method TSRMLS_CC), NULL, NULL, args, OAUTH_FETCH_USETOKEN TSRMLS_CC);

  if (args) {
    FREE_ARGS_HASH(args);
  }

  if (retcode != -1 && soo->lastresponse.c) {
    array_init(return_value);
    MAKE_STD_ZVAL(zret);
    ZVAL_STRINGL(zret, soo->lastresponse.c, soo->lastresponse.len, 1);
    so_set_response_args(soo->properties, zret, return_value TSRMLS_CC);
    return;
  }
  RETURN_FALSE;
}
/* }}} */

/* {{{ proto array OAuth::getLastResponseInfo(void)
   Get information about the last response */
SO_METHOD(getLastResponseInfo)
{
  php_so_object *soo;
  zval **data_ptr;
  ulong hf = 0;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }

  soo = fetch_so_object(getThis() TSRMLS_CC);

  hf = zend_hash_func(OAUTH_ATTR_LAST_RES_INFO, sizeof(OAUTH_ATTR_LAST_RES_INFO));

  if (zend_hash_quick_find(soo->properties, OAUTH_ATTR_LAST_RES_INFO, sizeof(OAUTH_ATTR_LAST_RES_INFO), hf, (void **)&data_ptr) == SUCCESS) {
    if (Z_TYPE_PP(data_ptr) == IS_ARRAY) {
      convert_to_array_ex(data_ptr);
    }
    RETURN_ZVAL(*data_ptr, 1, 0);
  }
  RETURN_FALSE;
}
/* }}} */

/* {{{ proto array OAuth::getLastResponse(void)
   Get last response */
SO_METHOD(getLastResponse)
{
  php_so_object *soo;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
    return;
  }

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (soo->lastresponse.c) {
    RETURN_STRINGL(soo->lastresponse.c, soo->lastresponse.len, 1);
  }
#if jawed_0
  void *p_data_ptr;
  zval **data_ptr;
  ulong hf = 0;
  ulong hlen = 0;
  char *hkey = OAUTH_ATTR_LAST_RES;
  hkey = OAUTH_RAW_LAST_RES;
  hlen = strlen(hkey)+1;
  hf = zend_hash_func(hkey,hlen);
  if (zend_hash_quick_find(soo->properties, hkey, hlen, hf, &p_data_ptr) == SUCCESS) {
    data_ptr = p_data_ptr;
    RETURN_STRING(Z_STRVAL_P(*data_ptr), 0);
  }
  RETURN_FALSE;
#endif
}
/* }}} */

SO_METHOD(getLastResponseHeaders)
{
  php_so_object *soo;

  if (FAILURE==zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "")) {
    return;
  }

  soo = fetch_so_object(getThis() TSRMLS_CC);
  if (soo->headers_in.c) {
    RETURN_STRINGL(soo->headers_in.c, soo->headers_in.len, 1);
  }
  RETURN_FALSE;
}

/* {{{ proto string OAuth::getRequestHeader(string http_method, string url [, string|array extra_parameters ])
   Generate OAuth header string signature based on the final HTTP method, URL and a string/array of parameters */
SO_METHOD(getRequestHeader)
{
  php_so_object *soo;
  int url_len, http_method_len = 0;
  char *url;
  zval *request_args = NULL;
  char *http_method = NULL;

  soo = fetch_so_object(getThis() TSRMLS_CC);

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss|z", &http_method, &http_method_len, &url, &url_len, &request_args) == FAILURE) {
    return;
  }

  if (url_len < 1) {
    RETURN_BOOL(FALSE);
  }

  if (oauth_fetch(soo, url, http_method, request_args, NULL, NULL,
        (OAUTH_FETCH_USETOKEN | OAUTH_FETCH_HEADONLY) TSRMLS_CC) < 0) {
    RETURN_BOOL(FALSE);
  } else {
    RETURN_STRINGL(soo->headers_out.c, soo->headers_out.len, 1);
  }

  RETURN_FALSE;
}

/* {{{ proto void OAuthException::__construct()
   Instantiate a new OAuthException object */
PHP_METHOD(oauthexception, __construct)
{
}

/* {{{ arginfo */
OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_urlencode, 0, 0, 1)
  ZEND_ARG_INFO(0, uri)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_sbs, 0, 0, 3)
  ZEND_ARG_INFO(0, http_method)
  ZEND_ARG_INFO(0, uri)
  ZEND_ARG_INFO(0, parameters)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth__construct, 0, 0, 2)
  ZEND_ARG_INFO(0, consumer_key)
  ZEND_ARG_INFO(0, consumer_secret)
  ZEND_ARG_INFO(0, signature_method)
  ZEND_ARG_INFO(0, auth_type)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_getrequesttoken, 0, 0, 1)
  ZEND_ARG_INFO(0, request_token_url)
  ZEND_ARG_INFO(0, callback_url)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_setversion, 0, 0, 1)
  ZEND_ARG_INFO(0, version)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_noparams, 0, 0, 0)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_setauthtype, 0, 0, 1)
  ZEND_ARG_INFO(0, auth_type)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_setnonce, 0, 0, 1)
  ZEND_ARG_INFO(0, nonce)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_settimestamp, 0, 0, 1)
  ZEND_ARG_INFO(0, ts)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_settimeout, 0, 0, 1)
  ZEND_ARG_INFO(0, timeout_in_milliseconds)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_setcapath, 0, 0, 2)
  ZEND_ARG_INFO(0, ca_path)
  ZEND_ARG_INFO(0, ca_info)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_settoken, 0, 0, 2)
  ZEND_ARG_INFO(0, token)
  ZEND_ARG_INFO(0, token_secret)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_setrequestengine, 0, 0, 1)
  ZEND_ARG_INFO(0, reqengine)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_fetch, 0, 0, 1)
  ZEND_ARG_INFO(0, protected_resource_url)
  ZEND_ARG_INFO(0, extra_parameters) /* ARRAY_INFO(1, arg, 0) */
  ZEND_ARG_INFO(0, http_method)
  ZEND_ARG_INFO(0, request_headers)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_getaccesstoken, 0, 0, 1)
  ZEND_ARG_INFO(0, access_token_url)
  ZEND_ARG_INFO(0, auth_session_handle)
  ZEND_ARG_INFO(0, auth_verifier)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_setrsacertificate, 0, 0, 1)
  ZEND_ARG_INFO(0, cert)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_gensig, 0, 0, 2)
  ZEND_ARG_INFO(0, http_method)
  ZEND_ARG_INFO(0, url)
  ZEND_ARG_INFO(0, extra_parameters) /* ARRAY_INFO(1, arg, 0) */
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_setsslchecks, 0, 0, 1)
  ZEND_ARG_INFO(0, sslcheck)
ZEND_END_ARG_INFO()

OAUTH_ARGINFO
ZEND_BEGIN_ARG_INFO_EX(arginfo_oauth_getrequestheader, 0, 0, 2)
  ZEND_ARG_INFO(0, http_method)
  ZEND_ARG_INFO(0, url)
  ZEND_ARG_INFO(0, extra_parameters) /* ARRAY_INFO(1, arg, 0) */
ZEND_END_ARG_INFO()


/* }}} */

static zend_function_entry so_functions[] = { /* {{{ */
  SO_ME(__construct,      arginfo_oauth__construct,    ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
  SO_ME(setRSACertificate,  arginfo_oauth_setrsacertificate,  ZEND_ACC_PUBLIC)
  SO_ME(getRequestToken,    arginfo_oauth_getrequesttoken,  ZEND_ACC_PUBLIC)
  SO_ME(getAccessToken,    arginfo_oauth_getaccesstoken,  ZEND_ACC_PUBLIC)
  SO_ME(getLastResponse,    arginfo_oauth_noparams,      ZEND_ACC_PUBLIC)
  SO_ME(getLastResponseInfo,  arginfo_oauth_noparams,      ZEND_ACC_PUBLIC)
  SO_ME(getLastResponseHeaders,  arginfo_oauth_noparams,      ZEND_ACC_PUBLIC)
  SO_ME(setToken,        arginfo_oauth_settoken,      ZEND_ACC_PUBLIC)
  SO_ME(setRequestEngine,    arginfo_oauth_setrequestengine,     ZEND_ACC_PUBLIC)
  SO_ME(setVersion,      arginfo_oauth_setversion,    ZEND_ACC_PUBLIC)
  SO_ME(setAuthType,      arginfo_oauth_setauthtype,    ZEND_ACC_PUBLIC)
  SO_ME(setNonce,        arginfo_oauth_setnonce,      ZEND_ACC_PUBLIC)
  SO_ME(setTimestamp,      arginfo_oauth_settimestamp,    ZEND_ACC_PUBLIC)
  SO_ME(fetch,        arginfo_oauth_fetch,      ZEND_ACC_PUBLIC)
  SO_ME(enableDebug,      arginfo_oauth_noparams,      ZEND_ACC_PUBLIC)
  SO_ME(disableDebug,      arginfo_oauth_noparams,      ZEND_ACC_PUBLIC)
  SO_ME(enableSSLChecks,    arginfo_oauth_noparams,      ZEND_ACC_PUBLIC)
  SO_ME(disableSSLChecks,    arginfo_oauth_noparams,      ZEND_ACC_PUBLIC)
  SO_ME(enableRedirects,    arginfo_oauth_noparams,      ZEND_ACC_PUBLIC)
  SO_ME(disableRedirects,    arginfo_oauth_noparams,      ZEND_ACC_PUBLIC)
  SO_ME(setCAPath,      arginfo_oauth_setcapath,    ZEND_ACC_PUBLIC)
  SO_ME(getCAPath,      arginfo_oauth_noparams,      ZEND_ACC_PUBLIC)
  SO_ME(generateSignature,  arginfo_oauth_gensig,      ZEND_ACC_PUBLIC)
  SO_ME(setTimeout,      arginfo_oauth_settimeout,    ZEND_ACC_PUBLIC)
  SO_ME(setSSLChecks,      arginfo_oauth_setsslchecks,    ZEND_ACC_PUBLIC)
  SO_ME(getRequestHeader,    arginfo_oauth_getrequestheader,  ZEND_ACC_PUBLIC)
  SO_ME(__destruct,      arginfo_oauth_noparams,      ZEND_ACC_PUBLIC)
  {NULL, NULL, NULL}
};
/* }}} */

#ifdef ZEND_ENGINE_2_4
static zval *oauth_read_member(zval *obj, zval *mem, int type, const zend_literal *key TSRMLS_DC) /* {{{ */
#else
static zval *oauth_read_member(zval *obj, zval *mem, int type TSRMLS_DC) /* {{{ */
#endif
{
  zval *return_value = NULL;
  php_so_object *soo;

  soo = fetch_so_object(obj TSRMLS_CC);

  return_value = OAUTH_READ_PROPERTY(obj, mem, type);

  if(!strcasecmp(Z_STRVAL_P(mem),"debug")) {
    convert_to_boolean(return_value);
    ZVAL_BOOL(return_value, soo->debug);
  } else if(!strcasecmp(Z_STRVAL_P(mem),"sslChecks")) {
    ZVAL_LONG(return_value, soo->sslcheck);
  }
  return return_value;
} /* }}} */

#ifdef ZEND_ENGINE_2_4
static void oauth_write_member(zval *obj, zval *mem, zval *value, const zend_literal *key TSRMLS_DC) /* {{{ */
#else
static void oauth_write_member(zval *obj, zval *mem, zval *value TSRMLS_DC) /* {{{ */
#endif
{
  const char *property;
  php_so_object *soo;

  property = Z_STRVAL_P(mem);
  soo = fetch_so_object(obj TSRMLS_CC);

  if(!strcmp(property,"debug")) {
    soo->debug = Z_LVAL_P(value);
  } else if(!strcmp(property,"sslChecks")) {
    soo->sslcheck = Z_LVAL_P(value);
  }
  OAUTH_WRITE_PROPERTY(obj, mem, value);
} /* }}} */

/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(oauth)
{
  zend_class_entry soce, soo_ex_ce;

#if OAUTH_USE_CURL
  if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
    return FAILURE;
  }
#endif

  INIT_CLASS_ENTRY(soce, "OAuth", so_functions);
  soce.create_object = new_so_object;

  soo_class_entry = zend_register_internal_class(&soce TSRMLS_CC);
  memcpy(&so_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

  so_object_handlers.read_property = oauth_read_member;
  so_object_handlers.write_property = oauth_write_member;
  so_object_handlers.clone_obj = oauth_clone_obj;

  zend_declare_property_long(soo_class_entry, "debug", sizeof("debug")-1, 0, ZEND_ACC_PUBLIC TSRMLS_CC);
  zend_declare_property_long(soo_class_entry, "sslChecks", sizeof("sslChecks")-1, 1, ZEND_ACC_PUBLIC TSRMLS_CC);
  zend_declare_property_string(soo_class_entry, "debugInfo", sizeof("debugInfo")-1, "", ZEND_ACC_PUBLIC TSRMLS_CC);

  INIT_CLASS_ENTRY(soo_ex_ce, "OAuthException", NULL);

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 2)
  soo_exception_ce = zend_register_internal_class_ex(&soo_ex_ce, zend_exception_get_default(), NULL TSRMLS_CC);
#else
  soo_exception_ce = zend_register_internal_class_ex(&soo_ex_ce, zend_exception_get_default(TSRMLS_C), NULL TSRMLS_CC);
#endif
  zend_declare_property_null(soo_exception_ce, "lastResponse", sizeof("lastResponse")-1, ZEND_ACC_PUBLIC TSRMLS_CC);
  zend_declare_property_null(soo_exception_ce, "debugInfo", sizeof("debugInfo")-1, ZEND_ACC_PUBLIC TSRMLS_CC);

  REGISTER_STRING_CONSTANT("OAUTH_SIG_METHOD_HMACSHA1", OAUTH_SIG_METHOD_HMACSHA1, CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("OAUTH_SIG_METHOD_HMACSHA256", OAUTH_SIG_METHOD_HMACSHA256, CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("OAUTH_SIG_METHOD_RSASHA1", OAUTH_SIG_METHOD_RSASHA1, CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("OAUTH_SIG_METHOD_PLAINTEXT", OAUTH_SIG_METHOD_PLAINTEXT, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_AUTH_TYPE_AUTHORIZATION", OAUTH_AUTH_TYPE_AUTHORIZATION, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_AUTH_TYPE_URI", OAUTH_AUTH_TYPE_URI, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_AUTH_TYPE_FORM", OAUTH_AUTH_TYPE_FORM, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_AUTH_TYPE_NONE", OAUTH_AUTH_TYPE_NONE, CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("OAUTH_HTTP_METHOD_GET", OAUTH_HTTP_METHOD_GET, CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("OAUTH_HTTP_METHOD_POST", OAUTH_HTTP_METHOD_POST, CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("OAUTH_HTTP_METHOD_PUT", OAUTH_HTTP_METHOD_PUT, CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("OAUTH_HTTP_METHOD_HEAD", OAUTH_HTTP_METHOD_HEAD, CONST_CS | CONST_PERSISTENT);
  REGISTER_STRING_CONSTANT("OAUTH_HTTP_METHOD_DELETE", OAUTH_HTTP_METHOD_DELETE, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_REQENGINE_STREAMS", OAUTH_REQENGINE_STREAMS, CONST_CS | CONST_PERSISTENT);
#ifdef OAUTH_USE_CURL
  REGISTER_LONG_CONSTANT("OAUTH_REQENGINE_CURL", OAUTH_REQENGINE_CURL, CONST_CS | CONST_PERSISTENT);
#endif
  REGISTER_LONG_CONSTANT("OAUTH_SSLCHECK_NONE", OAUTH_SSLCHECK_NONE, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_SSLCHECK_HOST", OAUTH_SSLCHECK_HOST, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_SSLCHECK_PEER", OAUTH_SSLCHECK_PEER, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_SSLCHECK_BOTH", OAUTH_SSLCHECK_BOTH, CONST_CS | CONST_PERSISTENT);

  oauth_provider_register_class(TSRMLS_C);
  REGISTER_LONG_CONSTANT("OAUTH_OK", OAUTH_OK, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_BAD_NONCE", OAUTH_BAD_NONCE, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_BAD_TIMESTAMP", OAUTH_BAD_TIMESTAMP, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_CONSUMER_KEY_UNKNOWN", OAUTH_CONSUMER_KEY_UNKNOWN, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_CONSUMER_KEY_REFUSED", OAUTH_CONSUMER_KEY_REFUSED, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_INVALID_SIGNATURE", OAUTH_INVALID_SIGNATURE, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_TOKEN_USED", OAUTH_TOKEN_USED, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_TOKEN_EXPIRED", OAUTH_TOKEN_EXPIRED, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_TOKEN_REVOKED", OAUTH_TOKEN_REVOKED, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_TOKEN_REJECTED", OAUTH_TOKEN_REJECTED, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_VERIFIER_INVALID", OAUTH_VERIFIER_INVALID, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_PARAMETER_ABSENT", OAUTH_PARAMETER_ABSENT, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("OAUTH_SIGNATURE_METHOD_REJECTED", OAUTH_SIGNATURE_METHOD_REJECTED, CONST_CS | CONST_PERSISTENT);
  return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(oauth)
{
  soo_class_entry = NULL;
  soo_exception_ce = NULL;
#if OAUTH_USE_CURL
  curl_global_cleanup();
#endif
  return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(oauth)
{
  php_info_print_table_start();
  php_info_print_table_header(2, "OAuth support", "enabled");
  php_info_print_table_row(2, "PLAINTEXT support", "enabled");
#if HAVE_OPENSSL_EXT
  php_info_print_table_row(2, "RSA-SHA1 support", "enabled");
#else
  php_info_print_table_row(2, "RSA-SHA1 support", "not supported");
#endif
  php_info_print_table_row(2, "HMAC-SHA1 support", "enabled");
#if OAUTH_USE_CURL
  php_info_print_table_row(2, "Request engine support", "php_streams, curl");
#else
  php_info_print_table_row(2, "Request engine support", "php_streams");
#endif
  php_info_print_table_row(2, "source version", "$Id: oauth.c 325799 2012-05-24 21:07:51Z jawed $");
  php_info_print_table_row(2, "version", OAUTH_EXT_VER);
  php_info_print_table_end();
}
/* }}} */

/* TODO expose a function for base sig string */
zend_function_entry oauth_functions[] = { /* {{{ */
  PHP_FE(oauth_urlencode,    arginfo_oauth_urlencode)
  PHP_FE(oauth_get_sbs,    arginfo_oauth_sbs)
  { NULL, NULL, NULL }
};
/* }}} */

/* {{{ oauth_module_entry */
zend_module_entry oauth_module_entry = {
  STANDARD_MODULE_HEADER_EX, NULL,
  NULL,
  "OAuth",
  oauth_functions,
  PHP_MINIT(oauth),
  PHP_MSHUTDOWN(oauth),
  NULL,
  NULL,
  PHP_MINFO(oauth),
  OAUTH_EXT_VER,
  STANDARD_MODULE_PROPERTIES
};
/* }}} */

#if COMPILE_DL_OAUTH
ZEND_GET_MODULE(oauth)
#endif

/**
 * Local Variables:
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: t
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4 noexpandtab
 */
