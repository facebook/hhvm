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
*/

#include "hphp/runtime/ext/ldap/ext_ldap.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/root-map.h"
#include <folly/String.h>
#include <lber.h>
#include "hphp/util/rds-local.h"
#include "hphp/util/text-util.h"

#define LDAP_DEPRECATED 1
#include <ldap.h>

#include <folly/portability/SysTime.h>

#define PHP_LD_FULL_ADD 0xff

namespace HPHP {

const int64_t
  k_LDAP_ESCAPE_FILTER  = 1<<0,
  k_LDAP_ESCAPE_DN      = 1<<1;

#define LDAP_MODIFY_BATCH_ADD 0x01
#define LDAP_MODIFY_BATCH_REMOVE 0x02
#define LDAP_MODIFY_BATCH_REMOVE_ALL 0x12
#define LDAP_MODIFY_BATCH_REPLACE 0x03
#define LDAP_MODIFY_BATCH_ATTRIB "attrib"
#define LDAP_MODIFY_BATCH_MODTYPE "modtype"
#define LDAP_MODIFY_BATCH_VALUES "values"

static const StaticString
  s_LDAP_MODIFY_BATCH_ATTRIB(LDAP_MODIFY_BATCH_ATTRIB),
  s_LDAP_MODIFY_BATCH_MODTYPE(LDAP_MODIFY_BATCH_MODTYPE),
  s_LDAP_MODIFY_BATCH_VALUES(LDAP_MODIFY_BATCH_VALUES);

static struct LdapExtension final : Extension {
  LdapExtension() : Extension("ldap", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}
  void requestInit() override;
  void requestShutdown() override;
  void moduleInit() override;
} s_ldap_extension;

///////////////////////////////////////////////////////////////////////////////

struct LdapLink : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(LdapLink)

  LdapLink() {}
  ~LdapLink() override { closeImpl(false); }

  void close() {
    closeImpl(false);
    rebindproc.unset();
  }

  bool isInvalid() const override {
    return link == nullptr;
  }

private:
  void closeImpl(bool forSweep);

public:
  CLASSNAME_IS("ldap link");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  LDAP *link{nullptr};
  Variant rebindproc;
};

struct LdapRequestData {
  LdapRequestData() : m_num_links(0), m_max_links(-1) {
  }

  long m_num_links;
  long m_max_links;

  RootMap<LdapLink> m_links;
};
static RDS_LOCAL(LdapRequestData, s_ldap_data);
#define LDAPG(name) s_ldap_data->m_ ## name

void LdapExtension::requestInit() {
  if (!s_ldap_data.isNull()) {
    s_ldap_data->m_links.reset();
  }
}

void LdapExtension::requestShutdown() {
  if (!s_ldap_data.isNull()) {
    s_ldap_data->m_links.reset();
  }
}

namespace {

req::ptr<LdapLink> getLdapLinkFromToken(void* userData) {
  return s_ldap_data->m_links.lookupRoot(userData);
}

void* getLdapLinkToken(const req::ptr<LdapLink>& link) {
  return reinterpret_cast<void*>(
    s_ldap_data->m_links.addRoot(link)
  );
}

// Note: a raw pointer is ok here since clearLdapLink is being
// called from ~LdapLink which might be getting invoked from
// sweep and we can't create any new req::ptrs at the point.
void clearLdapLink(const LdapLink* link) {
  if (!s_ldap_data.isNull()) {
    s_ldap_data->m_links.removeRoot(link);
  }
}

}

void LdapLink::sweep() {
  closeImpl(true);
  rebindproc.releaseForSweep();
}

void LdapLink::closeImpl(bool forSweep) {
  if (link) {
    ldap_unbind_s(link);
    link = nullptr;
    LDAPG(num_links)--;
    if (!forSweep) {
      clearLdapLink(this);
    }
  }
}

struct LdapResult : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(LdapResult)

  LdapResult(LDAPMessage *res) : data(res) {}
  ~LdapResult() override { close();}

  void close() {
    if (data) {
      ldap_msgfree(data);
      data = nullptr;
    }
  }

  bool isInvalid() const override {
    return data == nullptr;
  }

  CLASSNAME_IS("ldap result");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof();}

  LDAPMessage *data;
};
IMPLEMENT_RESOURCE_ALLOCATION(LdapResult)

struct LdapResultEntry : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(LdapResultEntry)

  LdapResultEntry(LDAPMessage *entry, req::ptr<LdapResult> res)
    : data(entry), ber(nullptr), result(std::move(res)) {}
  ~LdapResultEntry() override { close();}

  void close() {
    if (ber != nullptr) {
      ber_free(ber, 0);
      ber = nullptr;
    }
    data = nullptr;
  }

  bool isInvalid() const override {
    return data == nullptr || result->isInvalid();
  }

  CLASSNAME_IS("ldap result entry");
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  LDAPMessage *data;
  BerElement *ber;
  req::ptr<LdapResult> result;
};

void LdapResultEntry::sweep() {
  close();
}

///////////////////////////////////////////////////////////////////////////////

template<typename T>
static req::ptr<LdapLink> get_valid_ldap_link_resource(const T& link) {
  auto ld = dyn_cast_or_null<LdapLink>(link);
  if (ld == nullptr || ld->isInvalid()) {
    raise_warning("Not a valid ldap link resource");
    return nullptr;
  }
  return ld;
}

static req::ptr<LdapResult> get_valid_ldap_result_resource(const OptResource& result) {
  auto res = dyn_cast_or_null<LdapResult>(result);
  if (res == nullptr || res->isInvalid()) {
    raise_warning("Not a valid ldap result resource");
    return nullptr;
  }
  return res;
}

static req::ptr<LdapResultEntry> get_valid_ldap_result_entry_resource(const OptResource& result_entry) {
  auto entry = dyn_cast_or_null<LdapResultEntry>(result_entry);
  if (entry == nullptr || entry->isInvalid()) {
    raise_warning("Not a valid ldap result entry resource");
    return nullptr;
  }
  return entry;
}

///////////////////////////////////////////////////////////////////////////////

static int _get_lderrno(LDAP *ldap) {
  int lderr;
  ldap_get_option(ldap, LDAP_OPT_ERROR_NUMBER, &lderr);
  return lderr;
}

static bool php_ldap_do_modify(const OptResource& link, const String& dn, const Array& entry,
                               int oper) {
  bool is_full_add = false; /* flag for full add operation so ldap_mod_add
                               can be put back into oper, gerrit THomson */

  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }

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
  Array stringHolder = Array::CreateVec();
  for (int i = 0; i < num_attribs; i++) {
    ldap_mods[i] = (LDAPMod*)malloc(sizeof(LDAPMod));
    ldap_mods[i]->mod_op = oper | LDAP_MOD_BVALUES;
    ldap_mods[i]->mod_type = nullptr;

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
    ldap_mods[i]->mod_bvalues[num_values] = nullptr;
    ++iter;
  }
  ldap_mods[num_attribs] = nullptr;

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
    if (old_sizelimit) {
      ldap_get_option(ldap, LDAP_OPT_SIZELIMIT, old_sizelimit);
    }
    ldap_set_option(ldap, LDAP_OPT_SIZELIMIT, &sizelimit);
  }

  /* timelimit */
  if (timelimit > -1) {
    if (old_timelimit) {
      ldap_get_option(ldap, LDAP_OPT_TIMELIMIT, old_timelimit);
    }
    ldap_set_option(ldap, LDAP_OPT_TIMELIMIT, &timelimit);
  }

  /* deref */
  if (deref > -1) {
    if (old_deref) {
      ldap_get_option(ldap, LDAP_OPT_DEREF, old_deref);
    }
    ldap_set_option(ldap, LDAP_OPT_DEREF, &deref);
  }
}

static Variant php_ldap_do_search(const Variant& link, const Variant& base_dn,
                                  const Variant& filter,
                                  const Variant& attributes,
                                  int attrsonly, int sizelimit, int timelimit,
                                  int deref, int scope) {
  const Array& arr_attributes = attributes.isNull()
                              ? null_array
                              : attributes.toArray();
  int num_attribs = arr_attributes.size();
  int old_sizelimit = -1, old_timelimit = -1, old_deref = -1;
  auto ldap_attrs = std::unique_ptr<char*[]>{new char*[num_attribs+1]};
  Array stringHolder = Array::CreateVec();
  char *ldap_base_dn = nullptr;
  char *ldap_filter = nullptr;

  for (int i = 0; i < num_attribs; i++) {
    if (!arr_attributes.exists(i)) {
      raise_warning("Array initialization wrong");
      return false;
    }
    String attr = arr_attributes[i].toString();
    stringHolder.append(attr);
    ldap_attrs[i] = (char*)attr.data();
  }
  ldap_attrs[num_attribs] = nullptr;

  /* parallel search? */
  if (link.isArray()) {
    int nlinks = link.toArray().size();
    if (nlinks == 0) {
      raise_warning("No links in link array");
      return false;
    }

    int nbases;
    if (base_dn.isArray()) {
      nbases = base_dn.toArray().size();
      if (nbases != nlinks) {
        raise_warning("Base must either be a string, or an array with the "
                      "same number of elements as the links array");
        return false;
      }
    } else {
      nbases = 0; /* this means string, not array */
      /* If anything else than string is passed, ldap_base_dn = nullptr */
      if (base_dn.isString()) {
        ldap_base_dn = (char*)base_dn.toString().data();
      } else {
        ldap_base_dn = nullptr;
      }
    }

    int nfilters;
    if (filter.isArray()) {
      nfilters = filter.toArray().size();
      if (nfilters != nlinks) {
        raise_warning("Filter must either be a string, or an array with the "
                      "same number of elements as the links array");
        return false;
      }
    } else {
      nfilters = 0; /* this means string, not array */
      String sfilter = filter.toString();
      stringHolder.append(sfilter);
      ldap_filter = (char*)sfilter.data();
    }

    req::vector<req::ptr<LdapLink>> lds;
    lds.resize(nlinks);

    req::vector<int> rcs;
    rcs.resize(nlinks);

    Array ret = Array::CreateVec();
    ArrayIter iter(link.toArray());
    ArrayIter iterdn(base_dn.toArray());
    ArrayIter iterfilter(filter.toArray());
    for (int i = 0; i < nlinks; i++) {
      auto ld = get_valid_ldap_link_resource(iter.second());
      if (!ld) {
        return false;
      }
      if (nbases != 0) { /* base_dn an array? */
        Variant entry = iterdn.second();
        ++iterdn;

        /* If anything else than string is passed, ldap_base_dn = nullptr */
        if (entry.isString()) {
          ldap_base_dn = (char*)entry.toString().data();
        } else {
          ldap_base_dn = nullptr;
        }
      }
      if (nfilters != 0) { /* filter an array? */
        Variant entry = iterfilter.second();
        ++iterfilter;
        String sentry = entry.toString();
        stringHolder.append(sentry);
        ldap_filter = (char*)sentry.data();
      }

      php_set_opts(ld->link,
                   sizelimit, timelimit, deref,
                   &old_sizelimit, &old_timelimit, &old_deref);

      /* Run the actual search */
      rcs[i] = ldap_search(ld->link, ldap_base_dn, scope, ldap_filter,
                           ldap_attrs.get(), attrsonly);

      php_set_opts(ld->link,
                   old_sizelimit, old_timelimit, old_deref,
                   nullptr, nullptr, nullptr);

      lds[i] = ld;
      ++iter;
    }

    /* Collect results from the searches */
    for (int i = 0; i < nlinks; i++) {
      LDAPMessage *ldap_res;
      if (rcs[i] != -1) {
        rcs[i] = ldap_result(lds[i]->link, rcs[i], LDAP_MSG_ALL,
                             nullptr, &ldap_res);
      }
      if (rcs[i] != -1) {
        ret.append(Variant(req::make<LdapResult>(ldap_res)));
      } else {
        ret.append(false);
      }
    }

    return ret;
  }

  /* not parallel search */
  String sfilter = filter.toString();
  ldap_filter = (char*)sfilter.data();

  /* If anything else than string is passed, ldap_base_dn = nullptr */
  if (base_dn.isString()) {
    ldap_base_dn = (char*)base_dn.toString().data();
  }

  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }

  php_set_opts(ld->link, sizelimit, timelimit, deref, &old_sizelimit,
               &old_timelimit, &old_deref);

  /* Run the actual search */
  LDAPMessage *ldap_res;
  int rc = ldap_search_s(ld->link, ldap_base_dn, scope, ldap_filter,
                         ldap_attrs.get(), attrsonly, &ldap_res);

  auto result = req::make<LdapResult>(ldap_res);
  php_set_opts(ld->link,
               old_sizelimit, old_timelimit, old_deref,
               nullptr, nullptr, nullptr);

  if (rc != LDAP_SUCCESS && rc != LDAP_SIZELIMIT_EXCEEDED
#ifdef LDAP_ADMINLIMIT_EXCEEDED
      && rc != LDAP_ADMINLIMIT_EXCEEDED
#endif
#ifdef LDAP_REFERRAL
      && rc != LDAP_REFERRAL
#endif
     ) {
    raise_warning("Search: %s", ldap_err2string(rc));
    return false;
  }

  if (rc == LDAP_SIZELIMIT_EXCEEDED) {
    raise_warning("Partial search results returned: Sizelimit exceeded");
  }
#ifdef LDAP_ADMINLIMIT_EXCEEDED
  else if (rc == LDAP_ADMINLIMIT_EXCEEDED) {
    raise_warning("Partial search results returned: Adminlimit exceeded");
  }
#endif
  return Variant{std::move(result)};
}

static int _ldap_rebind_proc(LDAP* /*ldap*/, const char* url, ber_tag_t /*req*/,
                             ber_int_t /*msgid*/, void* params) {
  auto ld = getLdapLinkFromToken(params);

  /* link exists and callback set? */
  if (!ld || ld->rebindproc.isNull()) {
    raise_warning("Link not found or no callback set");
    return LDAP_OTHER;
  }

  /* callback */
  Variant ret = vm_call_user_func
    (ld->rebindproc, make_vec_array(Variant(ld), String(url, CopyString)));
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

  while (attribute != nullptr) {
    struct berval **ldap_value =
      ldap_get_values_len(ldap, ldap_result_entry, attribute);
    int num_values = ldap_count_values_len(ldap_value);

    Array tmp;
    tmp.set(s_count, num_values);
    for (int i = 0; i < num_values; i++) {
      auto const val = ldap_value[i];
      tmp.set(i, String(val->bv_val, val->bv_len, CopyString));
    }
    ldap_value_free_len(ldap_value);

    String sAttribute = to_lower ? String(toLower(attribute))
                                 : String(attribute, CopyString);
    ret.set(sAttribute, tmp);
    ret.set(num_attrib, sAttribute);

    num_attrib++;
    ldap_memfree(attribute);
    attribute = ldap_next_attribute(ldap, ldap_result_entry, ber);
  }

  if (ber != nullptr) {
    ber_free(ber, 0);
  }

  ret.set(s_count, num_attrib);
}

///////////////////////////////////////////////////////////////////////////////

Variant HHVM_FUNCTION(ldap_connect,
                      const Variant& hostname /* = uninit_variant */,
                      int64_t port /* = 389 */) {
  const String& str_hostname = hostname.isNull()
                             ? null_string
                             : hostname.toString();
  if (LDAPG(max_links) != -1 && LDAPG(num_links) >= LDAPG(max_links)) {
    raise_warning("Too many open links (%ld)", LDAPG(num_links));
    return false;
  }

  auto ld = req::make<LdapLink>();

  LDAP *ldap = nullptr;
  if (!str_hostname.empty() && str_hostname.find('/') >= 0) {
    int rc = ldap_initialize(&ldap, str_hostname.data());
    if (rc != LDAP_SUCCESS) {
      raise_warning("Could not create session handle: %s",
                    ldap_err2string(rc));
      return false;
    }
  } else {
    ldap = ldap_init((char*)str_hostname.data(), port);
  }

  if (ldap) {
    LDAPG(num_links)++;
    ld->link = ldap;
    return Variant(std::move(ld));
  }
  raise_warning("Unable to initialize LDAP: %s",
                folly::errnoStr(errno).c_str());
  return false;
}

Variant HHVM_FUNCTION(ldap_explode_dn,
                      const String& dn,
                      int64_t with_attrib) {
  char **ldap_value;
  if (!(ldap_value = ldap_explode_dn((char*)dn.data(), with_attrib))) {
    /* Invalid parameters were passed to ldap_explode_dn */
    return false;
  }

  int i = 0;
  while (ldap_value[i] != nullptr) i++;
  int count = i;

  Array ret;
  ret.set(s_count, count);
  for (i = 0; i < count; i++) {
    ret.set(i, String(ldap_value[i], CopyString));
  }

  ldap_value_free(ldap_value);
  return ret;
}

Variant HHVM_FUNCTION(ldap_dn2ufn,
                      const String& db) {
  char *ufn = ldap_dn2ufn((char*)db.data());
  if (ufn) {
    String ret(ufn, CopyString);
    ldap_memfree(ufn);
    return ret;
  }
  return false;
}

String HHVM_FUNCTION(ldap_err2str,
                     int64_t errnum) {
  return String(ldap_err2string(errnum), CopyString);
}

bool HHVM_FUNCTION(ldap_add,
                   const OptResource& link,
                   const String& dn,
                   const Array& entry) {
  return php_ldap_do_modify(link, dn, entry, PHP_LD_FULL_ADD);
}

bool HHVM_FUNCTION(ldap_mod_add,
                   const OptResource& link,
                   const String& dn,
                   const Array& entry) {
  return php_ldap_do_modify(link, dn, entry, LDAP_MOD_ADD);
}

bool HHVM_FUNCTION(ldap_mod_del,
                   const OptResource& link,
                   const String& dn,
                   const Array& entry) {
  return php_ldap_do_modify(link, dn, entry, LDAP_MOD_DELETE);
}

bool HHVM_FUNCTION(ldap_mod_replace,
                   const OptResource& link,
                   const String& dn,
                   const Array& entry) {
  return php_ldap_do_modify(link, dn, entry, LDAP_MOD_REPLACE);
}

bool HHVM_FUNCTION(ldap_modify,
                   const OptResource& link,
                   const String& dn,
                   const Array& entry) {
  return php_ldap_do_modify(link, dn, entry, LDAP_MOD_REPLACE);
}

bool HHVM_FUNCTION(ldap_modify_batch,
                   const OptResource& link,
                   const String& dn,
                   const Array& modifs) {
  /*
  $modifs = [
    [
      "attrib" => "unicodePwd",
      "modtype" => LDAP_MODIFY_BATCH_REMOVE,
      "values" => [$oldpw]
    ],
    [
      "attrib" => "unicodePwd",
      "modtype" => LDAP_MODIFY_BATCH_ADD,
      "values" => [$newpw]
    ],
    [
      "attrib" => "userPrincipalName",
      "modtype" => LDAP_MODIFY_BATCH_REPLACE,
      "values" => ["janitor@corp.contoso.com"]
    ],
    [
      "attrib" => "userCert",
      "modtype" => LDAP_MODIFY_BATCH_REMOVE_ALL
    ]
  ];
  */

  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }

  ssize_t num_mods = modifs.size();
  int return_value;

  /* perform validation */
  {
    /* make sure the DN contains no NUL bytes */
    if (dn.find('\0') != String::npos) {
      raise_warning("DN must not contain NUL bytes");
      return false;
    }

    for (ssize_t i = 0; i < num_mods; ++i) {
      /* are the keys consecutive integers? */
      if (!modifs.exists((int64_t)i)) {
        raise_warning("Modifications array must have integer indices "
                      "0, 1, ...");
        return false;
      }

      Variant mod = modifs[(int64_t)i];

      /* is the value an array itself? */
      if (!mod.isArray()) {
        raise_warning("Each entry of modifications array must be an array "
                      "itself");
        return false;
      }

      /* for the modification hashtable... */
      const Array& modprops = mod.asCArrRef();
      ssize_t num_modprops = modprops.size();
      ArrayIter modprops_iter(modprops);

      for (ssize_t j = 0; j < num_modprops; ++j) {
        /* is the key a string? */
        if (!modprops_iter.first().isString()) {
          raise_warning("Each entry of modifications array must be "
                        "string-indexed");
          return false;
        }

        /* is this a valid entry? */
        const String& modkey = modprops_iter.first().asCStrRef();
        if (modkey != s_LDAP_MODIFY_BATCH_ATTRIB
            && modkey != s_LDAP_MODIFY_BATCH_MODTYPE
            && modkey != s_LDAP_MODIFY_BATCH_VALUES) {
          raise_warning("The only allowed keys in entries of the modifications "
                        "array are '" LDAP_MODIFY_BATCH_ATTRIB "', '"
                        LDAP_MODIFY_BATCH_MODTYPE "' and '"
                        LDAP_MODIFY_BATCH_VALUES "'");
          return false;
        }

        /* does the value type match the key? */
        if (modkey == s_LDAP_MODIFY_BATCH_ATTRIB) {
          if (!modprops_iter.second().isString()) {
            raise_warning("A '" LDAP_MODIFY_BATCH_ATTRIB "' value must be "
                          "a string");
            return false;
          }
          if (modprops_iter.second().asCStrRef().find('\0') != String::npos) {
            raise_warning("A '" LDAP_MODIFY_BATCH_ATTRIB "' value must not "
                          "contain NUL bytes");
            return false;
          }
        } else if (modkey == s_LDAP_MODIFY_BATCH_MODTYPE) {
          if (!modprops_iter.second().isInteger()) {
            raise_warning("A '" LDAP_MODIFY_BATCH_MODTYPE "' value must be "
                          "an integer");
            return false;
          }

          int64_t modtype = modprops_iter.second().asInt64Val();
          if (modtype != LDAP_MODIFY_BATCH_ADD
              && modtype != LDAP_MODIFY_BATCH_REMOVE
              && modtype != LDAP_MODIFY_BATCH_REPLACE
              && modtype != LDAP_MODIFY_BATCH_REMOVE_ALL) {
            raise_warning("The '" LDAP_MODIFY_BATCH_MODTYPE "' value must "
                          "match one of the LDAP_MODIFY_BATCH_* constants");
            return false;
          }

          /* if it's REMOVE_ALL, there must not be a values array; */
          /* otherwise, there must */
          if (modtype == LDAP_MODIFY_BATCH_REMOVE_ALL) {
            if (modprops.exists(s_LDAP_MODIFY_BATCH_VALUES)) {
              raise_warning("If '" LDAP_MODIFY_BATCH_MODTYPE "' is "
                            "LDAP_MODIFY_BATCH_REMOVE_ALL, a '"
                            LDAP_MODIFY_BATCH_VALUES "' array must not "
                            "be provided");
              return false;
            }
          } else {
            if (!modprops.exists(s_LDAP_MODIFY_BATCH_VALUES)) {
              raise_warning("If '" LDAP_MODIFY_BATCH_MODTYPE "' is not "
                            "LDAP_MODIFY_BATCH_REMOVE_ALL, a '"
                            LDAP_MODIFY_BATCH_VALUES "' array must "
                            "be provided");
              return false;
            }
          }
        } else if (modkey == s_LDAP_MODIFY_BATCH_VALUES) {
          if (!modprops_iter.second().isArray()) {
            raise_warning("A '" LDAP_MODIFY_BATCH_VALUES "' value must be "
                          "an array");
            return false;
          }

          const Array& modvalues = modprops_iter.second().asCArrRef();
          ssize_t num_modvalues = modvalues.size();

          /* is the array not empty? */
          if (num_modvalues == 0) {
            raise_warning("A '" LDAP_MODIFY_BATCH_VALUES "' array must have "
                          "at least one element");
            return false;
          }

          /* for the modification hashtable... */
          for (ssize_t k = 0; k < num_modvalues; ++k) {
            /* are the keys consecutive integers? */
            if (!modvalues.exists((int64_t)k)) {
              raise_warning("A '" LDAP_MODIFY_BATCH_VALUES "' array must have "
                            "integer indices 0, 1, ...");
              return false;
            }

            Variant modvalue = modvalues[(int64_t)k];

            /* is the data element a string? */
            if (!modvalue.isString()) {
              raise_warning("Each element of a '" LDAP_MODIFY_BATCH_VALUES "' "
                            "array must be a string");
              return false;
            }
          }
        }

        ++modprops_iter;
      }
    }
  }

  /* validation was successful */

  /* allocate array of modifications */
  req::vector<LDAPMod *> ldap_mods(num_mods + 1);

  {
    /* for each modification */
    for (int64_t i = 0; i < num_mods; ++i) {
      /* allocate the modification struct */
      ldap_mods[i] = req::make_raw<LDAPMod>();

      /* fetch the relevant data */
      const Array& mod = modifs[i].asCArrRef();
      const String& attrib = mod[s_LDAP_MODIFY_BATCH_ATTRIB].asCStrRef();
      int64_t modtype = mod[s_LDAP_MODIFY_BATCH_MODTYPE].asInt64Val();
      const Array& vals = mod[s_LDAP_MODIFY_BATCH_VALUES].asCArrRef();

      /* map the modification type */
      int oper;
      switch (modtype) {
      case LDAP_MODIFY_BATCH_ADD:
        oper = LDAP_MOD_ADD;
        break;
      case LDAP_MODIFY_BATCH_REMOVE:
      case LDAP_MODIFY_BATCH_REMOVE_ALL:
        oper = LDAP_MOD_DELETE;
        break;
      case LDAP_MODIFY_BATCH_REPLACE:
        oper = LDAP_MOD_REPLACE;
        break;
      default:
        raise_error("Unknown and uncaught modification type.");
        return false;
      }

      /* fill in the basic info */
      ldap_mods[i]->mod_op = oper | LDAP_MOD_BVALUES;
      ldap_mods[i]->mod_type = req::strndup(attrib.data(), attrib.size());

      if (modtype == LDAP_MODIFY_BATCH_REMOVE_ALL) {
        /* no values */
        ldap_mods[i]->mod_bvalues = nullptr;
      } else {
        /* allocate space for the values as part of this modification */
        ssize_t num_modvals = vals.size();
        ldap_mods[i]->mod_bvalues =
          (struct berval **)req::malloc_untyped((num_modvals + 1) *
                                        sizeof(struct berval *));

        /* for each value */
        for (int64_t j = 0; j < num_modvals; ++j) {
          /* fetch it */
          const String& modval = vals[j].asCStrRef();

          /* allocate the data struct */
          ldap_mods[i]->mod_bvalues[j] = req::make_raw<struct berval>();

          /* fill it */
          ldap_mods[i]->mod_bvalues[j]->bv_len = modval.size();
          ldap_mods[i]->mod_bvalues[j]->bv_val =
            req::make_raw_array<char>(modval.size());
          memcpy(ldap_mods[i]->mod_bvalues[j]->bv_val, modval.data(),
                 modval.size());
        }

        /* nullptr-terminate values */
        ldap_mods[i]->mod_bvalues[num_modvals] = nullptr;
      }
    }

    /* nullptr-terminate modifications */
    ldap_mods[num_mods] = nullptr;
  }

  /* perform (finally) */
  if ((return_value = ldap_modify_ext_s(ld->link, dn.data(), ldap_mods.data(),
                                        nullptr, nullptr)) != LDAP_SUCCESS) {
    raise_warning("Batch Modify: %s", ldap_err2string(return_value));
    /* only return after cleanup */
  }

  /* clean up */
  {
    for (ssize_t i = 0; i < num_mods; ++i) {
      /* attribute */
      req::free(ldap_mods[i]->mod_type);

      if (ldap_mods[i]->mod_bvalues != nullptr) {
        /* each BER value */
        for (ssize_t j = 0; ldap_mods[i]->mod_bvalues[j] != nullptr; ++j) {
          /* free the data bytes */
          req::destroy_raw_array(ldap_mods[i]->mod_bvalues[j]->bv_val,
                                 ldap_mods[i]->mod_bvalues[j]->bv_len);

          /* free the bvalue struct */
          req::destroy_raw(ldap_mods[i]->mod_bvalues[j]);
        }

        /* the BER value array */
        req::free(ldap_mods[i]->mod_bvalues);
      }

      /* the modifications */
      req::destroy_raw(ldap_mods[i]);
    }
  }

  return (return_value == LDAP_SUCCESS);
}

bool HHVM_FUNCTION(ldap_bind,
                   const OptResource& link,
                   const Variant& bind_rdn /* = uninit_variant */,
                   const Variant& bind_password /* = uninit_variant */) {

  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }

  const String& str_bind_rdn = bind_rdn.isNull()
                             ? null_string
                             : bind_rdn.toString();
  const String& str_bind_password = bind_password.isNull()
                                  ? null_string
                                  : bind_password.toString();

  if (memchr(str_bind_rdn.data(), '\0', str_bind_rdn.size()) != nullptr) {
    raise_warning("DN contains a null byte");
    return false;
  }
  if (memchr(str_bind_password.data(), '\0',
      str_bind_password.size()) != nullptr) {
    raise_warning("Password contains a null byte");
    return false;
  }

  int rc;
  if ((rc = ldap_bind_s(ld->link, (char*)str_bind_rdn.data(),
                        (char*)str_bind_password.data(),
                        LDAP_AUTH_SIMPLE)) != LDAP_SUCCESS) {
    raise_warning("Unable to bind to server: %s", ldap_err2string(rc));
    return false;
  }
  return true;
}

bool HHVM_FUNCTION(ldap_set_rebind_proc,
                   const OptResource& link,
                   const Variant& callback) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }

  if (callback.isString() && callback.toString().empty()) {
    /* unregister rebind procedure */
    if (!ld->rebindproc.isNull()) {
      ld->rebindproc.unset();
      ldap_set_rebind_proc(ld->link, nullptr, nullptr);
    }
    return true;
  }

  /* callable? */
  if (!is_callable(callback)) {
    raise_warning("Callback argument is not a valid callback");
    return false;
  }

  /* register rebind procedure */
  if (ld->rebindproc.isNull()) {
    ldap_set_rebind_proc(ld->link,
                         _ldap_rebind_proc,
                         getLdapLinkToken(ld));
  } else {
    ld->rebindproc.unset();
  }

  ld->rebindproc = callback;
  return true;
}

bool HHVM_FUNCTION(ldap_sort,
                   const OptResource& link,
                   const OptResource& result,
                   const String& sortfilter) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto res = get_valid_ldap_result_resource(result);
  if (!res) {
    return false;
  }

  if (ldap_sort_entries(
        ld->link, &res->data,
        !sortfilter.empty() ? (char*)sortfilter.data() : nullptr,
        strcmp) != LDAP_SUCCESS) {
    raise_warning("%s", ldap_err2string(_get_lderrno(ld->link)));
    return false;
  }
  return true;
}

bool HHVM_FUNCTION(ldap_start_tls,
                   const OptResource& link) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  int rc, protocol = LDAP_VERSION3;
  if (((rc = ldap_set_option(ld->link, LDAP_OPT_PROTOCOL_VERSION, &protocol))
       != LDAP_SUCCESS) ||
      ((rc = ldap_start_tls_s(ld->link, nullptr, nullptr)) != LDAP_SUCCESS)) {
    raise_warning("Unable to start TLS: %s", ldap_err2string(rc));
    return false;
  }
  return true;
}

bool HHVM_FUNCTION(ldap_unbind, const OptResource& link) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  ld->close();
  return true;
}

bool HHVM_FUNCTION(ldap_get_option,
                   const OptResource& link,
                   int64_t option,
                   Variant& retval) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }

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
      char *val = nullptr;
      if (ldap_get_option(ld->link, option, &val) || val == nullptr ||
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

bool HHVM_FUNCTION(ldap_set_option,
                   const Variant& link,
                   int64_t option,
                   const Variant& newval) {
  LDAP *ldap = nullptr;
  if (!link.isNull()) {
    auto ld = get_valid_ldap_link_resource(link);
    if (!ld) {
      return false;
    }
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
      *ctrls = nullptr;
      ctrlp = ctrls;
      Array stringHolder = Array::CreateVec();
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
          ctrl->ldctl_value.bv_val = nullptr;
          ctrl->ldctl_value.bv_len = 0;
        }
        if (ctrlval.exists(s_iscritical)) {
          ctrl->ldctl_iscritical = val.toBoolean() ? 1 : 0;
        } else {
          ctrl->ldctl_iscritical = 0;
        }

        ++ctrlp;
        *ctrlp = nullptr;
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

bool HHVM_FUNCTION(ldap_close,
                   const OptResource& link) {
  return HHVM_FN(ldap_unbind)(link);
}

Variant HHVM_FUNCTION(ldap_list,
                      const Variant& link,
                      const Variant& base_dn,
                      const Variant& filter,
                      const Variant& attributes /* = uninit_variant */,
                      int64_t attrsonly /* = 0 */,
                      int64_t sizelimit /* = -1 */,
                      int64_t timelimit /* = -1 */,
                      int64_t deref /* = -1 */) {
  return php_ldap_do_search(link, base_dn, filter, attributes, attrsonly,
                            sizelimit, timelimit, deref, LDAP_SCOPE_ONELEVEL);
}

Variant HHVM_FUNCTION(ldap_read,
                      const Variant& link,
                      const Variant& base_dn,
                      const Variant& filter,
                      const Variant& attributes /* = uninit_variant */,
                      int64_t attrsonly /* = 0 */,
                      int64_t sizelimit /* = -1 */,
                      int64_t timelimit /* = -1 */,
                      int64_t deref /* = -1 */) {
  return php_ldap_do_search(link, base_dn, filter, attributes, attrsonly,
                            sizelimit, timelimit, deref, LDAP_SCOPE_BASE);
}

Variant HHVM_FUNCTION(ldap_search,
                      const Variant& link,
                      const Variant& base_dn,
                      const Variant& filter,
                      const Variant& attributes /* = uninit_variant */,
                      int64_t attrsonly /* = 0 */,
                      int64_t sizelimit /* = -1 */,
                      int64_t timelimit /* = -1 */,
                      int64_t deref /* = -1 */) {
  return php_ldap_do_search(link, base_dn, filter, attributes, attrsonly,
                            sizelimit, timelimit, deref, LDAP_SCOPE_SUBTREE);
}

bool HHVM_FUNCTION(ldap_rename,
                   const OptResource& link,
                   const String& dn,
                   const String& newrdn,
                   const String& newparent,
                   bool deleteoldrdn) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }

  int rc = ldap_rename_s(ld->link, (char*)dn.data(), (char*)newrdn.data(),
                         !newparent.empty() ? (char*)newparent.data() : nullptr,
                         deleteoldrdn, nullptr, nullptr);
  return rc == LDAP_SUCCESS;
}

bool HHVM_FUNCTION(ldap_delete,
                   const OptResource& link,
                   const String& dn) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }

  int rc;
  if ((rc = ldap_delete_s(ld->link, (char*)dn.data())) != LDAP_SUCCESS) {
    raise_warning("Delete: %s", ldap_err2string(rc));
    return false;
  }
  return true;
}

Variant HHVM_FUNCTION(ldap_compare,
                      const OptResource& link,
                      const String& dn,
                      const String& attribute,
                      const String& value) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return -1LL;
  }

  int rc = ldap_compare_s(ld->link, (char*)dn.data(), (char*)attribute.data(),
                          (char*)value.data());
  switch (rc) {
  case LDAP_COMPARE_TRUE:  return true;
  case LDAP_COMPARE_FALSE: return false;
  }
  raise_warning("Compare: %s", ldap_err2string(rc));
  return -1LL;
}

Variant HHVM_FUNCTION(ldap_errno,
                      const OptResource& link) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }

  return _get_lderrno(ld->link);
}

Variant HHVM_FUNCTION(ldap_error,
                     const OptResource& link) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }

  int ld_errno = _get_lderrno(ld->link);
  return String(ldap_err2string(ld_errno), CopyString);
}

Variant HHVM_FUNCTION(ldap_get_dn,
                      const OptResource& link,
                      const OptResource& result_entry) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto entry = get_valid_ldap_result_entry_resource(result_entry);
  if (!entry) {
    return false;
  }

  char *text = ldap_get_dn(ld->link, entry->data);
  if (text) {
    String ret(text, CopyString);
    ldap_memfree(text);
    return ret;
  }
  return false;
}

Variant HHVM_FUNCTION(ldap_count_entries,
                      const OptResource& link,
                      const OptResource& result) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto res = get_valid_ldap_result_resource(result);
  if (!res) {
    return false;
  }

  return ldap_count_entries(ld->link, res->data);
}

Variant HHVM_FUNCTION(ldap_get_entries,
                      const OptResource& link,
                      const OptResource& result) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto res = get_valid_ldap_result_resource(result);
  if (!res) {
    return false;
  }

  LDAP *ldap = ld->link;

  int num_entries = ldap_count_entries(ldap, res->data);
  Array ret;
  ret.set(s_count, num_entries);
  if (num_entries == 0) {
    return init_null();
  }

  LDAPMessage *ldap_result_entry = ldap_first_entry(ldap, res->data);
  if (ldap_result_entry == nullptr) {
    return false;
  }

  num_entries = 0;
  while (ldap_result_entry != nullptr) {
    Array tmp1 = Array::CreateDict();
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

Variant HHVM_FUNCTION(ldap_first_entry,
                      const OptResource& link,
                      const OptResource& result) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto res = get_valid_ldap_result_resource(result);
  if (!res) {
    return false;
  }

  LDAPMessage *entry;
  if ((entry = ldap_first_entry(ld->link, res->data)) == nullptr) {
    return false;
  }

  return Variant(req::make<LdapResultEntry>(entry, res));
}

Variant HHVM_FUNCTION(ldap_next_entry,
                      const OptResource& link,
                      const OptResource& result_entry) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto entry = get_valid_ldap_result_entry_resource(result_entry);
  if (!entry) {
    return false;
  }

  LDAPMessage *msg;
  if ((msg = ldap_next_entry(ld->link, entry->data)) == nullptr) {
    return false;
  }

  return Variant(req::make<LdapResultEntry>(msg, entry->result));
}

Variant HHVM_FUNCTION(ldap_get_attributes,
                    const OptResource& link,
                    const OptResource& result_entry) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto entry = get_valid_ldap_result_entry_resource(result_entry);
  if (!entry) {
    return false;
  }

  Array ret = Array::CreateDict();
  get_attributes(ret, ld->link, entry->data, false);
  return ret;
}

Variant HHVM_FUNCTION(ldap_first_attribute,
                      const OptResource& link,
                      const OptResource& result_entry) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto entry = get_valid_ldap_result_entry_resource(result_entry);
  if (!entry) {
    return false;
  }

  if (entry->ber) {
    // we are already iterating the attributes
    ber_free(entry->ber, 0);
    entry->ber = nullptr;
  }

  char *attribute;
  if ((attribute =
       ldap_first_attribute(ld->link, entry->data, &entry->ber)) == nullptr) {
    entry->ber = nullptr;
    return false;
  }
  String ret(attribute, CopyString);
  ldap_memfree(attribute);
  return ret;
}

Variant HHVM_FUNCTION(ldap_next_attribute,
                      const OptResource& link,
                      const OptResource& result_entry) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto entry = get_valid_ldap_result_entry_resource(result_entry);
  if (!entry) {
    return false;
  }

  if (entry->ber == nullptr) {
    raise_warning("called before calling ldap_first_attribute() or "
                  "no attributes found in result entry");
    return false;
  }

  char *attribute;
  if ((attribute =
       ldap_next_attribute(ld->link, entry->data, entry->ber)) == nullptr) {
    assertx(entry->ber != nullptr);
    ber_free(entry->ber, 0);
    entry->ber = nullptr;
    return false;
  }
  String ret(attribute, CopyString);
  ldap_memfree(attribute);
  return ret;
}

Variant HHVM_FUNCTION(ldap_first_reference,
                      const OptResource& link,
                      const OptResource& result) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto res = get_valid_ldap_result_resource(result);
  if (!res) {
    return false;
  }

  LDAPMessage *entry;
  if ((entry = ldap_first_reference(ld->link, res->data)) == nullptr) {
    return false;
  }

  return Variant(req::make<LdapResultEntry>(entry, res));
}

Variant HHVM_FUNCTION(ldap_next_reference,
                      const OptResource& link,
                      const OptResource& result_entry) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto entry = get_valid_ldap_result_entry_resource(result_entry);
  if (!entry) {
    return false;
  }

  LDAPMessage *entry_next;
  if ((entry_next = ldap_next_reference(ld->link, entry->data)) == nullptr) {
    return false;
  }

  return Variant(req::make<LdapResultEntry>(entry_next, entry->result));
}

bool HHVM_FUNCTION(ldap_parse_reference,
                   const OptResource& link,
                   const OptResource& result_entry,
                   Array& referrals) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto entry = get_valid_ldap_result_entry_resource(result_entry);
  if (!entry) {
    return false;
  }

  char **lreferrals, **refp;
  if (ldap_parse_reference(ld->link, entry->data, &lreferrals,
                           nullptr /* &serverctrls */, 0) != LDAP_SUCCESS) {
    return false;
  }

  referrals = Array::CreateVec();
  if (lreferrals != nullptr) {
    refp = lreferrals;
    while (*refp) {
      referrals.append(String(*refp, CopyString));
      refp++;
    }
    ldap_value_free(lreferrals);
  }
  return true;
}

bool HHVM_FUNCTION(ldap_parse_result,
                   const OptResource& link,
                   const OptResource& result,
                   int64_t& errcode,
                   String& matcheddn,
                   String& errmsg,
                   Array& referrals) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto res = get_valid_ldap_result_resource(result);
  if (!res) {
    return false;
  }

  int lerrcode;
  char **lreferrals, **refp;
  char *lmatcheddn, *lerrmsg;
  int rc = ldap_parse_result(ld->link, res->data, &lerrcode,
                             &lmatcheddn, &lerrmsg, &lreferrals,
                             nullptr /* &serverctrls */, 0);
  if (rc != LDAP_SUCCESS) {
    raise_warning("Unable to parse result: %s", ldap_err2string(rc));
    return false;
  }

  errcode = lerrcode;

  /* Reverse -> fall through */
  referrals = Array::CreateVec();
  if (lreferrals != nullptr) {
    refp = lreferrals;
    while (*refp) {
      referrals.append(String(*refp, CopyString));
      refp++;
    }
    ldap_value_free(lreferrals);
  }

  if (lerrmsg == nullptr) {
    errmsg = staticEmptyString();
  } else {
    errmsg = String(lerrmsg, CopyString);
    ldap_memfree(lerrmsg);
  }

  if (lmatcheddn == nullptr) {
    matcheddn = staticEmptyString();
  } else {
    matcheddn = String(lmatcheddn, CopyString);
    ldap_memfree(lmatcheddn);
  }
  return true;
}

bool HHVM_FUNCTION(ldap_free_result,
                   const OptResource& result) {
  auto res = get_valid_ldap_result_resource(result);
  if (!res) {
    return false;
  }
  res->close();
  return true;
}

Variant HHVM_FUNCTION(ldap_get_values_len,
                      const OptResource& link,
                      const OptResource& result_entry,
                      const String& attribute) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto entry = get_valid_ldap_result_entry_resource(result_entry);
  if (!entry) {
    return false;
  }

  struct berval **ldap_value_len;
  if ((ldap_value_len =
       ldap_get_values_len(ld->link, entry->data,
                           (char*)attribute.data())) == nullptr) {
    raise_warning("Cannot get the value(s) of attribute %s",
                  ldap_err2string(_get_lderrno(ld->link)));
    return false;
  }

  int num_values = ldap_count_values_len(ldap_value_len);
  Array ret;
  for (int i = 0; i < num_values; i++) {
    auto const val = ldap_value_len[i];
    ret.set(i, String(val->bv_val, val->bv_len, CopyString));
  }
  ret.set(s_count, num_values);
  ldap_value_free_len(ldap_value_len);
  return ret;
}

Variant HHVM_FUNCTION(ldap_get_values,
                      const OptResource& link,
                      const OptResource& result_entry,
                      const String& attribute) {
  return HHVM_FN(ldap_get_values_len)(link, result_entry, attribute);
}

bool HHVM_FUNCTION(ldap_control_paged_result,
                   const OptResource& link,
                   int64_t pagesize,
                   bool iscritical,
                   const String& cookie) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  LDAPControl ctrl, *ctrlsp[2];
  int rc;
  struct berval lcookie;
  BerElement *ber;

  ber = ber_alloc_t(LBER_USE_DER);
  if (ber == nullptr) {
    raise_warning("Unable to alloc BER encoding resources for paged "
                  "results control");
    return false;
  }

  lcookie.bv_val = (char *)cookie.c_str();
  lcookie.bv_len = cookie.length();
  if (ber_printf(ber, "{iO}", pagesize, &lcookie) == LBER_ERROR) {
    raise_warning("Unable to BER printf paged results control");
    ber_free(ber, 1);
    return false;
  }

  rc = ber_flatten2(ber, &ctrl.ldctl_value, 0);
  if (rc == LBER_ERROR) {
    raise_warning("Unable to BER encode paged results control");
    ber_free(ber, 1);
    return false;
  }

  ctrl.ldctl_oid = LDAP_CONTROL_PAGEDRESULTS;
  ctrl.ldctl_iscritical = iscritical;

  ctrlsp[0] = &ctrl;
  ctrlsp[1] = nullptr;
  rc = ldap_set_option(ld->link, LDAP_OPT_SERVER_CONTROLS, ctrlsp);

  ber_free(ber, 1);
  return rc == LDAP_SUCCESS;
}

bool HHVM_FUNCTION(ldap_control_paged_result_response,
                   const OptResource& link,
                   const OptResource& result,
                   String& cookie,
                   int64_t& estimated) {
  auto ld = get_valid_ldap_link_resource(link);
  if (!ld) {
    return false;
  }
  auto res = get_valid_ldap_result_resource(result);
  if (!res) {
    return false;
  }
  int rc, lerrcode;
  LDAPControl **lserverctrls, *lctrl;
  BerElement *ber;
  int lestimated;
  struct berval lcookie;
  ber_tag_t tag;

  rc = ldap_parse_result(ld->link,
                         res->data,
                         &lerrcode,
                         nullptr,   /* matcheddn */
                         nullptr,   /* errmsg */
                         nullptr,   /* referrals */
                         &lserverctrls,
                         0);
  if (rc != LDAP_SUCCESS) {
    raise_warning("Unable to parse result: %s (%d)", ldap_err2string(rc), rc);
    return false;
  }

  if (lerrcode != LDAP_SUCCESS) {
    raise_warning("Result is: %s (%d)", ldap_err2string(lerrcode), lerrcode);
    return false;
  }

  if (lserverctrls == nullptr) {
    raise_warning("No server controls in result");
    return false;
  }

  lctrl = ldap_find_control(LDAP_CONTROL_PAGEDRESULTS, lserverctrls);
  if (lctrl == nullptr) {
    raise_warning("No paged results control response in result");
    ldap_controls_free(lserverctrls);
    return false;
  }

  ber = ber_init(&lctrl->ldctl_value);
  if (ber == nullptr) {
    raise_warning("Unable to alloc BER decoding resources for paged "
                  "results control response");
    ldap_controls_free(lserverctrls);
    return false;
  }

  tag = ber_scanf(ber, "{io}", &lestimated, &lcookie);
  ber_free(ber, 1);
  ldap_controls_free(lserverctrls);

  if (tag == LBER_ERROR) {
    raise_warning("Unable to decode paged results control response");
    return false;
  }

  if (lestimated < 0) {
    raise_warning("Invalid paged results control response value");
    ber_memfree(lcookie.bv_val);
    return false;
  }

  cookie = String(lcookie.bv_val, lcookie.bv_len, CopyString);
  estimated = lestimated;

  ber_memfree(lcookie.bv_val);
  return true;
}

String HHVM_FUNCTION(ldap_escape,
                     const String& value,
                     const String& ignores /* = "" */,
                     int64_t flags /* = 0 */) {
  char esc[256] = {};

  if (flags & k_LDAP_ESCAPE_FILTER) { // llvm.org/bugs/show_bug.cgi?id=18389
    esc['*'*1u] = esc['('*1u] = esc[')'*1u] = esc['\0'*1u] = esc['\\'*1u] = 1;
  }

  if (flags & k_LDAP_ESCAPE_DN) {
    esc[','*1u] = esc['='*1u] = esc['+'*1u] = esc['<'*1u] = esc['\\'*1u] = 1;
    esc['>'*1u] = esc[';'*1u] = esc['"'*1u] = esc['#'*1u] = 1;
  }

  if (!flags) {
    memset(esc, 1, sizeof(esc));
  }

  for (int i = 0; i < ignores.size(); i++) {
    esc[(unsigned char)ignores[i]] = 0;
  }

  char hex[] = "0123456789abcdef";

  String result(3UL * value.size(), ReserveString);
  char *rdata = result.get()->mutableData(), *r = rdata;

  for (int i = 0; i < value.size(); i++) {
    auto c = (unsigned char)value[i];
    if (esc[c]) {
      *r++ = '\\';
      *r++ = hex[c >> 4];
      *r++ = hex[c & 0xf];
    } else {
      *r++ = c;
    }
  }

  result.setSize(r - rdata);
  return result;
}

void LdapExtension::moduleInit() {
  HHVM_RC_INT(LDAP_ESCAPE_FILTER, k_LDAP_ESCAPE_FILTER);
  HHVM_RC_INT(LDAP_ESCAPE_DN, k_LDAP_ESCAPE_DN);

  HHVM_FE(ldap_connect);
  HHVM_FE(ldap_explode_dn);
  HHVM_FE(ldap_dn2ufn);
  HHVM_FE(ldap_err2str);
  HHVM_FE(ldap_add);
  HHVM_FE(ldap_mod_add);
  HHVM_FE(ldap_mod_del);
  HHVM_FE(ldap_mod_replace);
  HHVM_FE(ldap_modify);
  HHVM_FE(ldap_modify_batch);
  HHVM_FE(ldap_bind);
  HHVM_FE(ldap_set_rebind_proc);
  HHVM_FE(ldap_sort);
  HHVM_FE(ldap_start_tls);
  HHVM_FE(ldap_unbind);
  HHVM_FE(ldap_get_option);
  HHVM_FE(ldap_set_option);
  HHVM_FE(ldap_close);
  HHVM_FE(ldap_list);
  HHVM_FE(ldap_read);
  HHVM_FE(ldap_search);
  HHVM_FE(ldap_rename);
  HHVM_FE(ldap_delete);
  HHVM_FE(ldap_compare);
  HHVM_FE(ldap_errno);
  HHVM_FE(ldap_error);
  HHVM_FE(ldap_get_dn);
  HHVM_FE(ldap_count_entries);
  HHVM_FE(ldap_get_entries);
  HHVM_FE(ldap_first_entry);
  HHVM_FE(ldap_next_entry);
  HHVM_FE(ldap_get_attributes);
  HHVM_FE(ldap_first_attribute);
  HHVM_FE(ldap_next_attribute);
  HHVM_FE(ldap_first_reference);
  HHVM_FE(ldap_next_reference);
  HHVM_FE(ldap_parse_reference);
  HHVM_FE(ldap_parse_result);
  HHVM_FE(ldap_free_result);
  HHVM_FE(ldap_get_values_len);
  HHVM_FE(ldap_get_values);
  HHVM_FE(ldap_control_paged_result);
  HHVM_FE(ldap_control_paged_result_response);
  HHVM_FE(ldap_escape);

  HHVM_RC_INT_SAME(LDAP_DEREF_ALWAYS);
  HHVM_RC_INT_SAME(LDAP_DEREF_FINDING);
  HHVM_RC_INT_SAME(LDAP_DEREF_NEVER);
  HHVM_RC_INT_SAME(LDAP_DEREF_SEARCHING);

  HHVM_RC_INT_SAME(LDAP_MODIFY_BATCH_ADD);
  HHVM_RC_INT_SAME(LDAP_MODIFY_BATCH_REMOVE);
  HHVM_RC_INT_SAME(LDAP_MODIFY_BATCH_REMOVE_ALL);
  HHVM_RC_INT_SAME(LDAP_MODIFY_BATCH_REPLACE);
  HHVM_RC_STR_SAME(LDAP_MODIFY_BATCH_ATTRIB);
  HHVM_RC_STR_SAME(LDAP_MODIFY_BATCH_MODTYPE);
  HHVM_RC_STR_SAME(LDAP_MODIFY_BATCH_VALUES);

  HHVM_RC_INT_SAME(LDAP_OPT_DEREF);
  HHVM_RC_INT_SAME(LDAP_OPT_SIZELIMIT);
  HHVM_RC_INT_SAME(LDAP_OPT_TIMELIMIT);
  HHVM_RC_INT_SAME(LDAP_OPT_PROTOCOL_VERSION);
  HHVM_RC_INT_SAME(LDAP_OPT_ERROR_NUMBER);
  HHVM_RC_INT_SAME(LDAP_OPT_REFERRALS);
  HHVM_RC_INT_SAME(LDAP_OPT_ERROR_STRING);
  HHVM_RC_INT_SAME(LDAP_OPT_SERVER_CONTROLS);
  HHVM_RC_INT_SAME(LDAP_OPT_CLIENT_CONTROLS);

#ifdef LDAP_OPT_NETWORK_TIMEOUT
  HHVM_RC_INT_SAME(LDAP_OPT_NETWORK_TIMEOUT);
#elif defined(LDAP_X_OPT_NETWORK_TIMEOUT)
  HHVM_RC_INT(LDAP_OPT_NETWORK_TIMEOUT, LDAP_X_OPT_NETWORK_TIMEOUT);
#endif
#ifdef LDAP_OPT_TIMEOUT
  HHVM_RC_INT_SAME(LDAP_OPT_TIMEOUT);
#endif
#ifdef LDAP_OPT_RESTART
  HHVM_RC_INT_SAME(LDAP_OPT_RESTART);
#endif
#ifdef LDAP_OPT_HOST_NAME
  HHVM_RC_INT_SAME(LDAP_OPT_HOST_NAME);
#endif
#ifdef LDAP_OPT_MATCHED_DN
  HHVM_RC_INT_SAME(LDAP_OPT_MATCHED_DN);
#endif
#ifdef LDAP_OPT_DEBUG_LEVEL
  HHVM_RC_INT_SAME(LDAP_OPT_DEBUG_LEVEL);
#endif

  LDAP* link = nullptr;
  if (ldap_create(&link) == LDAP_SUCCESS) {
    ldap_unbind(link);
  }
}

///////////////////////////////////////////////////////////////////////////////
}
