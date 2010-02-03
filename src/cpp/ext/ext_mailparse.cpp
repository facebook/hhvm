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

#include <cpp/ext/ext_mailparse.h>
#include <cpp/base/runtime_option.h>
#include <cpp/ext/ext_process.h>
#include "crutch.h"

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// utility functions

/**
 * Removes whitespaces from the end, and replaces control characters with ' '
 * from the beginning.
 */
static String php_trim(CStrRef str) {
  string s(str.c_str());
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

///////////////////////////////////////////////////////////////////////////////

bool php_mail(CStrRef to, CStrRef subject, CStrRef message, CStrRef headers,
              CStrRef extra_cmd) {
  // assumes we always have sendmail installed
  assert(!RuntimeOption::SendmailPath.empty());

  ostringstream os;
  os << RuntimeOption::SendmailPath;
  if (!extra_cmd.empty()) {
    os << ' ' << extra_cmd.c_str();
  }

  errno = 0;
  FILE *sendmail = popen(os.str().c_str(), "w");
  if (sendmail == NULL || EACCES == errno) {
    Logger::Warning("Unable to execute %s",
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

static const StaticString zero("\0", 1);

bool f_mail(CStrRef to, CStrRef subject, CStrRef message, CStrRef additional_headers /* = null_string */, CStrRef additional_parameters /* = null_string */) {
  // replace \0 with spaces
  String to2 = to.replace(zero, " ");
  String subject2 = subject.replace(zero, " ");
  String message2 = message.replace(zero, " ");
  String headers2;
  if (!additional_headers.empty()) {
    headers2 = additional_headers.replace(zero, " ");
  }
  String params2;
  if (!additional_parameters.empty()) {
    params2 = additional_parameters.replace(zero, " ");
  }

  to2 = php_trim(to2);
  subject2 = php_trim(subject2);

  if (!RuntimeOption::MailForceExtraParameters.empty()) {
    params2 = f_escapeshellcmd(RuntimeOption::MailForceExtraParameters);
  } else {
    params2 = f_escapeshellcmd(params2);
  }

  return php_mail(to2, subject2, message2, headers2, params2);
}

int f_ezmlm_hash(CStrRef addr) {
  unsigned long h = 5381L;
  int str_len = addr.length();
  for (int i = 0; i < str_len; i++) {
    h = (h + (h << 5)) ^
        ((unsigned long)(unsigned char)tolower(addr.charAt(i)));
  }
  h = (h % 53);
  return (int)h;
}

Object f_mailparse_msg_create() {
  Array _schema(NEW(ArrayElement)(-1, "OO"), (ArrayElement*)NULL);
  Array _params((ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_msg_create", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_mailparse_msg_free(CObjRef mimemail) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mimemail)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_msg_free", _schema, _params);
  return (Variant)_ret[0];
}

Object f_mailparse_msg_parse_file(CStrRef filename) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(filename), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_msg_parse_file", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

bool f_mailparse_msg_parse(CObjRef mimemail, CStrRef data) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mimemail)), NEW(ArrayElement)(data), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_msg_parse", _schema, _params);
  return (Variant)_ret[0];
}

String f_mailparse_msg_extract_part_file(CObjRef mimemail, CStrRef filename, CStrRef callbackfunc /* = null_string */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mimemail)), NEW(ArrayElement)(filename), NEW(ArrayElement)(callbackfunc), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_msg_extract_part_file", _schema, _params);
  return (Variant)_ret[0];
}

String f_mailparse_msg_extract_whole_part_file(CObjRef mimemail, CStrRef filename, CStrRef callbackfunc /* = null_string */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mimemail)), NEW(ArrayElement)(filename), NEW(ArrayElement)(callbackfunc), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_msg_extract_whole_part_file", _schema, _params);
  return (Variant)_ret[0];
}

void f_mailparse_msg_extract_part(CObjRef mimemail, CStrRef msgbody, CStrRef callbackfunc /* = null_string */) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mimemail)), NEW(ArrayElement)(msgbody), NEW(ArrayElement)(callbackfunc), (ArrayElement*)NULL);
  Crutch::Invoke("mailparse_msg_extract_part", _schema, _params);
}

Array f_mailparse_msg_get_part_data(CObjRef mimemail) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mimemail)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_msg_get_part_data", _schema, _params);
  return (Variant)_ret[0];
}

Object f_mailparse_msg_get_part(CObjRef mimemail, CStrRef mimesection) {
  Array _schema(NEW(ArrayElement)(-1, "OO"), NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mimemail)), NEW(ArrayElement)(mimesection), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_msg_get_part", _schema, _params);
  return OpaqueObject::GetObject((Variant)_ret[0]);
}

Array f_mailparse_msg_get_structure(CObjRef mimemail) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(mimemail)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_msg_get_structure", _schema, _params);
  return (Variant)_ret[0];
}

Array f_mailparse_rfc822_parse_addresses(CStrRef addresses) {
  Array _schema((ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(addresses), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_rfc822_parse_addresses", _schema, _params);
  return (Variant)_ret[0];
}

bool f_mailparse_stream_encode(CObjRef sourcefp, CObjRef destfp, CStrRef encoding) {
  Array _schema(NEW(ArrayElement)(0, "O"), NEW(ArrayElement)(1, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(sourcefp)), NEW(ArrayElement)(OpaqueObject::GetIndex(destfp)), NEW(ArrayElement)(encoding), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_stream_encode", _schema, _params);
  return (Variant)_ret[0];
}

Array f_mailparse_uudecode_all(CObjRef fp) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(fp)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_uudecode_all", _schema, _params);
  return (Variant)_ret[0];
}

String f_mailparse_determine_best_xfer_encoding(CObjRef fp) {
  Array _schema(NEW(ArrayElement)(0, "O"), (ArrayElement*)NULL);
  Array _params(NEW(ArrayElement)(OpaqueObject::GetIndex(fp)), (ArrayElement*)NULL);
  Array _ret = Crutch::Invoke("mailparse_determine_best_xfer_encoding", _schema, _params);
  return (Variant)_ret[0];
}

///////////////////////////////////////////////////////////////////////////////
}
