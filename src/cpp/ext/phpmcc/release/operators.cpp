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
#include <cpp/base/runtime_option.h>
#include <cpp/base/server/server_stats.h>

#include "ext_php_mcc.h"
#include "constants.h"
#include "ext_php_mcc_impl.h"
#include "serialization.h"
#include "types.h"

using namespace HPHP;

namespace HPHP {

/** Converts PHP assoc array of string to a malloced array of nstrings. Note,
    this method references the string data in the PHP array.  It does not make
    a separate copy.  */
static inline nstring_t* nstring_array_new(nstring_t* &resultp,
                                           size_t &countp, CVarRef values) {
  nstring_t* result;
  size_t count;
  size_t ix = 0;

  if (values.is(KindOfArray)) {
    Array ht = values.toArray();
    count = ht.size();

    if ((result = (nstring_t*)malloc(sizeof(nstring_t) * count)) == NULL) {
      return result;
    }

    for (ArrayIter it = ht.begin(); !it.end(); ++it) {
      Variant key = it.first();
      Variant value = it.second();
      if (key.isString()) {
        String s = key.toString();
        result[ix].str = const_cast<char*>(s.data());
        result[ix].len = s.size();
        ix++;
      } else if (value.isString()) {
        String s = value.toString();
        result[ix].str = const_cast<char*>(s.data());
        result[ix].len = s.size();
        ix++;
      }
    }
  } else if (values.isString()) {
    if ((result = (nstring_t*)malloc(sizeof(nstring_t))) == NULL) {
      return result;
    }
    String s = values.toString();
    result->str = const_cast<char*>(s.data());
    result->len = s.size();
    ix = 1;
  } else {
    result = NULL;
  }

  resultp = result;
  countp = ix;
  return result;
}

static inline void nstring_array_del(nstring_t* nstring_array) {
  free(nstring_array);
}

/** Converts PHP assoc array of string to a malloced array of nstrings. Note,
    this method references the string data in the PHP array.  It does not make
    a separate copy.  */
static inline uint32 nstring_dict_new(MccResourcePtr &phpmcc,
                                      CArrRef input,
                                      nstring_t*& ret_keys,
                                      nstring_t*& ret_values) {
  nstring_t* keys = NULL, * values = NULL;
  size_t count;
  count = input.size();
  uint32 ix = 0;

  if ((keys = (nstring_t*)malloc(sizeof(nstring_t) * count)) == NULL) {
    goto epilogue;
  }
  if ((values = (nstring_t*)malloc(sizeof(nstring_t) * count)) == NULL) {
    goto epilogue;
  }

  for (ArrayIter it = input.begin(); !it.end(); ++it) {
    CVarRef key = it.first();
    ASSERT(key.isString());
    String skey = key.toString();
    keys[ix].str = const_cast<char*>(skey.data());
    keys[ix].len = skey.size();
    Variant value = it.second();
    ASSERT(value.isString());
    String sval = value.toString();
    values[ix].str = const_cast<char*>(sval.data());
    values[ix].len = sval.size();

    // at this point, if we've made it here, keys[ix] points to the key and
    // values[ix] points to the string-ified version of the value.
    ix++;
  }

  ret_keys = keys;
  ret_values = values;

 epilogue:
  if (ix == 0) {
    if (keys != NULL) {
      free(keys);
    }
    if (values != NULL) {
      free(values);
    }
  }
  return ix;
}

///////////////////////////////////////////////////////////////////////////////
// transformation

/** Converts a PHP value into a string, performing all the necessary
    transformations.

    @param phpmcc          - php mcc handle.
    @param key             - a pointer to the key.
    @param object          - the object to transform.
    @param value           - where to store the resulting object.
    @param flags           - flags indicating the transformations completed is
    written to this address.
    @param compress        - if nonzero, compression is applied to the object.
    @param proxy_replicate - if nonzero, the proxy-replicate flag is set.
    @param async_set       - if nonzero, the async-set flag is set.
    additional php objects (zvals), the address of the
    zvals is written to this address.  the caller must
    free it when it is done with the value pointer.

    @return 0 if no error occured, nonzero otherwise.
*/
static int phpmcc_transform_value(MccResourcePtr &phpmcc,
                                  CStrRef key,
                                  CVarRef object,
                                  String& value,
                                  phpmcc_flags_t& flags,
                                  const bool compress,
                                  int64 proxy_replicate,
                                  int64 async_set) {
  String serialized, compressed;

  flags = (phpmcc_flags_t)0;
  if (!object.isString()) {
    if (!phpmcc_fb_serialize_value(phpmcc, key, flags, object, serialized) &&
        !phpmcc_php_serialize_value(phpmcc, key, flags, object, serialized)) {
      return -1;
    }
  } else {
    serialized = object.toString();
  }

  /* If compression fails or if result is greater than original, just send
     the uncompressed original. */
  if (compress && phpmcc->m_compression_threshold != 0 &&
      (size_t)serialized.size() >= phpmcc->m_compression_threshold) {

    if (!phpmcc_zlib_compress_value(phpmcc, flags, serialized, compressed) ||
        serialized.size() <= compressed.size()) {
      flags = (phpmcc_flags_t)
        (flags & ~(phpmcc_compressed | phpmcc_nzlib_compressed));
    } else {
      serialized = compressed;
    }
  }

  value = serialized;

  if (async_set) {
    flags = (phpmcc_flags_t)(flags | phpmcc_async_set);
  }
  if (proxy_replicate) {
    flags = (phpmcc_flags_t)(flags | phpmcc_proxy_replicate);
  }
  return 0;
}

static inline uint32 phpmcc_transform_values(MccResourcePtr &phpmcc,
                                             CArrRef input,
                                             Array& result,
                                             phpmcc_flags_t*& ret_flagss,
                                             bool compress,
                                             int64 proxy_replicate,
                                             int64 async_set) {
  phpmcc_flags_t* flagss = NULL;
  size_t count = input.size();
  uint32 ix = 0;

  if ((flagss =
       (phpmcc_flags_t*) malloc(sizeof(phpmcc_flags_t) * count)) == NULL) {
    return 0;
  }

  for (ArrayIter it = input.begin(); !it.end(); ++it) {
    CVarRef key = it.first();
    if (!key.isString()) continue;
    Variant value = it.second();
    // now examine the value.
    if (value.isNull()) {
      // no value?
      continue;
    }
    String res;
    if (phpmcc_transform_value(phpmcc, key.toString(), value, res, flagss[ix],
                               compress, proxy_replicate, async_set) != 0) {
      /* error has occured */
      continue;
    }
    result.set(key, res);

    ix++;
  }
  ret_flagss = flagss;
  return ix;
}

///////////////////////////////////////////////////////////////////////////////
// updates

static Variant phpmcc_update(MccResourcePtr &phpmcc,
                             CStrRef key,
                             CVarRef object,
                             int64 exptime,
                             bool compress,
                             int64 proxy_replicate,
                             int64 async_set,
                             mcc_cmdtype_t cmd) {
  String value;
  phpmcc_flags_t flags = (phpmcc_flags_t)0;
  mcc_res_t result = mcc_res_local_error;
  mcc_req_t* req = NULL;
  typedef mcc_req_t* (*update_funcptr_t) (mcc_handle_t, const nstring_t* const,
                                          const uint32_t, const uint32_t,
                                          const nstring_t* const,
                                          const size_t);
  update_funcptr_t update_op;

  if (phpmcc_transform_value(phpmcc, key, object, value, flags,
                             compress, proxy_replicate, async_set) == 0) {
    if (RuntimeOption::MemcacheReadOnly) {
      return true;
    }
    /* get the function pointer for the command type so we don't have to keep
     * on switching on cmd.  this is in theory taken care of by CSE, but we'll
     * do it also for readability. */
    bool ok = true;
    switch (cmd) {
    case mcc_set_cmd:
      update_op = mcc_set;
      break;
    case mcc_add_cmd:
      update_op = mcc_add;
      break;
    case mcc_replace_cmd:
      update_op = mcc_replace;
      break;
    default:
      result = mcc_res_local_error;
      ASSERT(0);
      ok = false;
    }
    if (ok) {
      /* make the request on the primary. */
      nstring_t nvalue, nkey;
      phpstring_to_nstring(nvalue, value);
      phpstring_to_nstring(nkey, key);
      req = update_op(phpmcc->m_mcc, &nkey, flags, exptime, &nvalue, 1);
      /* propagation to mirrors for update might be handled by mcproxy.  if a
       * update proxy has not been set, then propagate it ourselves. */
      if ((phpmcc->m_proxy_ops & mcc_proxy_update_op) == 0) {
        for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
             mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
          mcc_req_t* mirror_req =
            update_op((*mirror)->m_mcc, &nkey, flags, exptime, &nvalue, 1);
          if (mirror_req != NULL) {
            mcc_req_del(mirror_req);
          }
          mcc_clear_err(phpmcc->m_mcc, NULL);
        }
      }

      if (req != NULL) {
        result = req->result;
        mcc_req_del(req);
      }
    }
  }

  Variant retval;
  if (result == mcc_res_stored) {
    retval = true;
  } else if (result == mcc_res_notstored) {
    retval = false;
  } else {
    mcc_log_if_error(phpmcc);
    retval = null;
  }
  if (phpmcc->m_events.size() > 0) {
    phpmcc_apevent_dispatcher(phpmcc);
  }
  return retval;
}

/* todo work out these optional args */
static Variant zim_phpmcc_update(MccResourcePtr &phpmcc,
                                 CVarRef phpkey,
                                 CVarRef value,
                                 int64 exptime /* = 0 */,
                                 bool compress /* = 1 */,
                                 int64 proxy_replicate /* = 0 */,
                                 int64 async_set /* = 0 */,
                                 mcc_cmdtype_t cmd) {
  if (!phpmcc.get()) {
    return null;
  }

  if (!phpkey.isString()) {
    phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MISSINGS_REQS,
               __FILE__, __LINE__, "key must be a string");
    return null;
  }

  return phpmcc_update(phpmcc, phpkey.toString(), value,
                       exptime, compress, proxy_replicate, async_set, cmd);
}

static Variant phpmcc_multi_update(MccResourcePtr &phpmcc,
                                   mcc_cmdtype_t cmd, CArrRef keys_values,
                                   uint32_t exptime, bool compress,
                                   int64 proxy_replicate, int64 async_set) {
  nstring_t* keys = NULL, * values = NULL;
  phpmcc_flags_t* flagss = NULL;
  unsigned npairs;
  mcc_req_t* reqs, * current;
  int log_errors = 0;

  Array transformed;
  if (!phpmcc_transform_values(phpmcc, keys_values, transformed, flagss,
                               compress, proxy_replicate, async_set)) {
    return null;
  }
  ASSERT(flagss);

  if ((npairs = nstring_dict_new(phpmcc, transformed, keys, values)) == 0) {
    return null;
  }

  current = reqs = mcc_update_generic
    (phpmcc->m_mcc, cmd, keys, (uint32_t*)flagss,
     &exptime, values, npairs, 1 /* unique flags */,
     0 /* shared expiration times */);
  Array retval;
  while (current != NULL) {
    nstring_t* key = &current->u.getreq.key;
    Variant phpvalue;

    if (current->result == mcc_res_stored) {
      phpvalue = true;
    } else if (current->result == mcc_res_notstored) {
      phpvalue = false;
    } else {
      log_errors = 1;
      phpvalue = null;
    }
    retval.set(String(key->str, key->len, CopyString), phpvalue);

    current = current->item.tqe_next;
  }

  if (reqs != NULL) {
    mcc_req_del(reqs);
  }

  /* propagation to mirrors for update might be handled by mcproxy.  if a
   * update proxy has not been set, then propagate it ourselves. */
  if ((phpmcc->m_proxy_ops & mcc_proxy_update_op) == 0) {
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mcc_req_t* mirror_reqs =
        mcc_update_generic((*mirror)->m_mcc, cmd, keys, (uint32_t*)flagss,
                           &exptime, values, npairs,
                           1 /* unique flags */,
                           0 /* shared expiration times */);

      if (mirror_reqs != NULL) {
        mcc_req_del(mirror_reqs);
      }
      mcc_clear_err(phpmcc->m_mcc, NULL);
    }
  }

  if (keys != NULL) {
    nstring_array_del(keys);
  }
  if (values != NULL) {
    nstring_array_del(values);
  }
  if (flagss != NULL) {
    free(flagss);
  }
  if (log_errors == 1) {
    mcc_log_if_error(phpmcc);
  }

  /* If we didn't get any results, check for errors */
  if (phpmcc->m_events.size() > 0) {
    phpmcc_apevent_dispatcher(phpmcc);
  }
  return retval;
}

static Variant zim_phpmcc_multi_update(MccResourcePtr &phpmcc,
                                       CArrRef keys_values,
                                       int64 exptime /* = 0 */,
                                       int64 compress /* = 1 */,
                                       int64 proxy_replicate /* = 0 */,
                                       int64 async_set /* = 0 */,
                                       mcc_cmdtype_t cmd) {
  if (!phpmcc.get()) {
    return null;
  }
  return phpmcc_multi_update(phpmcc, cmd, keys_values, exptime,
                             compress, proxy_replicate, async_set);
}

static Variant zim_phpmcc_arith_update(MccResourcePtr &phpmcc,
                                       CStrRef key,
                                       int64 value /* = 1 */,
                                       mcc_cmdtype_t cmd) {
  if (!phpmcc.get()) {
    return null;
  }

  Variant retval;
  int64 result = 0;
  mcc_res_t status;
  typedef uint32_t (*arith_funcptr_t) (mcc_handle_t, const nstring_t* const,
                                       const int, mcc_res_t*);
  arith_funcptr_t op;

  switch(cmd) {
  case mcc_incr_cmd:
    op = mcc_incr_ex;
    break;
  case mcc_decr_cmd:
    op = mcc_decr_ex;
    break;
  default:
    ASSERT(0);
    return 0;
  }
  nstring_t nkey;
  phpstring_to_nstring(nkey, key);
  result = op(phpmcc->m_mcc, &nkey, value, &status);

  if (status != mcc_res_stored) {
    mcc_log_if_error(phpmcc);
    retval = false;
  } else {
    retval = result;
  }

  /* propagation to mirrors for increment/decrement might be handled by
   * mcproxy.  if a delete proxy has not been set, then propagate it
   * ourselves. */
  if ((phpmcc->m_proxy_ops & mcc_proxy_arith_op) == 0) {
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      result = op((*mirror)->m_mcc, &nkey, value, &status);
    }
  }

  if (phpmcc->m_events.size() > 0) {
    phpmcc_apevent_dispatcher(phpmcc);
  }
  return retval;
}

static Variant phpmcc_delete(MccResourcePtr &phpmcc, CVarRef phpkeys,
                             int64 exptime, bool with_details) {
  nstring_t* keys;
  size_t nkeys;
  if (nstring_array_new(keys, nkeys, phpkeys) == NULL) {
    return null;
  }

  mcc_req_t* reqs = mcc_delete(phpmcc->m_mcc, keys, nkeys, exptime);
  mcc_req_t* current = reqs;

  Variant retval;
  if (phpkeys.is(KindOfArray)) {
    while (current != NULL) {
      nstring_t* key = &current->u.getreq.key;
      String skey(key->str, key->len, CopyString);
      switch(current->result) {
      case mcc_res_deleted:
      case mcc_res_notfound:
        if (with_details) {
          retval.set(skey, (current->result == mcc_res_deleted) ?
                     PHPMCC_DELETE_DELETED : PHPMCC_DELETE_NOTFOUND);
        } else {
          retval.set(skey, current->result == mcc_res_deleted);
        }
        break;
      case mcc_res_bad_key:
        if (with_details) {
          retval.set(skey, PHPMCC_DELETE_ERROR_NOLOG);
        } else {
          retval.set(skey, null);
        }
        break;
      default:
        if (with_details) {
          retval.set(skey, PHPMCC_DELETE_ERROR_LOG);
        } else {
          retval.set(skey, null);
        }
        break;
      }

      current = current->item.tqe_next;
    }
  } else if (phpkeys.isString()) {
    if (current != NULL) {
      switch(current->result) {
      case mcc_res_deleted:
      case mcc_res_notfound:
        if (with_details) {
          retval = (current->result == mcc_res_deleted) ?
            PHPMCC_DELETE_DELETED : PHPMCC_DELETE_NOTFOUND;
        } else {
          retval = current->result == mcc_res_deleted;
        }
        break;
      case mcc_res_bad_key:
        if (with_details) {
          retval = PHPMCC_DELETE_ERROR_NOLOG;
        } else {
          retval = null;
        }
        break;
      default:
        Logger::Error("php_mcc: Failed to delete key %s with result %d",
                      phpkeys.toString().data(), (int)current->result);
        if (with_details) {
          retval = PHPMCC_DELETE_ERROR_LOG;
        } else {
          retval = null;
        }
        break;
      }
    }
  }

  mcc_req_del(reqs);

  /* propagation to mirrors for delete might be handled by mcproxy.  if a
   * delete proxy has not been set, then propagate it ourselves. */
  if ((phpmcc->m_proxy_ops & mcc_proxy_delete_op) == 0) {
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      reqs = mcc_delete((*mirror)->m_mcc, keys, nkeys, exptime);
      mcc_req_del(reqs);
    }
  }

  nstring_array_del(keys);
  if (phpmcc->m_events.size() > 0) {
    phpmcc_apevent_dispatcher(phpmcc);
  }
  return retval;
}

///////////////////////////////////////////////////////////////////////////////
// processors

/*
  phpmcc_get works with a callback setup.  phpmcc_get(..) sets up the keys and
  the callback context structure.  Then it calls phpmcc_get_helper(..), which
  retrieves the values from the primary group and the mirrors.  Once it has
  resolved the return values from the primary group and the mirrors, it
  post-processes the values into php zvals and calls the callback functions.

  There are two callback functions currently defined.  One handles multiget
  requests, and places each zval into a dictionary.  The other handles single
  get requests, and places the zval into the return value.
*/
static void
phpmcc_multiget_processor(const nstring_t search_key,
                          mcc_res_t result, CVarRef value,
                          phpmcc_get_processor_context_t* context) {
  Variant& hashtable = *(context->results);
  if (result == mcc_res_found) {
    context->hits ++;
    String skey(search_key.str, search_key.len, CopyString);
    hashtable.set(skey, value);
  } else if (mcc_res_is_err(result)) {
    context->errors ++;
  }
}

static void
phpmcc_multiget_record_errors_processor(const nstring_t search_key,
                                        mcc_res_t result,
                                        CVarRef value,
                                        phpmcc_get_processor_context_t* context) {
  Variant& hashtable = *(context->results);
  Variant& error_keys = *(context->additional_context);
  String skey(search_key.str, search_key.len, CopyString);
  if (result == mcc_res_found) {
    context->hits ++;
    if (!value.isNull()) {
      hashtable.set(skey, value);
    }
  } else {
    if (mcc_res_is_err(result)) {
      error_keys.set(skey, true);
      context->errors ++;
    }
  }
}

static void phpmcc_get_processor(const nstring_t search_key,
                                 mcc_res_t result,
                                 CVarRef value,
                                 phpmcc_get_processor_context_t* context) {
  Variant& retval = *(context->results);

  if (result == mcc_res_found) {
    context->hits ++;
    if (!value.isNull()) {
      retval = value;
      return;
    }
  }

  if (mcc_res_is_err(result)) {
    context->errors ++;
  }

  retval = null;
}

///////////////////////////////////////////////////////////////////////////////
// results

static void process_mirror_results(MccResourcePtr &phpmcc,
                                   const nstring_t& key,
                                   mcc_res_t* const result_ptr,
                                   nstring_t* const value_ptr,
                                   uint32_t* const flags_ptr,
                                   MccMirrorMcc::ConsistencyModel model,
                                   const mcc_req_t* mirror_result) {
  switch (model) {
  case MccMirrorMcc::CONSISTENCY_IGNORE:
    return;

  case MccMirrorMcc::CONSISTENCY_MATCH_ALL:
    /* check for a mismatch of return status (hit vs miss).  then
     * fallthrough and check the actual return values if they are both
     * hits. */
    if (*result_ptr != mirror_result->result) {
      /* the only case we permit the status to mismatch is
       * mcc_res_local_error, because that's how libmcc signals that
       * the server has no access points.  this is unfortunately
       * rather crude, as mcc_res_local_error is also returned when
       * the server has access points but they are all down. */
      if (mirror_result->result == mcc_res_local_error) {
        return;
      }

      phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MIRROR_CONSISTENCY,
                 __FILE__, __LINE__,
                 "CONSISTENCY_MATCH_ALL result comparison failed "
                 "(key = \"%.*s\").", key.len, key.str);
      *result_ptr = mcc_res_remote_error;
    }

    if (*result_ptr != mcc_res_found) {
      return;
    }
    break;

  case MccMirrorMcc::CONSISTENCY_MATCH_HITS:
    /* if either is a miss, don't compare. */
    if (*result_ptr != mcc_res_found ||
        mirror_result->result != mcc_res_found) {
      return;
    }
    break;

  case MccMirrorMcc::CONSISTENCY_MATCH_HITS_SUPERCEDES:
    /* if we miss in the primary and hit in the mirror, replace the
     * result with the mirror's result.  immediately return because
     * there's no need to do any checking. */
    if (*result_ptr != mcc_res_found &&
        mirror_result->result == mcc_res_found) {
      *result_ptr = mcc_res_found;
      *value_ptr = mirror_result->u.getreq.value;
      *flags_ptr = mirror_result->u.getreq.flags;
      return;
    }

    /* if either is a miss, don't compare. */
    if (*result_ptr != mcc_res_found ||
        mirror_result->result != mcc_res_found) {
      return;
    }
    break;
  }

  /* both the primary and the mirror hit.  compare the results. */
  ASSERT(*result_ptr == mcc_res_found);
  ASSERT(mirror_result->result == mcc_res_found);

  if (*flags_ptr != mirror_result->u.getreq.flags ||
      nstring_cmp(value_ptr, &mirror_result->u.getreq.value) != 0) {
    /* the results mismatched in some way, flag the error. */
    phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MIRROR_CONSISTENCY,
               __FILE__, __LINE__,
               "result comparison failed (key = \"%.*s\").",
               key.len, key.str);

    /* squash the hit */
    *result_ptr = mcc_res_remote_error;
  }
}

/* given a key, and the result from the primary server, determine the final
 * value to return to the client.  this must take into account the mirrors and
 * their results as well as deserializing and decompressing. */
static mcc_res_t resolve_value(MccResourcePtr &phpmcc,
                               const nstring_t& key, mcc_res_t result,
                               nstring_t value, uint32 flags,
                               mcc_req_t* const * const mirror_reqs,
                               Variant& php_value) {
  /* first, evaluate the mirrors. */
  int i = 0;
  for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
       mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
    mcc_req_t* iterator = mirror_reqs[i];
    if (iterator == NULL) {
      continue;
    }

    ASSERT(nstring_cmp(&key, &iterator->u.getreq.key) == 0);

    /* found our key */
    process_mirror_results(phpmcc, key, &result, &value, &flags,
                           (*mirror)->m_model, iterator);
    i++;
  }

  if (result == mcc_res_found) {
    int valid = 1;
    const char *val = value.str;
    int len = value.len;
    String php_uncompressedvalue;
    if (flags & (phpmcc_compressed | phpmcc_nzlib_compressed)) {
      if ((valid = phpmcc_zlib_uncompress_value(phpmcc, flags, val, len,
                                                php_uncompressedvalue))) {
        val = php_uncompressedvalue.data();
        len = php_uncompressedvalue.size();
      } else {
        phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_UNCOMPRESS,
                   __FILE__, __LINE__, "failed to uncompress key %s",
                   key.str);
      }
    }

    if (valid) {
      if (flags & (phpmcc_serialized | phpmcc_fb_serialized)) {
        if (!(valid = phpmcc_unserialize_value(phpmcc, (phpmcc_flags_t)flags,
                                               val, len, php_value))) {
          phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_UNSERIALIZE,
                     __FILE__, __LINE__, "failed to unserialize key %s",
                     key.str);
        }
      } else {
        if (val == value.str) {
          php_value = String(val, len, CopyString);
        } else {
          ASSERT(val == php_uncompressedvalue.data());
          php_value = php_uncompressedvalue;
        }
      }
    }

    /* Only add value if we successful */
    if (!valid) {
      php_value = null;
      phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_BAD_VALUE,
                 __FILE__, __LINE__, "invalid value for %s %u %u",
                 key.str, flags, len);
    }
  }
  return result;
}

/* phpmcc_get_helper retrieves the values from memcache.*/
static int phpmcc_get_helper(MccResourcePtr &phpmcc, nstring_t* keys,
                             unsigned int nkeys,
                             phpmcc_get_processor_funcptr_t func,
                             phpmcc_get_processor_context_t* context) {
  mcc_req_t** mirror_reqs = NULL, ** mirror_current = NULL;
  mcc_req_t* reqs = NULL, * current;
  unsigned int mirror_index;
  unsigned int reqs_returned = 0;

  reqs = mcc_get(phpmcc->m_mcc, keys, nkeys);
  size_t mirr_ct = phpmcc->m_mirror_mccs.size();
  /* propagate to mirrors */
  if (mirr_ct > 0) {
    mirror_reqs = (mcc_req_t**)malloc(sizeof(mcc_req_t*) * mirr_ct * 2);
    if (mirror_reqs == NULL) {
      free(mirror_reqs);
      return -1;
    }
    mirror_current = &mirror_reqs[mirr_ct];

    mirror_index = 0;
    for (MirrorIt mirror = phpmcc->m_mirror_mccs.begin();
         mirror != phpmcc->m_mirror_mccs.end(); ++mirror) {
      mirror_current[mirror_index] = mirror_reqs[mirror_index] =
        mcc_get((*mirror)->m_mcc, keys, nkeys);

      mcc_clear_err((*mirror)->m_mcc, NULL);
      mirror_index ++;
    }
  }

  for (current = reqs; current != NULL; current = TAILQ_NEXT(current, item),
         reqs_returned ++) {
    const nstring_t search_key = current->u.getreq.key;
    mcc_res_t primary_result = current->result;
    nstring_t primary_value = current->u.getreq.value;
    uint32_t primary_flags = current->u.getreq.flags;
    mcc_res_t final_result;
    Variant result;

    for (mirror_index = 0; mirror_index < mirr_ct; mirror_index ++) {
      for (mirror_current[mirror_index] = mirror_reqs[mirror_index];
           mirror_current[mirror_index] != NULL;
           mirror_current[mirror_index] =
             TAILQ_NEXT(mirror_current[mirror_index], item)) {
        if (nstring_cmp(&search_key,
                        &(mirror_current[mirror_index]->u.getreq.key)) == 0) {
          break;
        }
      }
    }

    final_result = resolve_value(phpmcc, search_key,
                                 primary_result, primary_value, primary_flags,
                                 mirror_current, result);
    func(search_key, final_result, result, context);
  }

  if (reqs_returned != nkeys) {
    phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_MISSINGS_REQS,
               __FILE__, __LINE__,
               "executed gets on %d keys but only received %d reqs",
               nkeys, reqs_returned);
  }

  if (reqs != NULL) {
    mcc_req_del(reqs);
  }

  for (mirror_index = 0; mirror_index < mirr_ct; mirror_index ++) {
    if (mirror_reqs[mirror_index] != NULL) {
      mcc_req_del(mirror_reqs[mirror_index]);
    }
  }
  if (mirror_reqs != NULL) {
    free(mirror_reqs);
  }

  return 0;
}

static void phpmcc_get(MccResourcePtr &phpmcc, Variant& retval,
                       CVarRef phpkeys,
                       phpmcc_get_details_t detailed_info_mode,
                       Variant& detailed_info) {
  nstring_t* keys = NULL;
  size_t nkeys = 0;

  phpmcc_get_processor_context_t context;
  phpmcc_get_processor_funcptr_t processor;

  if (nstring_array_new(keys, nkeys, phpkeys) == NULL) {
    goto epilogue;
  }

  context.hits = 0;
  context.errors = 0;
  if (phpkeys.is(KindOfArray)) {
    retval = Array::Create();
    context.results = &retval;

    // check to see if we have what we need to proceed.  if the arguments
    // do not check out, revert to normal multiget.
    switch (detailed_info_mode) {
    case PHPMCC_GET_RECORD_ERRORS:
      // if the variable is a null, then let's make it a value.
      if (detailed_info.isNull()) {
        detailed_info = Array::Create();
      }

      if (!detailed_info.is(KindOfArray)) {
        phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_DETAILED_BAD_ARG,
                   __FILE__, __LINE__,
                   "detailed info argument must be an array");
        detailed_info_mode = PHPMCC_GET_DEFAULT;
        break;
      }
      break;

    case PHPMCC_GET_DEFAULT:
      /* no special condition */
      break;

    default:
      phpmcc_log(phpmcc, MCC_APP_ERROR, PHPMCC_ERR_DETAILED_BAD_ARG,
                 __FILE__, __LINE__,
                 "detailed info mode is unknown (%d)", detailed_info_mode);
      detailed_info_mode = PHPMCC_GET_DEFAULT;
      break;
    }

    // if there was a problem with the arguments, we revert to a normal
    // multiget.  actually do the work here.
    switch (detailed_info_mode) {
    case PHPMCC_GET_RECORD_ERRORS:
      processor = phpmcc_multiget_record_errors_processor;
      context.additional_context = &detailed_info;
      break;

    case PHPMCC_GET_DEFAULT:
    default:
      processor = phpmcc_multiget_processor;
      context.additional_context = NULL;
      break;
    }
  } else {
    context.results = &retval;
    processor = phpmcc_get_processor;
  }

  if (phpmcc_get_helper(phpmcc, keys, nkeys, processor, &context) != 0) {
    goto epilogue;
  }

 epilogue:
  if (keys != NULL) {
    nstring_array_del(keys);
  }

  /* If we didn't get any results, check for errors */
  if ((context.errors > 0) || (context.hits == 0 && nkeys != 0)) {
    mcc_log_if_error(phpmcc);
  }

  if (phpmcc->m_events.size() > 0) {
    phpmcc_apevent_dispatcher(phpmcc);
  }
}

Variant c_phpmcc::t_multi_add(CArrRef keys_values,
                              int64 exptime /* = 0 */,
                              int64 compress /* = 1 */,
                              int64 proxy_replicate /* = 0 */,
                              int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::multi_add);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.madd", 1);
    ServerStats::Log("mcc.madd.count", keys_values.size());
  }
  return zim_phpmcc_multi_update(m_mcc, keys_values, exptime, compress,
                                 proxy_replicate, async_set, mcc_add_cmd);
}

Variant c_phpmcc::t_multi_replace(CArrRef keys_values,
                                  int64 exptime /* = 0 */,
                                  int64 compress /* = 1 */,
                                  int64 proxy_replicate /* = 0 */,
                                  int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::multi_replace);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.mreplace", 1);
    ServerStats::Log("mcc.mreplace.count", keys_values.size());
  }
  return zim_phpmcc_multi_update(m_mcc, keys_values, exptime, compress,
                                 proxy_replicate, async_set, mcc_replace_cmd);
}

Variant c_phpmcc::t_multi_set(CArrRef keys_values,
                              int64 exptime /* = 0 */,
                              int64 compress /* = 1 */,
                              int64 proxy_replicate /* = 0 */,
                              int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::multi_set);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.set", 1);
  }
  return zim_phpmcc_multi_update(m_mcc, keys_values, exptime, compress,
                                 proxy_replicate, async_set, mcc_set_cmd);
}

Variant c_phpmcc::t_add(CVarRef key,
                        CVarRef value,
                        int64 exptime /* = 0 */,
                        bool compress /* = 1 */,
                        int64 proxy_replicate /* = 0 */,
                        int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::add);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.add", 1);
  }
  return zim_phpmcc_update(m_mcc, key, value, exptime,
                           compress, proxy_replicate, async_set,
                           mcc_add_cmd);
}

Variant c_phpmcc::t_decr(CStrRef key, int64 value /* = 1 */) {
  FUNCTION_INJECTION(phpmcc::decr);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.decr", 1);
  }
  return zim_phpmcc_arith_update(m_mcc, key, value, mcc_decr_cmd);
}

Variant c_phpmcc::t_incr(CStrRef key, int64 value /* = 1 */) {
  FUNCTION_INJECTION(phpmcc::incr);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.incr", 1);
  }
  return zim_phpmcc_arith_update(m_mcc, key, value, mcc_incr_cmd);
}

Variant c_phpmcc::t_delete(CVarRef keys, int64 exptime /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::delete);
  if (!m_mcc.get()) {
    return null;
  }
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.delete", 1);
  }
  return phpmcc_delete(m_mcc, keys, exptime, false);
}

Variant c_phpmcc::t_delete_details(CVarRef keys, int64 exptime /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::delete_details);
  if (!m_mcc.get()) {
    return null;
  }
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.delete_details", 1);
  }
  return phpmcc_delete(m_mcc, keys, exptime, true);
}

Variant c_phpmcc::t_get(CVarRef keys, int64 detailed_info_mode /* = 0 */,
                        Variant detailed_info /* = null */) {
  FUNCTION_INJECTION(phpmcc::get);
  if (!m_mcc.get()) {
    return null;
  }
  if (detailed_info_mode == 0) detailed_info_mode = PHPMCC_GET_DEFAULT;
  Variant return_value;
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.get", 1);
  }
  phpmcc_get(m_mcc, return_value, keys,
             (phpmcc_get_details_t)detailed_info_mode, detailed_info);
  return return_value;
}

Variant c_phpmcc::t_get_multi(CVarRef keys, int64 detailed_info_mode /* = 0 */,
                              Variant detailed_info /* = null */) {
  FUNCTION_INJECTION(phpmcc::get_multi);
  if (!m_mcc.get()) {
    return null;
  }
  if (detailed_info_mode == 0) detailed_info_mode = PHPMCC_GET_DEFAULT;
  Variant return_value;
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.mget", 1);
    ServerStats::Log("mcc.mget.count", keys.toArray().size());
  }
  phpmcc_get(m_mcc, return_value, keys,
             (phpmcc_get_details_t)detailed_info_mode, detailed_info);
  return return_value;
}

Variant c_phpmcc::t_replace(CVarRef key,
                            CVarRef value,
                            int64 exptime /* = 0 */,
                            bool compress /* = 1 */,
                            int64 proxy_replicate /* = 0 */,
                            int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::replace);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.replace", 1);
  }
  return zim_phpmcc_update(m_mcc, key, value, exptime,
                           compress, proxy_replicate, async_set,
                           mcc_replace_cmd);
}

Variant c_phpmcc::t_set(CVarRef key,
                        CVarRef value,
                        int64 exptime /* = 0 */,
                        bool compress /* = 1 */,
                        int64 proxy_replicate /* = 0 */,
                        int64 async_set /* = 0 */) {
  FUNCTION_INJECTION(phpmcc::set);
  if (RuntimeOption::EnableStats && RuntimeOption::EnableMemcacheStats) {
    ServerStats::Log("mcc.set", 1);
  }
  return zim_phpmcc_update(m_mcc, key, value, exptime,
                           compress, proxy_replicate, async_set,
                           mcc_set_cmd);
}

///////////////////////////////////////////////////////////////////////////////
}
