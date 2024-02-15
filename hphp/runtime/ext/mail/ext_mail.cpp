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

#include <sstream>

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/zend-string.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// utility functions

/**
 * Detects multiple/malformed multiple newlines in mail headers.
 */
static int php_mail_detect_multiple_crlf(const String& str) {

  std::string hdr(str.c_str());
  unsigned int l = hdr.length();
  unsigned int i = 0;

  if (l == 0) {
    return 0;
  }

  /* Should not have any newlines at the beginning. */
  /* RFC 2822 2.2. Header Fields */
  if (hdr[0] < 33 || hdr[0] > 126 || hdr[0] == ':') {
    return 1;
  }

  while (i < l) {

    if (hdr[i] == '\r') {
      if ((i+1 == l) || (hdr[i+1] == '\r') || ((hdr[i+1] == '\n') && ((i+2 == l) || (hdr[i+2] == '\r') || (hdr[i+2] == '\n')))) {
        /* Malformed or multiple newlines. */
        return 1;
      } else {
        i += 2;
      }
    } else if (hdr[i] == '\n') {
      if ((i+1 == l) || (hdr[i+1] == '\r') || (hdr[i+1] == '\n')) {
        /* Malformed or multiple newlines. */
        return 1;
      } else {
        i += 2;
      }
    } else {
      i++;
    }
  }

  return 0;
}

/**
 * Removes whitespaces from the end.
 */
static String php_rtrim(const String& str) {
  std::string s(str.c_str());
  unsigned int l = s.length();
  while (l > 0 && isspace((unsigned char)s[l - 1])) {
    l--;
  }
  return s.substr(0, l);
}

/**
 * Removes whitespaces from the end, and replaces control characters with ' '
 * from the beginning.
 */
static String php_trim(const String& str) {
  std::string s(str.c_str());
  unsigned int l = s.length();
  while (l > 0 && isspace((unsigned char)s[l - 1])) {
    l--;
  }
  for (unsigned int i = 0; i < l; i++) {
    if (iscntrl((unsigned char)s[i])) {
      if (i + 2 < l && s[i] == '\r' && s[i + 1] == '\n' &&
          (s[i + 2] == ' ' || s[i + 2] == '\t')) {
        i += 2;
        while (i + 1 < l && (s[i + 1] == ' ' || s[i + 1] == '\t')) {
          i++;
        }
        continue;
      }
      s[i] = ' ';
    }
  }
  return s.substr(0, l);
}

bool php_mail(const String& to, const String& subject, const String& message,
              const String& headers, const String& extra_cmd) {
  // assumes we always have sendmail installed
  always_assert(!RuntimeOption::SendmailPath.empty());

  std::ostringstream os;
  os << RuntimeOption::SendmailPath;
  if (!extra_cmd.empty()) {
    os << ' ' << extra_cmd.c_str();
  }

  errno = 0;
  FILE *sendmail = popen(os.str().c_str(), "w");
  if (sendmail == NULL || EACCES == errno) {
    raise_warning("Unable to execute %s",
                  RuntimeOption::SendmailPath.c_str());
    return false;
  }

  fprintf(sendmail, "To: %s\n", to.c_str());
  fprintf(sendmail, "Subject: %s\n", subject.c_str());
  if (!headers.empty()) {
    fprintf(sendmail, "%s\n", headers.c_str());
  }
  fprintf(sendmail, "\n%s\n", message.c_str());

  int ret = pclose(sendmail);
  return (!ret);
}

///////////////////////////////////////////////////////////////////////////////

const StaticString
  s_zero("\0"),
  s_space(" ");

bool HHVM_FUNCTION(mail,
                   const String& to,
                   const String& subject,
                   const String& message,
                   const String& additional_headers /* = null_string */,
                   const String& additional_parameters /* = null_string */) {

  // replace \0 with spaces
  String to2 = string_replace(to, s_zero, s_space);
  String subject2 = string_replace(subject, s_zero, s_space);
  String message2 = string_replace(message, s_zero, s_space);
  String headers2;
  if (!additional_headers.empty()) {
    headers2 = string_replace(additional_headers, s_zero, s_space);
    headers2 = php_rtrim(headers2);

    if (php_mail_detect_multiple_crlf(headers2)) {
      raise_warning("Multiple or malformed newlines found in additional_headers");
      return false;
    }
  }
  String params2;
  if (!additional_parameters.empty()) {
    params2 = string_replace(additional_parameters, s_zero, s_space);
  }

  to2 = php_trim(to2);
  subject2 = php_trim(subject2);

  if (!RuntimeOption::MailForceExtraParameters.empty()) {
    params2 = string_escape_shell_cmd(
      RuntimeOption::MailForceExtraParameters.c_str());
  } else {
    params2 = string_escape_shell_cmd(params2.c_str());
  }

  return php_mail(to2, subject2, message2, headers2, params2);
}

int64_t HHVM_FUNCTION(ezmlm_hash, const String& addr) {
  unsigned int h = 5381;
  int str_len = addr.length();
  for (int i = 0; i < str_len; i++) {
    h = (h + (h << 5)) ^
        ((unsigned long)tolower(addr.charAt(i)));
  }
  h = (h % 53);
  return (int64_t)h;
}

///////////////////////////////////////////////////////////////////////////////

struct MailExtension final : Extension {
  MailExtension() : Extension("mail", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) { }
  void moduleRegisterNative() override {
    HHVM_FE(mail);
    HHVM_FE(ezmlm_hash);
  }
} s_mail_extension;

///////////////////////////////////////////////////////////////////////////////
}
