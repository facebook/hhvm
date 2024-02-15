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
#include "ext_gettext.h"

#include <stdio.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/file.h"

#include <libintl.h>

#include <folly/portability/Stdlib.h>

#define PHP_GETTEXT_MAX_DOMAIN_LENGTH 1024
#define PHP_GETTEXT_MAX_MSGID_LENGTH 4096

# define HAVE_NGETTEXT 1
# define HAVE_DNGETTEXT 1
# define HAVE_DCNGETTEXT 1
# define HAVE_BIND_TEXTDOMAIN_CODESET 1

namespace HPHP {

#define CHECK_DOMAIN_LENGTH() \
  if (domain.length() > PHP_GETTEXT_MAX_DOMAIN_LENGTH) { \
    raise_warning("domain passed too long"); \
    return false; \
  }

#define CHECK_GETTEXT_LENGTH(check_name, check_len) \
  if (check_len > PHP_GETTEXT_MAX_MSGID_LENGTH) { \
    raise_warning("%s passed too long", check_name); \
    return false; \
  }

Variant HHVM_FUNCTION(textdomain, const String& domain) {
  CHECK_DOMAIN_LENGTH();

  const char* realDomain = nullptr;
  if (!domain.empty() && domain != String(0)) {
    realDomain = domain.data();
  }

  return String(textdomain(realDomain), CopyString);
}

Variant HHVM_FUNCTION(gettext, const String& msgid) {
  CHECK_GETTEXT_LENGTH("msgid", msgid.length());
  return String(gettext(msgid.data()), CopyString);
}

Variant HHVM_FUNCTION(dgettext, const String& domain, const String& msgid) {
  CHECK_DOMAIN_LENGTH();
  CHECK_GETTEXT_LENGTH("msgid", msgid.length());
  return String(dgettext(domain.data(), msgid.data()), CopyString);
}

Variant HHVM_FUNCTION(dcgettext, const String& domain, const String& msgid,
                      int64_t category) {
  CHECK_DOMAIN_LENGTH();
  CHECK_GETTEXT_LENGTH("msgid", msgid.length());
  return String(dcgettext(domain.data(), msgid.data(), category), CopyString);
}

Variant HHVM_FUNCTION(bindtextdomain, const String& domain, const String& dir) {
  CHECK_DOMAIN_LENGTH();
  if (!domain.length()) {
    raise_warning("The first parameter of bindtextdomain must not be empty");
    return false;
  }

  char dirName[PATH_MAX];
  if (!dir.empty() && dir != String(0)) {
    auto tmp = File::TranslatePath(dir);
    if (tmp.empty()) {
      return false;
    }
    if (!realpath(tmp.c_str(), dirName)) {
      return false;
    }
  } else if (!getcwd(dirName, PATH_MAX)) {
    return false;
  }

  return String(bindtextdomain(domain.data(), dirName), CopyString);
}

#if HAVE_NGETTEXT
Variant HHVM_FUNCTION(ngettext, const String& msgid1, const String& msgid2,
                      int64_t count) {
  CHECK_GETTEXT_LENGTH("msgid1", msgid1.length());
  CHECK_GETTEXT_LENGTH("msgid2", msgid2.length());

  auto msgstr = ngettext(msgid1.data(), msgid2.data(), count);
  if (!msgstr) {
    return init_null();
  }
  return String(msgstr, CopyString);
}
#endif

#if HAVE_DNGETTEXT
Variant HHVM_FUNCTION(dngettext, const String& domain, const String& msgid1,
                      const String& msgid2, int64_t count) {
  CHECK_DOMAIN_LENGTH();
  CHECK_GETTEXT_LENGTH("msgid1", msgid1.length());
  CHECK_GETTEXT_LENGTH("msgid2", msgid2.length());

  auto msgstr = dngettext(domain.data(), msgid1.data(), msgid2.data(), count);
  if (!msgstr) {
    return init_null();
  }
  return String(msgstr, CopyString);
}
#endif

#if HAVE_DCNGETTEXT
Variant HHVM_FUNCTION(dcngettext, const String& domain, const String& msgid1,
                      const String& msgid2, int64_t count, int64_t category) {
  CHECK_DOMAIN_LENGTH();
  CHECK_GETTEXT_LENGTH("msgid1", msgid1.length());
  CHECK_GETTEXT_LENGTH("msgid2", msgid2.length());

  auto msgstr = dcngettext(domain.data(), msgid1.data(),
                           msgid2.data(), count, category);
  if (!msgstr) {
    return false;
  }
  return String(msgstr, CopyString);
}
#endif

#if HAVE_BIND_TEXTDOMAIN_CODESET
Variant HHVM_FUNCTION(bind_textdomain_codeset, const String& domain,
                      const String& codeset) {
  CHECK_DOMAIN_LENGTH();

  auto ret = bind_textdomain_codeset(domain.data(), codeset.data());
  if (!ret) {
    return false;
  }
  return String(ret, CopyString);
}
#endif

static GettextExtension s_gettext_extension;
void GettextExtension::moduleRegisterNative() {
  HHVM_FE(textdomain);
  HHVM_FE(gettext);
  HHVM_FALIAS(_, gettext);
  HHVM_FE(dgettext);
  HHVM_FE(dcgettext);
  HHVM_FE(bindtextdomain);
#if HAVE_NGETTEXT
  HHVM_FE(ngettext);
#endif
#if HAVE_DNGETTEXT
  HHVM_FE(dngettext);
#endif
#if HAVE_DCNGETTEXT
  HHVM_FE(dcngettext);
#endif
#if HAVE_BIND_TEXTDOMAIN_CODESET
  HHVM_FE(bind_textdomain_codeset);
#endif
}

void GettextExtension::moduleInfo(Array &info) {
  Extension::moduleInfo(info);
  info.set(String("GetText Support"), "enabled");
}

}
