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

#include "hphp/runtime/ext/ext_ldap.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/util/thread-local.h"
#include "folly/String.h"
#include <lber.h>

#define LDAP_DEPRECATED 1
#include <ldap.h>

#define PHP_LD_FULL_ADD 0xff

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(ldap);
///////////////////////////////////////////////////////////////////////////////

class LdapRequestData {
public:
  LdapRequestData() : m_num_links(0), m_max_links(-1) {
  }

  long m_num_links;
  long m_max_links;
};
static IMPLEMENT_THREAD_LOCAL(LdapRequestData, s_ldap_data);
#define LDAPG(name) s_ldap_data->m_ ## name

class LdapLink : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(LdapLink)

  LdapLink() : link(NULL) {}
  ~LdapLink() { close();}

  void close() {
    if (link) {
      ldap_unbind_s(link);
      link = NULL;
      LDAPG(num_links)--;
    }
    rebindproc.reset();
  }

  CLASSNAME_IS("ldap link");
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  LDAP *link;
  Variant rebindproc;
};
IMPLEMENT_OBJECT_ALLOCATION(LdapLink)

class LdapResult : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(LdapResult)

  LdapResult(LDAPMessage *res) : data(res) {}
  ~LdapResult() { close();}

  void close() {
    if (data) {
      ldap_msgfree(data);
      data = NULL;
    }
  }

  CLASSNAME_IS("ldap result");
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof();}

  LDAPMessage *data;
};
IMPLEMENT_OBJECT_ALLOCATION(LdapResult)

class LdapResultEntry : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(LdapResultEntry)

  LdapResultEntry(LDAPMessage *entry, ResourceData *res)
    : data(entry), ber(NULL), result(res) {}
  ~LdapResultEntry() { close();}

  void close() {
    if (ber != NULL) {
      ber_free(ber, 0);
      ber = NULL;
    }
    data = NULL;
  }

  CLASSNAME_IS("ldap result entry");
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  LDAPMessage *data;
  BerElement *ber;
  Resource result; // Reference to LdapResult to avoid premature deallocation
};

void LdapResultEntry::sweep() {
  close();
}

///////////////////////////////////////////////////////////////////////////////

static int _get_lderrno(LDAP *ldap) {
  int lderr;
  ldap_get_option(ldap, LDAP_OPT_ERROR_NUMBER, &lderr);
  return lderr;
}

static bool php_ldap_do_modify(CResRef link, const String& dn, CArrRef entry,
                               int oper) {
  bool is_full_add = false; /* flag for full add operation so ldap_mod_add
                               can be put back into oper, gerrit THomson */

  LdapLink *ld = link.getTyped<LdapLink>();

  int num_attribs = entry.size();
  LDAPMod **ldap_mods =
    (LDAPMod **)malloc((num_attribs+1) * sizeof(LDAPMod *));
  int *num_berval = (int*)malloc(num_attribs * sizeof(int));

  ArrayIter iter(entry);
  int num_values;

  /* added by gerrit thomson to fix ldap_add using ldap_mod_add */
  if (oper == PHP_LD_FULL_ADD) {
    oper = LDAP_MOD_ADD;
    is_full_add = true;
  }
  /* end additional , gerrit thomson */

  bool ret = false;
  Array stringHolder;
  for (int i = 0; i < num_attribs; i++) {
    ldap_mods[i] = (LDAPMod*)malloc(sizeof(LDAPMod));
    ldap_mods[i]->mod_op = oper | LDAP_MOD_BVALUES;
    ldap_mods[i]->mod_type = NULL;

    Variant key = iter.first();
    Variant value = iter.second();
    if (key.isString()) {
      ldap_mods[i]->mod_type = strdup(key.toString().data());
    } else {
      raise_warning("Unknown attribute in the data");
      /* Free allocated memory */
      while (i >= 0) {
        if (ldap_mods[i]->mod_type) {
          free(ldap_mods[i]->mod_type);
        }
        free(ldap_mods[i]);
        i--;
      }
      free(num_berval);
      free(ldap_mods);
      return false;
    }

    if (!value.isArray()) {
      num_values = 1;
    } else {
      num_values = value.toArray().size();
    }

    num_berval[i] = num_values;
    ldap_mods[i]->mod_bvalues =
      (struct berval**)malloc((num_values + 1) * sizeof(struct berval *));

    /* allow for arrays with one element, no allowance for arrays with
       none but probably not required, gerrit thomson. */
    if (num_values == 1 && !value.isArray()) {
      String svalue = value.toString();
      stringHolder.append(svalue);
      ldap_mods[i]->mod_bvalues[0] = (berval *)malloc(sizeof(struct berval));
      ldap_mods[i]->mod_bvalues[0]->bv_len = svalue.size();
      ldap_mods[i]->mod_bvalues[0]->bv_val = (char*)svalue.data();
    } else {
      Array arr = value.toArray();
      for (int j = 0; j < num_values; j++) {
        if (!arr.exists(j)) {
          raise_warning("Value array must have consecutive indices 0, 1, ...");
          num_berval[i] = j;
          num_attribs = i + 1;
          goto errexit;
        }
        String ivalue = arr[j].toString();
        ldap_mods[i]->mod_bvalues[j] = (berval *)malloc(sizeof(struct berval));
        ldap_mods[i]->mod_bvalues[j]->bv_len = ivalue.size();
        ldap_mods[i]->mod_bvalues[j]->bv_val = (char*)ivalue.data();
      }
    }
    ldap_mods[i]->mod_bvalues[num_values] = NULL;
    ++iter;
  }
  ldap_mods[num_attribs] = NULL;

  /* check flag to see if do_mod was called to perform full add,
     gerrit thomson */
  int rc;
  if (is_full_add) {
    if ((rc = ldap_add_s(ld->link, (char*)dn.data(), ldap_mods))
        != LDAP_SUCCESS) {
      raise_warning("Add: %s", ldap_err2string(rc));
    } else {
      ret = true;
    }
  } else {
    if ((rc = ldap_modify_s(ld->link, (char*)dn.data(), ldap_mods))
        != LDAP_SUCCESS) {
      raise_warning("Modify: %s", ldap_err2string(rc));
    } else {
      ret = true;
    }
  }

errexit:
  for (int i = 0; i < num_attribs; i++) {
    free(ldap_mods[i]->mod_type);
    for (int j = 0; j < num_berval[i]; j++) {
      free(ldap_mods[i]->mod_bvalues[j]);
    }
    free(ldap_mods[i]->mod_bvalues);
    free(ldap_mods[i]);
  }
  free(num_berval);
  free(ldap_mods);

  return ret;
}

static void php_set_opts(LDAP *ldap, int sizelimit, int timelimit, int deref,
                         int *old_sizelimit, int *old_timelimit,
                         int *old_deref) {
  /* sizelimit */
  if (sizelimit > -1) {
    ldap_get_option(ldap, LDAP_OPT_SIZELIMIT, old_sizelimit);
    ldap_set_option(ldap, LDAP_OPT_SIZELIMIT, &sizelimit);
  }

  /* timelimit */
  if (timelimit > -1) {
    ldap_get_option(ldap, LDAP_OPT_SIZELIMIT, old_timelimit);
    ldap_set_option(ldap, LDAP_OPT_TIMELIMIT, &timelimit);
  }

  /* deref */
  if (deref > -1) {
    ldap_get_option(ldap, LDAP_OPT_SIZELIMIT, old_deref);
    ldap_set_option(ldap, LDAP_OPT_DEREF, &deref);
  }
}

static Variant php_ldap_do_search(CVarRef link, CVarRef base_dn,
                                  CVarRef filter, CArrRef attributes,
                                  int attrsonly, int sizelimit, int timelimit,
                                  int deref, int scope) {
  int num_attribs = attributes.size();
  int old_sizelimit = -1, old_timelimit = -1, old_deref = -1;
  int ldap_err = 1, parallel_search = 1;
  char **ldap_attrs = (char**)malloc((num_attribs+1) * sizeof(char *));
  Array stringHolder;
  Array ret = Array::Create();

  char *ldap_base_dn = NULL;
  char *ldap_filter = NULL;
  LdapLink *ld = NULL;


  for (int i = 0; i < num_attribs; i++) {
    if (!attributes.exists(i)) {
      raise_warning("Array initialization wrong");
      ldap_err = 0;
      goto cleanup;
    }
    String attr = attributes[i].toString();
    stringHolder.append(attr);
    ldap_attrs[i] = (char*)attr.data();
  }
  ldap_attrs[num_attribs] = NULL;

  /* parallel search? */
  if (link.isArray()) {
    int nlinks = link.toArray().size();
    if (nlinks == 0) {
      raise_warning("No links in link array");
      ldap_err = 0;
      goto cleanup;
    }

    int nbases;
    if (base_dn.isArray()) {
      nbases = base_dn.toArray().size();
      if (nbases != nlinks) {
        raise_warning("Base must either be a string, or an array with the "
                      "same number of elements as the links array");
        ldap_err = 0;
        goto cleanup;
      }
    } else {
      nbases = 0; /* this means string, not array */
      /* If anything else than string is passed, ldap_base_dn = NULL */
      if (base_dn.isString()) {
        ldap_base_dn = (char*)base_dn.toString().data();
      } else {
        ldap_base_dn = NULL;
      }
    }

    int nfilters;
    if (filter.isArray()) {
      nfilters = filter.toArray().size();
      if (nfilters != nlinks) {
        raise_warning("Filter must either be a string, or an array with the "
                      "same number of elements as the links array");
        ldap_err = 0;
        goto cleanup;
      }
    } else {
      nfilters = 0; /* this means string, not array */
      String sfilter = filter.toString();
      stringHolder.append(sfilter);
      ldap_filter = (char*)sfilter.data();
    }

    LdapLink **lds = (LdapLink**)malloc(nlinks * sizeof(LdapLink*));
    int *rcs = (int*)malloc(nlinks * sizeof(int));

    ArrayIter iter(link.toArray());
    ArrayIter iterdn(base_dn.toArray());
    ArrayIter iterfilter(filter.toArray());
    for (int i = 0; i < nlinks; i++) {
      ld = iter.second().toResource().getTyped<LdapLink>(true, true);
      if (ld == NULL) {
        ldap_err = 0;
        goto cleanup_parallel;
      }
      if (nbases != 0) { /* base_dn an array? */
        Variant entry = iterdn.second();
        ++iterdn;

        /* If anything else than string is passed, ldap_base_dn = NULL */
        if (entry.isString()) {
          ldap_base_dn = (char*)entry.toString().data();
        } else {
          ldap_base_dn = NULL;
        }
      }
      if (nfilters != 0) { /* filter an array? */
        Variant entry = iterfilter.second();
        ++iterfilter;
        String sentry = entry.toString();
        stringHolder.append(sentry);
        ldap_filter = (char*)sentry.data();
      }

      php_set_opts(ld->link, sizelimit, timelimit, deref, &old_sizelimit,
                   &old_timelimit, &old_deref);

      /* Run the actual search */
      rcs[i] = ldap_search(ld->link, ldap_base_dn, scope, ldap_filter,
                           ldap_attrs, attrsonly);
      lds[i] = ld;
      ++iter;
    }

    /* Collect results from the searches */
    for (int i = 0; i < nlinks; i++) {
      LDAPMessage *ldap_res;
      if (rcs[i] != -1) {
        rcs[i] = ldap_result(lds[i]->link, LDAP_RES_ANY, 1 /* LDAP_MSG_ALL */,
                             NULL, &ldap_res);
      }
      if (rcs[i] != -1) {
        ret.append(Resource(NEWOBJ(LdapResult)(ldap_res)));
      } else {
        ret.append(false);
      }
    }

cleanup_parallel:
    free(lds);
    free(rcs);
  } else {
    /* parallel search? */
    String sfilter = filter.toString();
    ldap_filter = (char*)sfilter.data();

    /* If anything else than string is passed, ldap_base_dn = NULL */
    if (base_dn.isString()) {
      ldap_base_dn = (char*)base_dn.toString().data();
    }

    ld = link.toResource().getTyped<LdapLink>(true, true);
    if (ld == NULL) {
      ldap_err = 0;
      goto cleanup;
    }

    php_set_opts(ld->link, sizelimit, timelimit, deref, &old_sizelimit,
                 &old_timelimit, &old_deref);

    /* Run the actual search */
    LDAPMessage *ldap_res;
    int rc = ldap_search_s(ld->link, ldap_base_dn, scope, ldap_filter,
                           ldap_attrs, attrsonly, &ldap_res);

    if (rc != LDAP_SUCCESS && rc != LDAP_SIZELIMIT_EXCEEDED
  #ifdef LDAP_ADMINLIMIT_EXCEEDED
        && rc != LDAP_ADMINLIMIT_EXCEEDED
  #endif
  #ifdef LDAP_REFERRAL
        && rc != LDAP_REFERRAL
  #endif
    ) {
      raise_warning("Search: %s", ldap_err2string(rc));
      ldap_err = 0;
    } else {
      if (rc == LDAP_SIZELIMIT_EXCEEDED) {
        raise_warning("Partial search results returned: Sizelimit exceeded");
      }
#ifdef LDAP_ADMINLIMIT_EXCEEDED
      else if (rc == LDAP_ADMINLIMIT_EXCEEDED) {
        raise_warning("Partial search results returned: Adminlimit exceeded");
      }
#endif
      parallel_search = 0;
      ret.append(Resource(NEWOBJ(LdapResult)(ldap_res)));
    }
  }
cleanup:
  if (ld) {
    /* Restoring previous options */
    php_set_opts(ld->link, old_sizelimit, old_timelimit, old_deref, &sizelimit,
                 &timelimit, &deref);
  }
  if (ldap_attrs != NULL) {
    free(ldap_attrs);
  }

  if (!ldap_err) {
    return false;
  }

  if (!parallel_search) {
    return ret.dequeue();
  } else {
    return ret;
  }
}

static int _ldap_rebind_proc(LDAP *ldap, const char *url, ber_tag_t req,
                             ber_int_t msgid, void *params) {
  LdapLink *ld = (LdapLink*)params;

  /* link exists and callback set? */
  if (ld == NULL || ld->rebindproc.isNull()) {
    raise_warning("Link not found or no callback set");
    return LDAP_OTHER;
  }

  /* callback */
  Variant ret = vm_call_user_func
    (ld->rebindproc, make_packed_array(Resource(ld), String(url, CopyString)));
  return ret.toInt64();
}

const StaticString
  s_count("count"),
  s_dn("dn");

static void get_attributes(Array &ret, LDAP *ldap,
                           LDAPMessage *ldap_result_entry, bool to_lower) {
  int num_attrib = 0;
  BerElement *ber;
  char *attribute = ldap_first_attribute(ldap, ldap_result_entry, &ber);

  while (attribute != NULL) {
    struct berval **ldap_value =
      ldap_get_values_len(ldap, ldap_result_entry, attribute);
    int num_values = ldap_count_values_len(ldap_value);

    Array tmp;
    tmp.set(s_count, num_values);
    for (int i = 0; i < num_values; i++) {
      tmp.append(String(ldap_value[i]->bv_val, ldap_value[i]->bv_len,
                        CopyString));
    }
    ldap_value_free_len(ldap_value);

    String sAttribute(attribute, CopyString);
    ret.set(to_lower ? String(Util::toLower(attribute)) : sAttribute, tmp);
    ret.set(num_attrib, sAttribute);

    num_attrib++;
    ldap_memfree(attribute);
    attribute = ldap_next_attribute(ldap, ldap_result_entry, ber);
  }

  if (ber != NULL) {
    ber_free(ber, 0);
  }

  ret.set(s_count, num_attrib);
}

///////////////////////////////////////////////////////////////////////////////

Variant f_ldap_connect(const String& hostname /* = null_string */,
                       int port /* = 389 */) {
  if (LDAPG(max_links) != -1 && LDAPG(num_links) >= LDAPG(max_links)) {
    raise_warning("Too many open links (%ld)", LDAPG(num_links));
    return false;
  }

  LdapLink *ld = NEWOBJ(LdapLink)();
  Resource ret(ld);

  LDAP *ldap = NULL;
  if (!hostname.empty() && hostname.find('/') >= 0) {
    int rc = ldap_initialize(&ldap, hostname.data());
    if (rc != LDAP_SUCCESS) {
      raise_warning("Could not create session handle: %s",
                    ldap_err2string(rc));
      return false;
    }
  } else {
    ldap = ldap_init((char*)hostname.data(), port);
  }

  if (ldap) {
    LDAPG(num_links)++;
    ld->link = ldap;
    return ret;
  }
  raise_warning("Unable to initialize LDAP: %s",
                folly::errnoStr(errno).c_str());
  return false;
}

Variant f_ldap_explode_dn(const String& dn, int with_attrib) {
  char **ldap_value;
  if (!(ldap_value = ldap_explode_dn((char*)dn.data(), with_attrib))) {
    /* Invalid parameters were passed to ldap_explode_dn */
    return false;
  }

  int i = 0;
  while (ldap_value[i] != NULL) i++;
  int count = i;

  Array ret;
  ret.set(s_count, count);
  for (i = 0; i < count; i++) {
    ret.append(String(ldap_value[i], CopyString));
  }

  ldap_value_free(ldap_value);
  return ret;
}

Variant f_ldap_dn2ufn(const String& db) {
  char *ufn = ldap_dn2ufn((char*)db.data());
  if (ufn) {
    String ret(ufn, CopyString);
    ldap_memfree(ufn);
    return ret;
  }
  return false;
}

String f_ldap_err2str(int errnum) {
  return String(ldap_err2string(errnum), CopyString);
}

bool f_ldap_add(CResRef link, const String& dn, CArrRef entry) {
  return php_ldap_do_modify(link, dn, entry, PHP_LD_FULL_ADD);
}

bool f_ldap_mod_add(CResRef link, const String& dn, CArrRef entry) {
  return php_ldap_do_modify(link, dn, entry, LDAP_MOD_ADD);
}

bool f_ldap_mod_del(CResRef link, const String& dn, CArrRef entry) {
  return php_ldap_do_modify(link, dn, entry, LDAP_MOD_DELETE);
}

bool f_ldap_mod_replace(CResRef link, const String& dn, CArrRef entry) {
  return php_ldap_do_modify(link, dn, entry, LDAP_MOD_REPLACE);
}

bool f_ldap_modify(CResRef link, const String& dn, CArrRef entry) {
  return php_ldap_do_modify(link, dn, entry, LDAP_MOD_REPLACE);
}

bool f_ldap_bind(CResRef link, const String& bind_rdn /* = null_string */,
                 const String& bind_password /* = null_string */) {
  int rc;
  LdapLink *ld = link.getTyped<LdapLink>();
  if ((rc = ldap_bind_s(ld->link, (char*)bind_rdn.data(),
                        (char*)bind_password.data(),
                        LDAP_AUTH_SIMPLE)) != LDAP_SUCCESS) {
    raise_warning("Unable to bind to server: %s", ldap_err2string(rc));
    return false;
  }
  return true;
}

bool f_ldap_set_rebind_proc(CResRef link, CVarRef callback) {
  LdapLink *ld = link.getTyped<LdapLink>();

  if (callback.isString() && callback.toString().empty()) {
    /* unregister rebind procedure */
    if (!ld->rebindproc.isNull()) {
      ld->rebindproc.reset();
      ldap_set_rebind_proc(ld->link, NULL, NULL);
    }
    return true;
  }

  /* callable? */
  if (!f_is_callable(callback)) {
    raise_warning("Callback argument is not a valid callback");
    return false;
  }

  /* register rebind procedure */
  if (ld->rebindproc.isNull()) {
    ldap_set_rebind_proc(ld->link, _ldap_rebind_proc, (void *)link.get());
  } else {
    ld->rebindproc.reset();
  }

  ld->rebindproc = callback;
  return true;
}

bool f_ldap_sort(CResRef link, CResRef result, const String& sortfilter) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResult *res = result.getTyped<LdapResult>();

  if (ldap_sort_entries(ld->link, &res->data,
                        !sortfilter.empty() ? (char*)sortfilter.data() : NULL,
                        strcmp) != LDAP_SUCCESS) {
    raise_warning("%s", ldap_err2string(_get_lderrno(ld->link)));
    return false;
  }
  return true;
}

bool f_ldap_start_tls(CResRef link) {
  LdapLink *ld = link.getTyped<LdapLink>();
  int rc, protocol = LDAP_VERSION3;
  if (((rc = ldap_set_option(ld->link, LDAP_OPT_PROTOCOL_VERSION, &protocol))
       != LDAP_SUCCESS) ||
      ((rc = ldap_start_tls_s(ld->link, NULL, NULL)) != LDAP_SUCCESS)) {
    raise_warning("Unable to start TLS: %s", ldap_err2string(rc));
    return false;
  }
  return true;
}

bool f_ldap_unbind(CResRef link) {
  LdapLink *ld = link.getTyped<LdapLink>();
  ld->close();
  return true;
}

bool f_ldap_get_option(CResRef link, int option, VRefParam retval) {
  LdapLink *ld = link.getTyped<LdapLink>();

  switch (option) {
  /* options with int value */
  case LDAP_OPT_DEREF:
  case LDAP_OPT_SIZELIMIT:
  case LDAP_OPT_TIMELIMIT:
  case LDAP_OPT_PROTOCOL_VERSION:
  case LDAP_OPT_ERROR_NUMBER:
  case LDAP_OPT_REFERRALS:
#ifdef LDAP_OPT_RESTART
  case LDAP_OPT_RESTART:
#endif
    {
      int val;
      if (ldap_get_option(ld->link, option, &val)) {
        return false;
      }
      retval = (int64_t)val;
    } break;
#ifdef LDAP_OPT_NETWORK_TIMEOUT
  case LDAP_OPT_NETWORK_TIMEOUT:
    {
      struct timeval *timeout;
      if (ldap_get_option(ld->link, LDAP_OPT_NETWORK_TIMEOUT,
                          (void *) &timeout)) {
        if (timeout) {
          ldap_memfree(timeout);
        }
        return false;
      }
      retval = (int64_t)timeout->tv_sec;
      ldap_memfree(timeout);
    } break;
#elif defined(LDAP_X_OPT_CONNECT_TIMEOUT)
  case LDAP_X_OPT_CONNECT_TIMEOUT:
    {
      int timeout;
      if (ldap_get_option(ld->link, LDAP_X_OPT_CONNECT_TIMEOUT, &timeout)) {
        return false;
      }
      retval = (int64_t)(timeout / 1000);
    } break;
#endif
  /* options with string value */
  case LDAP_OPT_ERROR_STRING:
#ifdef LDAP_OPT_HOST_NAME
  case LDAP_OPT_HOST_NAME:
#endif
#ifdef HAVE_LDAP_SASL
  case LDAP_OPT_X_SASL_MECH:
  case LDAP_OPT_X_SASL_REALM:
  case LDAP_OPT_X_SASL_AUTHCID:
  case LDAP_OPT_X_SASL_AUTHZID:
#endif
#ifdef LDAP_OPT_MATCHED_DN
  case LDAP_OPT_MATCHED_DN:
#endif
    {
      char *val = NULL;
      if (ldap_get_option(ld->link, option, &val) || val == NULL ||
          *val == '\0') {
        if (val) {
          ldap_memfree(val);
        }
        return false;
      }
      retval = String(val, CopyString);
      ldap_memfree(val);
    } break;
/* options not implemented
  case LDAP_OPT_SERVER_CONTROLS:
  case LDAP_OPT_CLIENT_CONTROLS:
  case LDAP_OPT_API_INFO:
  case LDAP_OPT_API_FEATURE_INFO:
*/
  default:
    return false;
  }
  return true;
}

const StaticString
  s_oid("oid"),
  s_value("value"),
  s_iscritical("iscritical");

bool f_ldap_set_option(CVarRef link, int option, CVarRef newval) {
  LDAP *ldap = NULL;
  if (!link.isNull()) {
    LdapLink *ld = link.toResource().getTyped<LdapLink>();
    ldap = ld->link;
  }

  switch (option) {
  /* options with int value */
  case LDAP_OPT_DEREF:
  case LDAP_OPT_SIZELIMIT:
  case LDAP_OPT_TIMELIMIT:
  case LDAP_OPT_PROTOCOL_VERSION:
  case LDAP_OPT_ERROR_NUMBER:
#ifdef LDAP_OPT_DEBUG_LEVEL
  case LDAP_OPT_DEBUG_LEVEL:
#endif
    {
      int val = newval.toInt64();
      if (ldap_set_option(ldap, option, &val)) {
        return false;
      }
    } break;
#ifdef LDAP_OPT_NETWORK_TIMEOUT
  case LDAP_OPT_NETWORK_TIMEOUT:
    {
      struct timeval timeout;
      timeout.tv_sec = newval.toInt64();
      timeout.tv_usec = 0;
      if (ldap_set_option(ldap, LDAP_OPT_NETWORK_TIMEOUT, (void *) &timeout)) {
        return false;
      }
    } break;
#elif defined(LDAP_X_OPT_CONNECT_TIMEOUT)
  case LDAP_X_OPT_CONNECT_TIMEOUT:
    {
      int timeout = 1000 * newval.toInt64(); /* Convert to milliseconds */
      if (ldap_set_option(ldap, LDAP_X_OPT_CONNECT_TIMEOUT, &timeout)) {
        return false;
      }
    } break;
#endif
    /* options with string value */
  case LDAP_OPT_ERROR_STRING:
#ifdef LDAP_OPT_HOST_NAME
  case LDAP_OPT_HOST_NAME:
#endif
#ifdef HAVE_LDAP_SASL
  case LDAP_OPT_X_SASL_MECH:
  case LDAP_OPT_X_SASL_REALM:
  case LDAP_OPT_X_SASL_AUTHCID:
  case LDAP_OPT_X_SASL_AUTHZID:
#endif
#ifdef LDAP_OPT_MATCHED_DN
  case LDAP_OPT_MATCHED_DN:
#endif
    {
      String snewval = newval.toString();
      char *val = (char*)snewval.data();
      if (ldap_set_option(ldap, option, val)) {
        return false;
      }
    } break;
    /* options with boolean value */
  case LDAP_OPT_REFERRALS:
#ifdef LDAP_OPT_RESTART
  case LDAP_OPT_RESTART:
#endif
    {
      void *val = newval.toBoolean() ? LDAP_OPT_ON : LDAP_OPT_OFF;
      if (ldap_set_option(ldap, option, val)) {
        return false;
      }
    } break;
    /* options with control list value */
  case LDAP_OPT_SERVER_CONTROLS:
  case LDAP_OPT_CLIENT_CONTROLS:
    {
      LDAPControl *ctrl, **ctrls, **ctrlp;
      int ncontrols;
      char error=0;

      if (!newval.isArray() || !(ncontrols = newval.toArray().size())) {
        raise_warning("Expected non-empty array value for this option");
        return false;
      }
      ctrls = (LDAPControl**)malloc((1 + ncontrols) * sizeof(*ctrls));
      *ctrls = NULL;
      ctrlp = ctrls;
      Array stringHolder;
      for (ArrayIter iter(newval.toArray()); iter; ++iter) {
        Variant vctrlval = iter.second();
        if (!vctrlval.isArray()) {
          raise_warning("The array value must contain only arrays, "
                        "where each array is a control");
          error = 1;
          break;
        }
        Array ctrlval = vctrlval.toArray();
        if (!ctrlval.exists(s_oid)) {
          raise_warning("Control must have an oid key");
          error = 1;
          break;
        }
        String val = ctrlval[s_oid].toString();
        stringHolder.append(val);
        ctrl = *ctrlp = (LDAPControl*)malloc(sizeof(**ctrlp));
        ctrl->ldctl_oid = (char*)val.data();
        if (ctrlval.exists(s_value)) {
          val = ctrlval[s_value].toString();
          stringHolder.append(val);
          ctrl->ldctl_value.bv_val = (char*)val.data();
          ctrl->ldctl_value.bv_len = val.size();
        } else {
          ctrl->ldctl_value.bv_val = NULL;
          ctrl->ldctl_value.bv_len = 0;
        }
        if (ctrlval.exists(s_iscritical)) {
          ctrl->ldctl_iscritical = val.toBoolean() ? 1 : 0;
        } else {
          ctrl->ldctl_iscritical = 0;
        }

        ++ctrlp;
        *ctrlp = NULL;
      }
      if (!error) {
        error = ldap_set_option(ldap, option, ctrls);
      }
      ctrlp = ctrls;
      while (*ctrlp) {
        free(*ctrlp);
        ctrlp++;
      }
      free(ctrls);
      if (error) {
        return false;
      }
    } break;
  default:
    return false;
  }
  return true;
}

bool f_ldap_close(CResRef link) {
  return f_ldap_unbind(link);
}

Variant f_ldap_list(CVarRef link, CVarRef base_dn, CVarRef filter,
                    CArrRef attributes /* = null_array */,
                    int attrsonly /* = 0 */, int sizelimit /* = -1 */,
                    int timelimit /* = -1 */, int deref /* = -1 */) {
  return php_ldap_do_search(link, base_dn, filter, attributes, attrsonly,
                            sizelimit, timelimit, deref, LDAP_SCOPE_ONELEVEL);
}

Variant f_ldap_read(CVarRef link, CVarRef base_dn, CVarRef filter,
                    CArrRef attributes /* = null_array */,
                    int attrsonly /* = 0 */, int sizelimit /* = -1 */,
                    int timelimit /* = -1 */, int deref /* = -1 */) {
  return php_ldap_do_search(link, base_dn, filter, attributes, attrsonly,
                            sizelimit, timelimit, deref, LDAP_SCOPE_BASE);
}

Variant f_ldap_search(CVarRef link, CVarRef base_dn, CVarRef filter,
                      CArrRef attributes /* = null_array */,
                      int attrsonly /* = 0 */, int sizelimit /* = -1 */,
                      int timelimit /* = -1 */, int deref /* = -1 */) {
  return php_ldap_do_search(link, base_dn, filter, attributes, attrsonly,
                            sizelimit, timelimit, deref, LDAP_SCOPE_SUBTREE);
}

bool f_ldap_rename(CResRef link, const String& dn, const String& newrdn,
                   const String& newparent,
                   bool deleteoldrdn) {
  LdapLink *ld = link.getTyped<LdapLink>();
  int rc = ldap_rename_s(ld->link, (char*)dn.data(), (char*)newrdn.data(),
                         !newparent.empty() ? (char*)newparent.data() : NULL,
                         deleteoldrdn, NULL, NULL);
  return rc == LDAP_SUCCESS;
}

bool f_ldap_delete(CResRef link, const String& dn) {
  LdapLink *ld = link.getTyped<LdapLink>();
  int rc;
  if ((rc = ldap_delete_s(ld->link, (char*)dn.data())) != LDAP_SUCCESS) {
    raise_warning("Delete: %s", ldap_err2string(rc));
    return false;
  }
  return true;
}

Variant f_ldap_compare(CResRef link, const String& dn, const String& attribute,
                       const String& value) {
  LdapLink *ld = link.getTyped<LdapLink>();
  int rc = ldap_compare_s(ld->link, (char*)dn.data(), (char*)attribute.data(),
                          (char*)value.data());
  switch (rc) {
  case LDAP_COMPARE_TRUE:  return true;
  case LDAP_COMPARE_FALSE: return false;
  }
  raise_warning("Compare: %s", ldap_err2string(rc));
  return -1LL;
}

int64_t f_ldap_errno(CResRef link) {
  LdapLink *ld = link.getTyped<LdapLink>();
  return _get_lderrno(ld->link);
}

String f_ldap_error(CResRef link) {
  LdapLink *ld = link.getTyped<LdapLink>();
  int ld_errno = _get_lderrno(ld->link);
  return String(ldap_err2string(ld_errno), CopyString);
}

Variant f_ldap_get_dn(CResRef link, CResRef result_entry) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResultEntry *entry = result_entry.getTyped<LdapResultEntry>();

  char *text = ldap_get_dn(ld->link, entry->data);
  if (text) {
    String ret(text, CopyString);
    ldap_memfree(text);
    return ret;
  }
  return false;
}

int64_t f_ldap_count_entries(CResRef link, CResRef result) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResult *res = result.getTyped<LdapResult>();
  return ldap_count_entries(ld->link, res->data);
}

Variant f_ldap_get_entries(CResRef link, CResRef result) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResult *res = result.getTyped<LdapResult>();

  LDAP *ldap = ld->link;

  int num_entries = ldap_count_entries(ldap, res->data);
  Array ret;
  ret.set(s_count, num_entries);
  if (num_entries == 0) {
    return uninit_null();
  }

  LDAPMessage *ldap_result_entry = ldap_first_entry(ldap, res->data);
  if (ldap_result_entry == NULL) {
    return false;
  }

  num_entries = 0;
  while (ldap_result_entry != NULL) {
    Array tmp1 = Array::Create();
    get_attributes(tmp1, ldap, ldap_result_entry, true);

    char *dn = ldap_get_dn(ldap, ldap_result_entry);
    tmp1.set(s_dn, String(dn, CopyString));
    ldap_memfree(dn);

    ret.set(num_entries, tmp1);

    num_entries++;
    ldap_result_entry = ldap_next_entry(ldap, ldap_result_entry);
  }

  ret.set(s_count, num_entries);
  return ret;
}

Variant f_ldap_first_entry(CResRef link, CResRef result) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResult *res = result.getTyped<LdapResult>();

  LDAPMessage *entry;
  if ((entry = ldap_first_entry(ld->link, res->data)) == NULL) {
    return false;
  }

  return NEWOBJ(LdapResultEntry)(entry, res);
}

Variant f_ldap_next_entry(CResRef link, CResRef result_entry) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResultEntry *entry = result_entry.getTyped<LdapResultEntry>();

  LDAPMessage *msg;
  if ((msg = ldap_next_entry(ld->link, entry->data)) == NULL) {
    return false;
  }

  return NEWOBJ(LdapResultEntry)(msg, entry->result.get());
}

Array f_ldap_get_attributes(CResRef link, CResRef result_entry) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResultEntry *entry = result_entry.getTyped<LdapResultEntry>();
  Array ret = Array::Create();
  get_attributes(ret, ld->link, entry->data, false);
  return ret;
}

Variant f_ldap_first_attribute(CResRef link, CResRef result_entry) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResultEntry *entry = result_entry.getTyped<LdapResultEntry>();

  char *attribute;
  if ((attribute =
       ldap_first_attribute(ld->link, entry->data, &entry->ber)) == NULL) {
    return false;
  }
  String ret(attribute, CopyString);
  ldap_memfree(attribute);
  return ret;
}

Variant f_ldap_next_attribute(CResRef link, CResRef result_entry) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResultEntry *entry = result_entry.getTyped<LdapResultEntry>();

  if (entry->ber == NULL) {
    raise_warning("called before calling ldap_first_attribute() or "
                  "no attributes found in result entry");
    return false;
  }

  char *attribute;
  if ((attribute =
       ldap_next_attribute(ld->link, entry->data, entry->ber)) == NULL) {
    if (entry->ber != NULL) {
      ber_free(entry->ber, 0);
      entry->ber = NULL;
    }
    return false;
  }
  String ret(attribute, CopyString);
  ldap_memfree(attribute);
  return ret;
}

Variant f_ldap_first_reference(CResRef link, CResRef result) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResult *res = result.getTyped<LdapResult>();

  LDAPMessage *entry;
  if ((entry = ldap_first_reference(ld->link, res->data)) == NULL) {
    return false;
  }

  return NEWOBJ(LdapResultEntry)(entry, res);
}

Variant f_ldap_next_reference(CResRef link, CResRef result_entry) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResultEntry *entry = result_entry.getTyped<LdapResultEntry>();

  LDAPMessage *entry_next;
  if ((entry_next = ldap_next_reference(ld->link, entry->data)) == NULL) {
    return false;
  }

  return NEWOBJ(LdapResultEntry)(entry_next, entry->result.get());
}

bool f_ldap_parse_reference(CResRef link, CResRef result_entry,
                            VRefParam referrals) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResultEntry *entry = result_entry.getTyped<LdapResultEntry>();

  char **lreferrals, **refp;
  if (ldap_parse_reference(ld->link, entry->data, &lreferrals,
                           NULL /* &serverctrls */, 0) != LDAP_SUCCESS) {
    return false;
  }

  Array arr = Array::Create();
  if (lreferrals != NULL) {
    refp = lreferrals;
    while (*refp) {
      arr.append(String(*refp, CopyString));
      refp++;
    }
    ldap_value_free(lreferrals);
  }
  referrals = arr;
  return true;
}

bool f_ldap_parse_result(CResRef link, CResRef result, VRefParam errcode,
                         VRefParam matcheddn /* = null */,
                         VRefParam errmsg /* = null */,
                         VRefParam referrals /* = null */) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResult *res = result.getTyped<LdapResult>();

  int lerrcode;
  char **lreferrals, **refp;
  char *lmatcheddn, *lerrmsg;
  int rc = ldap_parse_result(ld->link, res->data, &lerrcode,
                             &lmatcheddn, &lerrmsg, &lreferrals,
                             NULL /* &serverctrls */, 0);
  if (rc != LDAP_SUCCESS) {
    raise_warning("Unable to parse result: %s", ldap_err2string(rc));
    return false;
  }

  errcode = lerrcode;

  /* Reverse -> fall through */
  Array arr = Array::Create();
  if (lreferrals != NULL) {
    refp = lreferrals;
    while (*refp) {
      arr.append(String(*refp, CopyString));
      refp++;
    }
    ldap_value_free(lreferrals);
  }
  referrals = arr;

  if (lerrmsg == NULL) {
    errmsg = String("");
  } else {
    errmsg = String(lerrmsg, CopyString);
    ldap_memfree(lerrmsg);
  }

  if (lmatcheddn == NULL) {
    matcheddn = String("");
  } else {
    matcheddn = String(lmatcheddn, CopyString);
    ldap_memfree(lmatcheddn);
  }
  return true;
}

bool f_ldap_free_result(CResRef result) {
  LdapResult *res = result.getTyped<LdapResult>();
  res->close();
  return true;
}

Variant f_ldap_get_values_len(CResRef link, CResRef result_entry,
                              const String& attribute) {
  LdapLink *ld = link.getTyped<LdapLink>();
  LdapResultEntry *entry = result_entry.getTyped<LdapResultEntry>();

  struct berval **ldap_value_len;
  if ((ldap_value_len =
       ldap_get_values_len(ld->link, entry->data,
                           (char*)attribute.data())) == NULL) {
    raise_warning("Cannot get the value(s) of attribute %s",
                  ldap_err2string(_get_lderrno(ld->link)));
    return false;
  }

  int num_values = ldap_count_values_len(ldap_value_len);
  Array ret;
  for (int i = 0; i < num_values; i++) {
    ret.append(String(ldap_value_len[i]->bv_val, ldap_value_len[i]->bv_len,
                      CopyString));
  }
  ret.set(s_count, num_values);
  ldap_value_free_len(ldap_value_len);
  return ret;
}

Variant f_ldap_get_values(CResRef link, CResRef result_entry,
                          const String& attribute) {
  return f_ldap_get_values_len(link, result_entry, attribute);
}

///////////////////////////////////////////////////////////////////////////////
}
