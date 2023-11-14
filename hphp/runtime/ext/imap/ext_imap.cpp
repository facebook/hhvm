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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/logger.h"
#include "hphp/util/rds-local.h"

#include <c-client.h> /* includes mail.h and rfc822.h */
#define namespace namespace_
#include <imap4r1.h>  /* location of c-client quota functions */
#undef namespace

#define PHP_EXPUNGE 32768
#define PHP_IMAP_ADDRESS_SIZE_BUF 10

#ifndef SENDBUFLEN
#define SENDBUFLEN 16385
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct ImapStream : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(ImapStream);

  ImapStream(MAILSTREAM *stream, int64_t flag)
      : m_stream(stream), m_flag(flag) {
  }
  ~ImapStream() override {
    close();
  }

  // overriding ResourceData
  CLASSNAME_IS("imap");
  const String& o_getClassNameHook() const override { return classnameof(); }

  void close();

  bool checkMsgNumber(int64_t msgindex) {
    if ((msgindex < 1) || ((unsigned) msgindex > m_stream->nmsgs)) {
      Logger::Warning("Bad message number");
      return false;
    }
    return true;
  }

  MAILSTREAM *m_stream;
  int64_t m_flag;
};

IMPLEMENT_RESOURCE_ALLOCATION(ImapStream);

///////////////////////////////////////////////////////////////////////////////

enum FolderListStyle {
  FLIST_ARRAY,
  FLIST_OBJECT
};

struct FOBJECTLIST {
  SIZEDTEXT text;
  int delimiter;
  long attributes;
  struct FOBJECTLIST *next;
};

struct ERRORLIST {
  SIZEDTEXT text;
  long errflg;
  struct ERRORLIST *next;
};
static void mail_free_errorlist(ERRORLIST **errlist);

struct MESSAGELIST {
  unsigned long msgid;
  struct MESSAGELIST *next;
};

struct ImapRequestData final : RequestEventHandler {
  void requestInit() override {
    m_user.clear();
    m_password.clear();

    m_errorstack = NIL;
    m_alertstack = NIL;
    //m_gets_stream = NIL;
  }
  void requestShutdown() override {
    if (m_errorstack != NIL) {
      /* output any remaining errors at their original error level */
      for (ERRORLIST *ecur = m_errorstack; ecur != NIL; ecur = ecur->next) {
        Logger::Warning("%s (errflg=%ld)", ecur->text.data, ecur->errflg);
      }
      mail_free_errorlist(&m_errorstack);
      m_errorstack = NIL;
    }

    if (m_alertstack != NIL) {
      /* output any remaining alerts at E_NOTICE level */
      for (STRINGLIST *acur = m_alertstack; acur != NIL; acur = acur->next) {
        Logger::Warning("%s", acur->text.data);
      }
      mail_free_stringlist(&m_alertstack);
      m_alertstack = NIL;
    }
  }

  std::string m_user;
  std::string m_password;

  STRINGLIST  *m_alertstack;
  ERRORLIST   *m_errorstack;

  STRINGLIST  *m_folders;
  STRINGLIST  *m_folders_tail;
  STRINGLIST  *m_sfolders;
  STRINGLIST  *m_sfolders_tail;
  MESSAGELIST *m_messages;
  MESSAGELIST *m_messages_tail;
  FOBJECTLIST *m_folder_objects;
  FOBJECTLIST *m_folder_objects_tail;
  FOBJECTLIST *m_sfolder_objects;
  FOBJECTLIST *m_sfolder_objects_tail;

  FolderListStyle m_folderlist_style;
  long m_status_flags;
  unsigned long m_status_messages;
  unsigned long m_status_recent;
  unsigned long m_status_unseen;
  unsigned long m_status_uidnext;
  unsigned long m_status_uidvalidity;
};
IMPLEMENT_STATIC_REQUEST_LOCAL(ImapRequestData, s_imap_data);
#define IMAPG(name) s_imap_data->m_ ## name

///////////////////////////////////////////////////////////////////////////////

void ImapStream::close() {
  if (m_stream) {
    mail_close_full(m_stream, m_flag);
    IMAPG(user).clear();
    IMAPG(password).clear();
    m_stream = NULL;
  }
}

/**
 * Calculate string length based on imap's rfc822_cat function.
 */
static int _php_rfc822_len(char *str) {
  if (!str || !*str) {
    return 0;
  }

  /* strings with special characters will need to be quoted, as a safety
   * measure we add 2 bytes for the quotes just in case.
   */
  int len = strlen(str) + 2;
  char *p = str;

  /* rfc822_cat() will escape all " and \ characters, therefor we need to
   * increase our buffer length to account for these characters.
   */
  while ((p = strpbrk(p, "\\\""))) {
    p++;
    len++;
  }

  return len;
}

static int _php_imap_address_size (ADDRESS *addresslist) {
  int ret=0, num_ent=0;

  ADDRESS *tmp = addresslist;
  if (tmp) {
    do {
      ret += _php_rfc822_len(tmp->personal);
      ret += _php_rfc822_len(tmp->adl);
      ret += _php_rfc822_len(tmp->mailbox);
      ret += _php_rfc822_len(tmp->host);
      num_ent++;
    } while ((tmp = tmp->next));
  }

  /*
   * rfc822_write_address_full() needs some extra space for '<>,', etc.
   * for this perpouse we allocate additional PHP_IMAP_ADDRESS_SIZE_BUF bytes
   * by default this buffer is 10 bytes long
  */
  ret += (ret) ? num_ent*PHP_IMAP_ADDRESS_SIZE_BUF : 0;

  return ret;
}

static char *_php_rfc822_write_address(ADDRESS *addresslist) {
  char address[SENDBUFLEN];
  if (_php_imap_address_size(addresslist) >= SENDBUFLEN) {
    Logger::Error("Address buffer overflow");
    return NULL;
  }
  address[0] = 0;
  rfc822_write_address(address, addresslist);
  return strdup(address);
}

#define ARR_SET_ENTRY(arr, obj, name, entry) \
  if (obj->entry) arr.set(LAZY_STATIC_STRING(name), String((const char*)obj->entry, CopyString));

static void set_address(DictInit &ai, const char *prop, ADDRESS *addr) {
  if (!addr) return;

  auto const fulladdress = _php_rfc822_write_address(addr);
  if (fulladdress) {
    ai.set(String(prop) + "address", String(fulladdress, AttachString));
  }

  auto paddress = Array::CreateVec();
  do {
    DictInit props(4);
    ARR_SET_ENTRY(props, addr, "personal", personal);
    ARR_SET_ENTRY(props, addr, "adl",      adl);
    ARR_SET_ENTRY(props, addr, "mailbox",  mailbox);
    ARR_SET_ENTRY(props, addr, "host",     host);
    paddress.append(ObjectData::FromArray(props.create()));
  } while ((addr = addr->next));
  ai.set(String(prop), paddress);
}

static Array _php_make_header_props(ENVELOPE *en) {
  DictInit ai(24);

  ARR_SET_ENTRY(ai, en, "remail",      remail);
  ARR_SET_ENTRY(ai, en, "date",        date);
  ARR_SET_ENTRY(ai, en, "Date",        date);
  ARR_SET_ENTRY(ai, en, "subject",     subject);
  ARR_SET_ENTRY(ai, en, "Subject",     subject);
  ARR_SET_ENTRY(ai, en, "in_reply_to", in_reply_to);
  ARR_SET_ENTRY(ai, en, "message_id",  message_id);
  ARR_SET_ENTRY(ai, en, "newsgroups",  newsgroups);
  ARR_SET_ENTRY(ai, en, "followup_to", followup_to);
  ARR_SET_ENTRY(ai, en, "references",  references);

  set_address(ai, "to",          en->to);
  set_address(ai, "from",        en->from);
  set_address(ai, "cc",          en->cc);
  set_address(ai, "bcc",         en->bcc);
  set_address(ai, "reply_to",    en->reply_to);
  set_address(ai, "sender",      en->sender);
  set_address(ai, "return_path", en->return_path);

  return ai.toArray();
}

static Object _php_imap_body(BODY *body, bool do_multipart) {
  DictInit props(17);

  if (body->type <= TYPEMAX) {
   props.set(LAZY_STATIC_STRING("type"), body->type);
  }

  if (body->encoding <= ENCMAX) {
    props.set(LAZY_STATIC_STRING("encoding"), body->encoding);
  }

  if (body->subtype) {
    props.set(LAZY_STATIC_STRING("ifsubtype"), 1);
    props.set(LAZY_STATIC_STRING("subtype"), String((const char*)body->subtype, CopyString));
  } else {
    props.set(LAZY_STATIC_STRING("ifsubtype"), 0);
  }

  if (body->description) {
    props.set(LAZY_STATIC_STRING("ifdescription"), 1);
    props.set(LAZY_STATIC_STRING("description"),
      String((const char*)body->description, CopyString));
  } else {
    props.set(LAZY_STATIC_STRING("ifdescription"), 0);
  }

  if (body->id) {
    props.set(LAZY_STATIC_STRING("ifid"), 1);
    props.set(LAZY_STATIC_STRING("id"), String((const char*)body->id, CopyString));
  } else {
    props.set(LAZY_STATIC_STRING("ifid"), 0);
  }


  if (body->size.lines) {
    props.set(LAZY_STATIC_STRING("lines"), (int64_t)body->size.lines);
  }

  if (body->size.bytes) {
    props.set(LAZY_STATIC_STRING("bytes"), (int64_t)body->size.bytes);
  }

  if (body->disposition.type) {
    props.set(LAZY_STATIC_STRING("ifdisposition"), 1);
    props.set(LAZY_STATIC_STRING("disposition"),
      String((const char*)body->disposition.type, CopyString));
  } else {
    props.set(LAZY_STATIC_STRING("ifdisposition"), 0);
  }

  if (body->disposition.parameter) {
    props.set(LAZY_STATIC_STRING("ifdparameters"), 1);

    auto dparams = Array::CreateVec();
    for (auto dpar = body->disposition.parameter; dpar; dpar = dpar->next) {
      DictInit dparam(2);
      dparam.set(LAZY_STATIC_STRING("attribute"), String((const char*)dpar->attribute, CopyString));
      dparam.set(LAZY_STATIC_STRING("value"), String((const char*)dpar->value, CopyString));
      dparams.append(ObjectData::FromArray(dparam.create()));
    }
    props.set(LAZY_STATIC_STRING("dparameters"), dparams);
  } else {
    props.set(LAZY_STATIC_STRING("ifdparameters"), 0);
  }

  if (body->parameter) {
    props.set(LAZY_STATIC_STRING("ifparameters"), 1);

    auto params = Array::CreateVec();
    for (auto par = body->parameter; par; par = par->next) {
      DictInit param(2);
      ARR_SET_ENTRY(param, par, "attribute", attribute);
      ARR_SET_ENTRY(param, par, "value", value);
      params.append(ObjectData::FromArray(param.create()));
    }
    props.set(LAZY_STATIC_STRING("parameters"), std::move(params));
  } else {
    props.set(LAZY_STATIC_STRING("ifparameters"), 0);
  }

  if (do_multipart) {
    /* multipart message ? */
    if (body->type == TYPEMULTIPART) {
      auto parts = Array::CreateVec();
      for (auto part = body->nested.part; part; part = part->next) {
        parts.append(_php_imap_body(&part->body, do_multipart));
      }
      props.set(LAZY_STATIC_STRING("parts"), std::move(parts));
    }

    /* encapsulated message ? */
    if ((body->type == TYPEMESSAGE) && (!strcasecmp(body->subtype, "rfc822"))) {
      auto parts = Array::CreateVec();
      parts.append(_php_imap_body(body->nested.msg->body, do_multipart));
      props.set(LAZY_STATIC_STRING("parts"), parts);
    }
  }

  return ObjectData::FromArray(props.create());
}

///////////////////////////////////////////////////////////////////////////////
// interfaces to C-client

/* {{{ mail_newfolderobjectlist
 *
 * Mail instantiate FOBJECTLIST
 * Returns: new FOBJECTLIST list
 * Author: CJH
 */
static FOBJECTLIST *mail_newfolderobjectlist(void) {
  return (FOBJECTLIST *)
    memset(fs_get(sizeof(FOBJECTLIST)), 0, sizeof(FOBJECTLIST));
}
/* }}} */

/* {{{ mail_free_foblist
 *
 * Mail garbage collect FOBJECTLIST
 * Accepts: pointer to FOBJECTLIST pointer
 * Author: CJH
 */
void mail_free_foblist(FOBJECTLIST **foblist, FOBJECTLIST **tail) {
  FOBJECTLIST *cur, *next;
  for (cur=*foblist, next=cur->next; cur; cur=next) {
    next = cur->next;

    if (cur->text.data)
      fs_give((void **)&(cur->text.data));

    fs_give((void **)&cur);
  }

  *tail = NIL;
  *foblist = NIL;
}
/* }}} */

/* {{{ mail_newerrorlist
 *
 * Mail instantiate ERRORLIST
 * Returns: new ERRORLIST list
 * Author: CJH
 */
static ERRORLIST *mail_newerrorlist(void) {
  return (ERRORLIST *) memset(fs_get(sizeof(ERRORLIST)), 0, sizeof(ERRORLIST));
}
/* }}} */

/* {{{ mail_free_errorlist
 *
 * Mail garbage collect FOBJECTLIST
 * Accepts: pointer to FOBJECTLIST pointer
 * Author: CJH
 */
static void mail_free_errorlist(ERRORLIST **errlist) {
  if (*errlist) {    /* only free if exists */
    if ((*errlist)->text.data) {
      fs_give((void **) &(*errlist)->text.data);
    }
    mail_free_errorlist (&(*errlist)->next);
    fs_give((void **) errlist);  /* return string to free storage */
  }
}
/* }}} */

/* {{{ mail_newmessagelist
 *
 * Mail instantiate MESSAGELIST
 * Returns: new MESSAGELIST list
 * Author: CJH
 */
static MESSAGELIST *mail_newmessagelist(void) {
  return (MESSAGELIST *) memset(fs_get(sizeof(MESSAGELIST)), 0,
                                sizeof(MESSAGELIST));
}
/* }}} */

/* {{{ mail_free_messagelist
 *
 * Mail garbage collect MESSAGELIST
 * Accepts: pointer to MESSAGELIST pointer
 * Author: CJH
 */
void mail_free_messagelist(MESSAGELIST **msglist, MESSAGELIST **tail) {
  MESSAGELIST *cur, *next;

  for (cur = *msglist, next = cur->next; cur; cur = next) {
    next = cur->next;
    fs_give((void **)&cur);
  }

  *tail = NIL;
  *msglist = NIL;
}
/* }}} */

static int mm_strlen(unsigned char *str) {
  return strlen((const char *)str);
}

static unsigned char *mm_cpystr(char *str) {
  return (unsigned char *)cpystr(str);
}

extern "C" {

void mm_searched(MAILSTREAM* /*stream*/, unsigned long number) {
  MESSAGELIST *cur = NIL;
  if (IMAPG(messages) == NIL) {
    IMAPG(messages) = mail_newmessagelist();
    IMAPG(messages)->msgid = number;
    IMAPG(messages)->next = NIL;
    IMAPG(messages_tail) = IMAPG(messages);
  } else {
    cur = IMAPG(messages_tail);
    cur->next = mail_newmessagelist();
    cur = cur->next;
    cur->msgid = number;
    cur->next = NIL;
    IMAPG(messages_tail) = cur;
  }
}

void mm_exists(MAILSTREAM* /*stream*/, unsigned long /*number*/) {}
void mm_expunged(MAILSTREAM* /*stream*/, unsigned long /*number*/) {}
void mm_flags(MAILSTREAM* /*stream*/, unsigned long /*number*/) {}

/* Author: CJH */
void mm_notify(MAILSTREAM* /*stream*/, char* str, long /*errflg*/) {
  STRINGLIST *cur = NIL;
  if (strncmp(str, "[ALERT] ", 8) == 0) {
    if (IMAPG(alertstack) == NIL) {
      IMAPG(alertstack) = mail_newstringlist();
      IMAPG(alertstack)->text.size =
        mm_strlen((IMAPG(alertstack)->text.data = mm_cpystr(str)));
      IMAPG(alertstack)->next = NIL;
    } else {
      cur = IMAPG(alertstack);
      while (cur->next != NIL) {
        cur = cur->next;
      }
      cur->next = mail_newstringlist ();
      cur = cur->next;
      cur->text.size = mm_strlen(cur->text.data = mm_cpystr(str));
      cur->next = NIL;
    }
  }
}

void mm_list(MAILSTREAM* /*stream*/, int delimiter, char* mailbox,
             long attributes) {
  STRINGLIST *cur=NIL;
  FOBJECTLIST *ocur=NIL;

  if (IMAPG(folderlist_style) == FLIST_OBJECT) {
    /* build up a the new array of objects */
    /* Author: CJH */
    if (IMAPG(folder_objects) == NIL) {
      IMAPG(folder_objects) = mail_newfolderobjectlist();
      IMAPG(folder_objects)->text.size =
        mm_strlen(IMAPG(folder_objects)->text.data=mm_cpystr(mailbox));
      IMAPG(folder_objects)->delimiter = delimiter;
      IMAPG(folder_objects)->attributes = attributes;
      IMAPG(folder_objects)->next = NIL;
      IMAPG(folder_objects_tail) = IMAPG(folder_objects);
    } else {
      ocur=IMAPG(folder_objects_tail);
      ocur->next=mail_newfolderobjectlist();
      ocur=ocur->next;
      ocur->text.size = mm_strlen(ocur->text.data = mm_cpystr(mailbox));
      ocur->delimiter = delimiter;
      ocur->attributes = attributes;
      ocur->next = NIL;
      IMAPG(folder_objects_tail) = ocur;
    }

  } else {
    /* build the old IMAPG(folders) variable to allow old
       imap_listmailbox() to work */
    if (!(attributes & LATT_NOSELECT)) {
      if (IMAPG(folders) == NIL) {
        IMAPG(folders)=mail_newstringlist();
        IMAPG(folders)->text.size =
          mm_strlen(IMAPG(folders)->text.data=mm_cpystr(mailbox));
        IMAPG(folders)->next=NIL;
        IMAPG(folders_tail) = IMAPG(folders);
      } else {
        cur=IMAPG(folders_tail);
        cur->next=mail_newstringlist ();
        cur=cur->next;
        cur->text.size = mm_strlen (cur->text.data = mm_cpystr (mailbox));
        cur->next = NIL;
        IMAPG(folders_tail) = cur;
      }
    }
  }
}

void mm_lsub(MAILSTREAM* /*stream*/, int delimiter, char* mailbox,
             long attributes) {
  STRINGLIST *cur=NIL;
  FOBJECTLIST *ocur=NIL;

  if (IMAPG(folderlist_style) == FLIST_OBJECT) {
    /* build the array of objects */
    /* Author: CJH */
    if (IMAPG(sfolder_objects) == NIL) {
      IMAPG(sfolder_objects) = mail_newfolderobjectlist();
      IMAPG(sfolder_objects)->text.size =
        mm_strlen(IMAPG(sfolder_objects)->text.data=mm_cpystr(mailbox));
      IMAPG(sfolder_objects)->delimiter = delimiter;
      IMAPG(sfolder_objects)->attributes = attributes;
      IMAPG(sfolder_objects)->next = NIL;
      IMAPG(sfolder_objects_tail) = IMAPG(sfolder_objects);
    } else {
      ocur=IMAPG(sfolder_objects_tail);
      ocur->next=mail_newfolderobjectlist();
      ocur=ocur->next;
      ocur->text.size=mm_strlen(ocur->text.data = mm_cpystr(mailbox));
      ocur->delimiter = delimiter;
      ocur->attributes = attributes;
      ocur->next = NIL;
      IMAPG(sfolder_objects_tail) = ocur;
    }
  } else {
    /* build the old simple array for imap_listsubscribed() */
    if (IMAPG(sfolders) == NIL) {
      IMAPG(sfolders)=mail_newstringlist();
      IMAPG(sfolders)->text.size =
        mm_strlen(IMAPG(sfolders)->text.data=mm_cpystr(mailbox));
      IMAPG(sfolders)->next=NIL;
      IMAPG(sfolders_tail) = IMAPG(sfolders);
    } else {
      cur=IMAPG(sfolders_tail);
      cur->next=mail_newstringlist ();
      cur=cur->next;
      cur->text.size = mm_strlen (cur->text.data = mm_cpystr (mailbox));
      cur->next = NIL;
      IMAPG(sfolders_tail) = cur;
    }
  }
}

void mm_status(MAILSTREAM* /*stream*/, char* /*mailbox*/, MAILSTATUS* status) {
  IMAPG(status_flags)=status->flags;
  if (IMAPG(status_flags) & SA_MESSAGES) {
    IMAPG(status_messages)=status->messages;
  }
  if (IMAPG(status_flags) & SA_RECENT) {
    IMAPG(status_recent)=status->recent;
  }
  if (IMAPG(status_flags) & SA_UNSEEN) {
    IMAPG(status_unseen)=status->unseen;
  }
  if (IMAPG(status_flags) & SA_UIDNEXT) {
    IMAPG(status_uidnext)=status->uidnext;
  }
  if (IMAPG(status_flags) & SA_UIDVALIDITY) {
    IMAPG(status_uidvalidity)=status->uidvalidity;
  }
}

void mm_log(char *str, long errflg) {
  ERRORLIST *cur = NIL;

  /* Author: CJH */
  if (errflg != NIL) { /* CJH: maybe put these into a more comprehensive
                          log for debugging purposes? */
    if (IMAPG(errorstack) == NIL) {
      IMAPG(errorstack) = mail_newerrorlist();
      IMAPG(errorstack)->text.size =
        mm_strlen(IMAPG(errorstack)->text.data = mm_cpystr(str));
      IMAPG(errorstack)->errflg = errflg;
      IMAPG(errorstack)->next = NIL;
    } else {
      cur = IMAPG(errorstack);
      while (cur->next != NIL) {
        cur = cur->next;
      }
      cur->next = mail_newerrorlist();
      cur = cur->next;
      cur->text.size = mm_strlen(cur->text.data = mm_cpystr(str));
      cur->errflg = errflg;
      cur->next = NIL;
    }
  }
}

void mm_login(NETMBX* mb, char* user, char* pwd, long /*trial*/) {
  if (*mb->user) {
    string_copy(user, mb->user, MAILTMPLEN);
  } else {
    string_copy(user, IMAPG(user).c_str(), MAILTMPLEN);
  }
  string_copy(pwd, IMAPG(password).c_str(), MAILTMPLEN);
}

void mm_dlog(char* /*str*/) {}
void mm_critical(MAILSTREAM* /*stream*/) {}
void mm_nocritical(MAILSTREAM* /*stream*/) {}
long mm_diskerror(MAILSTREAM* /*stream*/, long /*errcode*/, long /*serious*/) {
  return 1;
}
void mm_fatal(char* /*str*/) {}

} // extern "C"

///////////////////////////////////////////////////////////////////////////////

static Variant HHVM_FUNCTION(imap_8bit, const String& str) {
  unsigned long newlength;

  char *decode = (char *)rfc822_8bit((unsigned char *) str.data(),
                                        str.length(), &newlength);
  if (decode == NULL) {
    return false;
  }

  String ret = String(decode, newlength, CopyString);
  fs_give((void**) &decode);
  return ret;
}

static Variant HHVM_FUNCTION(imap_alerts) {
  if (IMAPG(alertstack) == NIL) {
    return false;
  }

  Array ret(Array::CreateVec());

  for (STRINGLIST *cur = IMAPG(alertstack); cur != NIL;
       cur = cur->next) {
      ret.append(String((const char *)cur->text.data, CopyString));
  }
  mail_free_stringlist(&IMAPG(alertstack));
  IMAPG(alertstack) = NIL;
  return ret;
}

static Variant HHVM_FUNCTION(imap_base64, const String& text) {
  unsigned long newlength;

  char *decode = (char *)rfc822_base64((unsigned char *) text.data(),
                                        text.length(), &newlength);
  if (decode == NULL) {
    return false;
  }

  String ret = String(decode, newlength, CopyString);
  fs_give((void**) &decode);
  return ret;
}

static Variant HHVM_FUNCTION(imap_binary, const String& str) {
  unsigned long newlength;

  char *decode = (char *)rfc822_binary((unsigned char *) str.data(),
                                        str.length(), &newlength);
  if (decode == NULL) {
    return false;
  }

  String ret = String(decode, newlength, CopyString);
  fs_give((void**) &decode);
  return ret;
}

static Variant HHVM_FUNCTION(imap_body, const OptResource& imap_stream,
                             int64_t msg_number, int64_t options /* = 0 */) {
  if (options && ((options & ~(FT_UID|FT_PEEK|FT_INTERNAL)) != 0)) {
    raise_warning("invalid value for the options parameter");
    return false;
  }

  auto obj = cast<ImapStream>(imap_stream);

  int msgindex;
  if (options & FT_UID) {
    /* This should be cached; if it causes an extra RTT to the
       IMAP server, then that's the price we pay for making sure
       we don't crash. */
    msgindex = mail_msgno(obj->m_stream, msg_number);
  } else {
    msgindex = msg_number;
  }

  if (!obj->checkMsgNumber(msgindex)) {
    return false;
  }

  unsigned long body_len = 0;
  char *body = mail_fetchtext_full(obj->m_stream, msg_number,
                                   &body_len, (options ? options : NIL));
  if (body_len == 0) {
    return empty_string_variant();
  } else {
    return String(body, body_len, CopyString);
  }
}

static Variant HHVM_FUNCTION(imap_bodystruct, const OptResource& imap_stream,
                             int64_t msg_number, const String& section) {
  auto obj = cast<ImapStream>(imap_stream);
  if (!obj->checkMsgNumber(msg_number)) {
    return false;
  }

  BODY *body;
  body = mail_body(obj->m_stream, msg_number, (unsigned char *)section.data());
  if (body == NULL) {
    return false;
  }

  return _php_imap_body(body, false);
}

static Variant HHVM_FUNCTION(imap_check, const OptResource& imap_stream) {
  auto obj = cast<ImapStream>(imap_stream);
  if (mail_ping(obj->m_stream) == NIL) {
    return false;
  }
  if (obj->m_stream && obj->m_stream->mailbox) {
    DictInit props(5);
    char date[100];
    rfc822_date(date);
    props.set(LAZY_STATIC_STRING("Date"), String(date, CopyString));
    props.set(LAZY_STATIC_STRING("Driver"), String(obj->m_stream->dtb->name, CopyString));
    props.set(LAZY_STATIC_STRING("Mailbox"), String(obj->m_stream->mailbox, CopyString));
    props.set(LAZY_STATIC_STRING("Nmsgs"), (int64_t)obj->m_stream->nmsgs);
    props.set(LAZY_STATIC_STRING("Recent"), (int64_t)obj->m_stream->recent);
    return ObjectData::FromArray(props.create());
  }
  return false;
}

static bool HHVM_FUNCTION(imap_clearflag_full, const OptResource& imap_stream,
                          const String& sequence, const String& flag,
                          int64_t options /* = 0 */) {
  auto obj = cast<ImapStream>(imap_stream);
  mail_clearflag_full(obj->m_stream, (char *)sequence.data(),
                      (char *)flag.data(), (options ? options : NIL));
  return true;
}

static bool HHVM_FUNCTION(imap_close, const OptResource& imap_stream,
                          int64_t flag /* = 0 */) {
  auto obj = cast<ImapStream>(imap_stream);
  if (flag) {
    if (flag != PHP_EXPUNGE) {
      Logger::Warning("invalid value for the flags parameter");
      return false;
    }
    flag = CL_EXPUNGE;
  }
  obj->m_flag = flag;
  obj->close();
  return true;
}

static bool HHVM_FUNCTION(imap_createmailbox, const OptResource& imap_stream,
                          const String& mailbox) {
  auto obj = cast<ImapStream>(imap_stream);
  if (mail_create(obj->m_stream, (char *)mailbox.data()) == T) {
    return true;
  } else {
    return false;
  }
}

static bool HHVM_FUNCTION(imap_delete, const OptResource& imap_stream,
                          const String& msg_number, int64_t options /* = 0 */) {
  auto obj = cast<ImapStream>(imap_stream);
  mail_setflag_full(obj->m_stream, (char *)msg_number.data(),
                    "\\DELETED",
                    (options ? options : NIL));
  return true;
}

static bool HHVM_FUNCTION(imap_deletemailbox, const OptResource& imap_stream,
                          const String& mailbox) {
  auto obj = cast<ImapStream>(imap_stream);
  if (mail_delete(obj->m_stream, (char *)mailbox.data()) == T) {
    return true;
  } else {
    return false;
  }
}

static Variant HHVM_FUNCTION(imap_errors, ) {
  if (IMAPG(errorstack) == NIL) {
    return false;
  }

  Array ret(Array::CreateVec());

  for (ERRORLIST *cur = IMAPG(errorstack); cur != NIL;
       cur = cur->next) {
      ret.append(String((const char *)cur->text.data, CopyString));
  }
  IMAPG(errorstack) = NIL;
  return ret;
}

static bool HHVM_FUNCTION(imap_expunge, const OptResource& imap_stream) {
  auto obj = cast<ImapStream>(imap_stream);
  mail_expunge(obj->m_stream);
  return true;
}

static Variant HHVM_FUNCTION(imap_fetch_overview, const OptResource& imap_stream,
                             const String& sequence,
                             int64_t options /* = 0 */) {
  if (options && options != FT_UID) {
    Logger::Warning("invalid value for the options parameter");
    return false;
  }

  auto obj = cast<ImapStream>(imap_stream);

  Array ret(Array::CreateVec());

  long status = (options & FT_UID)
    ? mail_uid_sequence(obj->m_stream, (unsigned char *)sequence.data())
    : mail_sequence(obj->m_stream, (unsigned char *)sequence.data());

  if (status) {
    MESSAGECACHE *elt;
    ENVELOPE *env;
    for (unsigned long i = 1; i <= obj->m_stream->nmsgs; i++) {
      if (((elt = mail_elt(obj->m_stream, i))->sequence) &&
          (env = mail_fetch_structure(obj->m_stream, i, NIL, NIL))) {

        DictInit props(16);
        ARR_SET_ENTRY(props, env, "subject", subject);

        if (env->from) {
          env->from->next = NULL;
          char *address = _php_rfc822_write_address(env->from);
          if (address) {
            props.set(LAZY_STATIC_STRING("from"), String(address, AttachString));
          }
        }
        if (env->to) {
          env->to->next = NULL;
          char *address = _php_rfc822_write_address(env->to);
          if (address) {
            props.set(LAZY_STATIC_STRING("to"), String(address, AttachString));
          }
        }

        ARR_SET_ENTRY(props, env, "date",        date);
        ARR_SET_ENTRY(props, env, "message_id",  message_id);
        ARR_SET_ENTRY(props, env, "references",  references);
        ARR_SET_ENTRY(props, env, "in_reply_to", in_reply_to);

        props.set(LAZY_STATIC_STRING("size"),     (int64_t)elt->rfc822_size);
        props.set(LAZY_STATIC_STRING("uid"),      (int64_t)mail_uid(obj->m_stream, i));
        props.set(LAZY_STATIC_STRING("msgno"),    (int64_t)i);
        props.set(LAZY_STATIC_STRING("recent"),   (int64_t)elt->recent);
        props.set(LAZY_STATIC_STRING("flagged"),  (int64_t)elt->flagged);
        props.set(LAZY_STATIC_STRING("answered"), (int64_t)elt->answered);
        props.set(LAZY_STATIC_STRING("deleted"),  (int64_t)elt->deleted);
        props.set(LAZY_STATIC_STRING("seen"),     (int64_t)elt->seen);
        props.set(LAZY_STATIC_STRING("draft"),    (int64_t)elt->draft);

        ret.append(ObjectData::FromArray(props.create()));
      }
    }
  }

  return ret;
}

static Variant HHVM_FUNCTION(imap_fetchbody, const OptResource& imap_stream,
                             int64_t msg_number, const String& section,
                             int64_t options /* = 0 */) {
  if (options && ((options & ~(FT_UID|FT_PEEK|FT_INTERNAL)) != 0)) {
    raise_warning("invalid value for the options parameter");
    return false;
  }

  auto obj = cast<ImapStream>(imap_stream);

  if (!options || !(options & FT_UID)) {
    if (!obj->checkMsgNumber(msg_number)) {
      return false;
    }
  }

  unsigned long len;
  char *body = mail_fetchbody_full(obj->m_stream, msg_number,
                                   (char*)section.data(),
                                   &len, (options ? options : NIL));

  if (!body) {
    raise_warning("No body information available");
    return false;
  }

  return String(body, len, CopyString);
}

static Variant HHVM_FUNCTION(imap_fetchheader, const OptResource& imap_stream,
                             int64_t msg_number, int64_t options /* = 0 */) {
  if (options && ((options & ~(FT_UID|FT_INTERNAL|FT_PREFETCHTEXT)) != 0)) {
    Logger::Warning("invalid value for the options parameter");
    return false;
  }

  auto obj = cast<ImapStream>(imap_stream);
  int msgindex;
  if (options & FT_UID) {
    /* This should be cached; if it causes an extra RTT to the
       IMAP server, then that's the price we pay for making sure
       we don't crash. */
    msgindex = mail_msgno(obj->m_stream, msg_number);
  } else {
    msgindex = msg_number;
  }

  if (!obj->checkMsgNumber(msgindex)) {
    return false;
  }

  return String(mail_fetchheader_full(obj->m_stream, msgindex, NIL, NIL,
                                      (options ? options : NIL)), CopyString);
}

static Variant HHVM_FUNCTION(imap_fetchstructure, const OptResource& imap_stream,
                             int64_t msg_number, int64_t options /* = 0 */) {
  if (options && ((options & ~FT_UID) != 0)) {
    raise_warning("invalid value for the options parameter");
    return false;
  }

  if (msg_number < 1) {
    return false;
  }

  auto obj = cast<ImapStream>(imap_stream);
  int msgindex;
  if (options & FT_UID) {
    /* This should be cached; if it causes an extra RTT to the
       IMAP server, then that's the price we pay for making sure
       we don't crash. */
    msgindex = mail_msgno(obj->m_stream, msg_number);
  } else {
    msgindex = msg_number;
  }

  if (!obj->checkMsgNumber(msgindex)) {
    return false;
  }

  BODY *body;

  mail_fetchstructure_full(obj->m_stream, msg_number, &body,
                           (options ? options : NIL));

  if (!body) {
    raise_warning("No body information available");
    return false;
  }

  return _php_imap_body(body, true);
}

static bool HHVM_FUNCTION(imap_gc, const OptResource& imap_stream,
                          int64_t caches) {
  if (caches && ((caches & ~(GC_TEXTS | GC_ELT | GC_ENV)) != 0)) {
    raise_warning("invalid value for the caches parameter");
    return false;
  }

  auto obj = cast<ImapStream>(imap_stream);
  mail_gc(obj->m_stream, caches);
  return true;
}

static Variant
HHVM_FUNCTION(imap_headerinfo, const OptResource& imap_stream, int64_t msg_number,
              int64_t fromlength /* = 0 */, int64_t subjectlength /* = 0 */,
              const String& /*defaulthost*/ /* = "" */) {
  auto obj = cast<ImapStream>(imap_stream);
  if (fromlength < 0 || fromlength > MAILTMPLEN) {
    Logger::Warning("From length has to be between 0 and %d", MAILTMPLEN);
    return false;
  }
  if (subjectlength < 0 || subjectlength > MAILTMPLEN) {
    Logger::Warning("Subject length has to be between 0 and %d", MAILTMPLEN);
    return false;
  }
  if (!obj->checkMsgNumber(msg_number)) {
    return false;
  }
  if (!mail_fetchstructure(obj->m_stream, msg_number, NIL)) {
    return false;
  }

  MESSAGECACHE *cache = mail_elt(obj->m_stream, msg_number);
  ENVELOPE *en = mail_fetchenvelope(obj->m_stream, msg_number);

  /* call a function to parse all the text, so that we can use the
     same function to parse text from other sources */
  Array props = _php_make_header_props(en);

  /* now run through properties that are only going to be returned
     from a server, not text headers */
  props.set(String("Recent"),   cache->recent ? (cache->seen ? "R": "N") : " ");
  props.set(String("Unseen"),   (cache->recent | cache->seen) ? " " : "U");
  props.set(String("Flagged"),  cache->flagged  ? "F" : " ");
  props.set(String("Answered"), cache->answered ? "A" : " ");
  props.set(String("Deleted"),  cache->deleted  ? "D" : " ");
  props.set(String("Draft"),    cache->draft    ? "X" : " ");

  char dummy[2000], fulladdress[MAILTMPLEN + 1];
  snprintf(dummy, sizeof(dummy), "%4ld", cache->msgno);
  props.set(String("Msgno"), String(dummy, CopyString));

  mail_date(dummy, cache);
  props.set(String("MailDate"), String(dummy, CopyString));

  snprintf(dummy, sizeof(dummy), "%ld", cache->rfc822_size);
  props.set(String("Size"), String(dummy, CopyString));

  props.set(String("udate"), (int64_t)mail_longdate(cache));

  if (en->from && fromlength) {
    fulladdress[0] = 0x00;
    mail_fetchfrom(fulladdress, obj->m_stream, msg_number, fromlength);
    props.set(String("fetchfrom"), String(fulladdress, CopyString));
  }
  if (en->subject && subjectlength) {
    fulladdress[0] = 0x00;
    mail_fetchsubject(fulladdress, obj->m_stream, msg_number, subjectlength);
    props.set(String("fetchsubject"), String(fulladdress, CopyString));
  }

  return ObjectData::FromArray(props.detach());
}

static Variant HHVM_FUNCTION(imap_header, const OptResource& imap_stream,
                             int64_t msg_number, int64_t fromlength /* = 0 */,
                             int64_t subjectlength /* = 0 */,
                             const String& defaulthost /* = "" */) {
  return HHVM_FN(imap_headerinfo)(imap_stream, msg_number,
                           fromlength, subjectlength, defaulthost);
}

static Variant HHVM_FUNCTION(imap_last_error, ) {
  if (IMAPG(errorstack) == NIL) {
    return false;
  }
  for (ERRORLIST *cur = IMAPG(errorstack); cur != NIL;
       cur = cur->next) {
    if (cur->next == NIL) {
      return String((const char *)cur->text.data, CopyString);
    }
  }
  return init_null();
}

static Variant HHVM_FUNCTION(imap_list, const OptResource& imap_stream,
                             const String& ref, const String& pattern) {
  auto obj = cast<ImapStream>(imap_stream);

  /* set flag for normal, old mailbox list */
  IMAPG(folderlist_style) = FLIST_ARRAY;

  IMAPG(folders) = IMAPG(folders_tail) = NIL;
  mail_list(obj->m_stream, (char*)ref.data(), (char*)pattern.data());
  if (IMAPG(folders) == NIL) {
    return false;
  }

  Array ret(Array::CreateVec());
  for (STRINGLIST *cur = IMAPG(folders); cur != NIL; cur = cur->next) {
    ret.append(String((const char *)cur->text.data, CopyString));
  }
  mail_free_stringlist(&IMAPG(folders));
  IMAPG(folders) = IMAPG(folders_tail) = NIL;
  return ret;
}

static Variant HHVM_FUNCTION(imap_listmailbox, const OptResource& imap_stream,
                             const String& ref, const String& pattern) {
  return HHVM_FN(imap_list)(imap_stream, ref, pattern);
}

static bool HHVM_FUNCTION(imap_mail_copy, const OptResource& imap_stream,
                          const String& msglist, const String& mailbox,
                          int64_t options /* = 0 */) {
  auto obj = cast<ImapStream>(imap_stream);
  if (mail_copy_full(obj->m_stream, (char *)msglist.data(),
                    (char *)mailbox.data(), (options ? options : NIL)) == T) {
    return true;
  } else {
    return false;
  }
}

static bool HHVM_FUNCTION(imap_mail_move, const OptResource& imap_stream,
                          const String& msglist, const String& mailbox,
                          int64_t options /* = 0 */) {
  auto obj = cast<ImapStream>(imap_stream);
  if (mail_copy_full(obj->m_stream, (char *)msglist.data(),
                     (char *)mailbox.data(),
                     (options ? (options | CP_MOVE) : CP_MOVE)) == T) {
    return true;
  } else {
    return false;
  }
}

static bool HHVM_FUNCTION(imap_mail, const String& to, const String& subject,
                          const String& message,
                          const String& additional_headers /* = "" */,
                          const String& cc /* = "" */,
                          const String& bcc /* = "" */,
                          const String& rpath /* = "" */) {
  if (to.empty()) {
    raise_warning("No to field in mail command");
  }

  if (subject.empty()) {
    raise_warning("No subject field in mail command");
  }

  if (message.empty()) {
    raise_warning("No message string in mail command");
  }

  if (RuntimeOption::SendmailPath.empty()) {
    return false;
  }

  FILE *sendmail = popen(RuntimeOption::SendmailPath.c_str(), "w");
  if (sendmail) {
    if (!rpath.empty()) {
      fprintf(sendmail, "From: %s\n", rpath.c_str());
    }

    fprintf(sendmail, "To: %s\n", to.c_str());

    if (!cc.empty()) {
      fprintf(sendmail, "Cc: %s\n", cc.c_str());
    }
    if (!bcc.empty()) {
      fprintf(sendmail, "Bcc: %s\n", bcc.c_str());
    }

    fprintf(sendmail, "Subject: %s\n", subject.c_str());

    if (!additional_headers.empty()) {
      fprintf(sendmail, "%s\n", additional_headers.c_str());
    }

    fprintf(sendmail, "\n%s\n", message.c_str());
    int ret = pclose(sendmail);
    return (!ret);
  } else {
    raise_warning("Could not execute mail delivery program");
    return false;
  }
}

static Variant HHVM_FUNCTION(imap_mailboxmsginfo, const OptResource& imap_stream) {
  auto obj = cast<ImapStream>(imap_stream);

  int64_t unreadmsg = 0, deletedmsg = 0, msize = 0;

  for (unsigned long i = 1; i <= obj->m_stream->nmsgs; i++) {
    MESSAGECACHE * cache = mail_elt (obj->m_stream, i);
    mail_fetchstructure (obj->m_stream, i, NIL);

    if (!cache->seen || cache->recent) {
      unreadmsg++;
    }

    if (cache->deleted) {
      deletedmsg++;
    }
    msize = msize + cache->rfc822_size;
  }

  char date[100];
  rfc822_date(date);

  DictInit props(8);
  props.set(LAZY_STATIC_STRING("Unread"), (int64_t)unreadmsg);
  props.set(LAZY_STATIC_STRING("Deleted"), (int64_t)deletedmsg);
  props.set(LAZY_STATIC_STRING("Nmsgs"), (int64_t)obj->m_stream->nmsgs);
  props.set(LAZY_STATIC_STRING("Size"), (int64_t)msize);
  props.set(LAZY_STATIC_STRING("Date"), String(date, CopyString));
  props.set(LAZY_STATIC_STRING("Driver"), String(obj->m_stream->dtb->name, CopyString));
  props.set(LAZY_STATIC_STRING("Mailbox"), String(obj->m_stream->mailbox, CopyString));
  props.set(LAZY_STATIC_STRING("Recent"), (int64_t)msize);
  return ObjectData::FromArray(props.create());
}

static Variant HHVM_FUNCTION(imap_msgno, const OptResource& imap_stream,
                             int64_t uid) {
  auto obj = cast<ImapStream>(imap_stream);
  return (int64_t)mail_msgno(obj->m_stream, uid);
}

static Variant HHVM_FUNCTION(imap_num_msg, const OptResource& imap_stream) {
  return (int64_t)cast<ImapStream>(imap_stream)->m_stream->nmsgs;
}

static Variant HHVM_FUNCTION(imap_num_recent, const OptResource& imap_stream) {
  auto obj = cast<ImapStream>(imap_stream);
  return (int64_t)obj->m_stream->recent;
}

static Variant HHVM_FUNCTION(imap_open, const String& mailbox,
                             const String& username, const String& password,
                             int64_t options /* = 0 */,
                             int64_t retries /* = 0 */) {
  String filename = mailbox;
  if (filename[0] != '{') {
    if (!FileUtil::checkPathAndWarn(filename, __FUNCTION__ + 2, 1)) {
      return init_null();
    }

    filename = File::TranslatePath(filename);
    if (filename.empty()) {
      return false;
    }
  }

  if (retries < 0) {
    Logger::Warning("Retries must be greater or equal to 0");
  } else {
    mail_parameters(NIL, SET_MAXLOGINTRIALS, (void *) retries);
  }

  IMAPG(user)     = std::string(username.data(), username.size());
  IMAPG(password) = std::string(password.data(), password.size());

  MAILSTREAM *stream = mail_open(NIL, (char*)filename.data(), options);
  if (stream == NIL) {
    Logger::Warning("Couldn't open stream %s", filename.data());
    IMAPG(user).clear();
    IMAPG(password).clear();
    return false;
  }

  return Variant(req::make<ImapStream>(
                 stream, (options & PHP_EXPUNGE) ? CL_EXPUNGE : NIL));
}

static bool HHVM_FUNCTION(imap_ping, const OptResource& imap_stream) {
  auto obj = cast<ImapStream>(imap_stream);
  return mail_ping(obj->m_stream);
}

static Variant HHVM_FUNCTION(imap_qprint, const String& str) {
  unsigned long newlength;

  char *decode = (char *)rfc822_qprint((unsigned char *) str.data(),
                                       str.length(), &newlength);
  if (decode == NULL) {
    return false;
  }

  String ret = String(decode, newlength, CopyString);
  fs_give((void**) &decode);
  return ret;
}

static bool HHVM_FUNCTION(imap_renamemailbox, const OptResource& imap_stream,
                          const String& old_mbox, const String& new_mbox) {
  auto obj = cast<ImapStream>(imap_stream);
  if (mail_rename(obj->m_stream, (char *)old_mbox.data(),
                  (char *)new_mbox.data()) == T) {
    return true;
  } else {
    return false;
  }
}

static bool HHVM_FUNCTION(imap_reopen, const OptResource& imap_stream,
                          const String& mailbox, int64_t options /* = 0 */,
                          int64_t retries /* = 0 */) {
  auto obj = cast<ImapStream>(imap_stream);
  long flags = NIL;
  long cl_flags = NIL;
  if (options) {
    flags = options;
    if (flags & PHP_EXPUNGE) {
      cl_flags = CL_EXPUNGE;
      flags ^= PHP_EXPUNGE;
    }
    obj->m_flag = cl_flags;
  }

  if (retries) {
    mail_parameters(NIL, SET_MAXLOGINTRIALS, (void *) retries);
  }

  MAILSTREAM *stream = mail_open(obj->m_stream, (char*)mailbox.data(), flags);
  if (stream == NIL) {
    raise_warning("Couldn't re-open stream");
    return false;
  }
  obj->m_stream = stream;
  return true;
}

static Variant HHVM_FUNCTION(imap_search, const OptResource& imap_stream,
                             const String& criteria, int64_t options /* = 0 */,
                             const String& charset /* = "" */) {
  auto obj = cast<ImapStream>(imap_stream);

  char *search_criteria = (char*)criteria.data();
  IMAPG(messages) = IMAPG(messages_tail) = NIL;

  SEARCHPGM *pgm = mail_criteria(search_criteria);

  mail_search_full(obj->m_stream,
                   (charset.empty() ? NIL : (char*)charset.data()),
                   pgm,
                   options);

  if (pgm && !(options & SE_FREE)) {
    mail_free_searchpgm(&pgm);
  }

  if (IMAPG(messages) == NIL) {
    return false;
  }

  Array ret(Array::CreateVec());

  MESSAGELIST *cur = IMAPG(messages);
  while (cur != NIL) {
    ret.append((int64_t)cur->msgid);
    cur = cur->next;
  }
  mail_free_messagelist(&IMAPG(messages), &IMAPG(messages_tail));
  return ret;
}

static bool HHVM_FUNCTION(imap_setflag_full, const OptResource& imap_stream,
                          const String& sequence, const String& flag,
                         int64_t options /* = 0 */) {
  auto obj = cast<ImapStream>(imap_stream);
  mail_setflag_full(obj->m_stream, (char*)sequence.data(), (char*)flag.data(),
                    (options ? options : NIL));
  return true;
}

static Variant HHVM_FUNCTION(imap_status, const OptResource& imap_stream,
                             const String& mailbox, int64_t options /* = 0 */) {
  auto obj = cast<ImapStream>(imap_stream);

  if (!mail_status(obj->m_stream, (char *)mailbox.data(), options)) {
    return false;
  }

  DictInit props(6);
  props.set(LAZY_STATIC_STRING("flags"), (int64_t)IMAPG(status_flags));
  if (IMAPG(status_flags) & SA_MESSAGES) {
    props.set(LAZY_STATIC_STRING("messages"), (int64_t)IMAPG(status_messages));
  }
  if (IMAPG(status_flags) & SA_RECENT) {
    props.set(LAZY_STATIC_STRING("recent"), (int64_t)IMAPG(status_recent));
  }
  if (IMAPG(status_flags) & SA_UNSEEN) {
    props.set(LAZY_STATIC_STRING("unseen"), (int64_t)IMAPG(status_unseen));
  }
  if (IMAPG(status_flags) & SA_UIDNEXT) {
    props.set(LAZY_STATIC_STRING("uidnext"), (int64_t)IMAPG(status_uidnext));
  }
  if (IMAPG(status_flags) & SA_UIDVALIDITY) {
    props.set(LAZY_STATIC_STRING("uidvalidity"), (int64_t)IMAPG(status_uidvalidity));
  }
  return ObjectData::FromArray(props.create());
}

static bool HHVM_FUNCTION(imap_subscribe, const OptResource& imap_stream,
                          const String& mailbox) {
  auto obj = cast<ImapStream>(imap_stream);
  if (mail_subscribe(obj->m_stream, (char *)mailbox.data()) == T) {
    return true;
  } else {
    return false;
  }
}

static Variant HHVM_FUNCTION(imap_timeout, int64_t timeout_type,
                             int64_t timeout /* = -1 */) {
  int actual_type;
  if (timeout == -1) {
    switch (timeout_type) {
      case 1:
        actual_type = GET_OPENTIMEOUT;
        break;
      case 2:
        actual_type = GET_READTIMEOUT;
        break;
      case 3:
        actual_type = GET_WRITETIMEOUT;
        break;
      case 4:
        actual_type = GET_CLOSETIMEOUT;
        break;
      default:
        return false;
        break;
    }
    return (int64_t)mail_parameters(NIL, actual_type, NIL);
  } else if (timeout >= 0) {
    switch (timeout_type) {
      case 1:
        actual_type = SET_OPENTIMEOUT;
        break;
      case 2:
        actual_type = SET_READTIMEOUT;
        break;
      case 3:
        actual_type = SET_WRITETIMEOUT;
        break;
      case 4:
        actual_type = SET_CLOSETIMEOUT;
        break;
      default:
        return false;
        break;
    }
    timeout = (int64_t)mail_parameters(NIL, actual_type, (void *) timeout);
    return true;
  }
  return false;
}

static Variant HHVM_FUNCTION(imap_uid, const OptResource& imap_stream,
                             int64_t msg_number) {
  auto obj = cast<ImapStream>(imap_stream);
  if (!obj->checkMsgNumber(msg_number)) {
    return false;
  }
  return (int64_t)mail_uid(obj->m_stream, msg_number);
}

static bool HHVM_FUNCTION(imap_undelete, const OptResource& imap_stream,
                          const String& msg_number, int64_t flags /* = 0 */) {
  auto obj = cast<ImapStream>(imap_stream);
  mail_clearflag_full(obj->m_stream, (char *)msg_number.data(),
                      "\\DELETED", (flags ? flags : NIL));
  return true;
}

static bool HHVM_FUNCTION(imap_unsubscribe, const OptResource& imap_stream,
                          const String& mailbox) {
  auto obj = cast<ImapStream>(imap_stream);
  if (mail_unsubscribe(obj->m_stream, (char *)mailbox.data()) == T) {
    return true;
  } else {
    return false;
  }
}

static Variant HHVM_FUNCTION(imap_utf8, const String& mime_encoded_text) {
  SIZEDTEXT src, dest;
  src.data  = NULL;
  src.size  = 0;
  dest.data = NULL;
  dest.size = 0;

  cpytxt(&src, (char *)mime_encoded_text.data(), mime_encoded_text.length());
  utf8_mime2text(&src, &dest, U8T_DECOMPOSE);

  if (src.data && src.data != dest.data) {
    free(src.data);
  }
  return String(reinterpret_cast<char*>(dest.data), dest.size, AttachString);
}

///////////////////////////////////////////////////////////////////////////////

static struct imapExtension final : Extension {
  imapExtension() : Extension("imap", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

  void moduleInit() override {
    mail_link(&unixdriver);   /* link in the unix driver */
    mail_link(&mhdriver);     /* link in the mh driver */
    /* According to c-client docs (internal.txt) this shouldn't be used. */
    /* mail_link(&mxdriver); */
    mail_link(&mmdfdriver);   /* link in the mmdf driver */
    mail_link(&newsdriver);   /* link in the news driver */
    mail_link(&philedriver);  /* link in the phile driver */

    mail_link(&imapdriver);   /* link in the imap driver */
    mail_link(&nntpdriver);   /* link in the nntp driver */
    mail_link(&pop3driver);   /* link in the pop3 driver */
    mail_link(&mbxdriver);    /* link in the mbx driver */
    mail_link(&tenexdriver);  /* link in the tenex driver */
    mail_link(&mtxdriver);    /* link in the mtx driver */
    mail_link(&dummydriver);  /* link in the dummy driver */

    auth_link(&auth_log);     /* link in the log authenticator */
    auth_link(&auth_md5);     /* link in the cram-md5 authenticator */

#ifndef SKIP_IMAP_GSS
    auth_link(&auth_gss);     /* link in the gss authenticator */
#endif

    auth_link(&auth_pla);     /* link in the plain authenticator */

#ifndef SKIP_IMAP_SSL
    ssl_onceonlyinit();
#endif

    /* plug in our gets */
    mail_parameters(NIL, SET_GETS, (void *) NIL);

    /* set default timeout values */
    void *timeout = reinterpret_cast<void *>(
      RequestInfo::s_requestInfo.getNoCheck()->
        m_reqInjectionData.getSocketDefaultTimeout());

    mail_parameters(NIL, SET_OPENTIMEOUT,  timeout);
    mail_parameters(NIL, SET_READTIMEOUT,  timeout);
    mail_parameters(NIL, SET_WRITETIMEOUT, timeout);
    mail_parameters(NIL, SET_CLOSETIMEOUT, timeout);

    // FIXME: Use a consistent constant/enum
    // rather than repeating hard-coded values
    HHVM_RC_INT(IMAP_OPENTIMEOUT, 1);
    HHVM_RC_INT(IMAP_READTIMEOUT, 2);
    HHVM_RC_INT(IMAP_WRITETIMEOUT, 3);
    HHVM_RC_INT(IMAP_CLOSETIMEOUT, 4);

    HHVM_RC_INT_SAME(NIL);

    HHVM_RC_INT_SAME(OP_DEBUG);
    HHVM_RC_INT_SAME(OP_READONLY);
    HHVM_RC_INT_SAME(OP_ANONYMOUS);
    HHVM_RC_INT_SAME(OP_SHORTCACHE);
    HHVM_RC_INT_SAME(OP_SILENT);
    HHVM_RC_INT_SAME(OP_PROTOTYPE);
    HHVM_RC_INT_SAME(OP_HALFOPEN);
    HHVM_RC_INT_SAME(OP_EXPUNGE);
    HHVM_RC_INT_SAME(OP_SECURE);

    HHVM_RC_INT_SAME(LATT_NOINFERIORS);
    HHVM_RC_INT_SAME(LATT_NOSELECT);
    HHVM_RC_INT_SAME(LATT_MARKED);
    HHVM_RC_INT_SAME(LATT_UNMARKED);
#ifdef LATT_REFERRAL
    HHVM_RC_INT_SAME(LATT_REFERRAL);
#endif
#ifdef LATT_HASCHILDREN
    HHVM_RC_INT_SAME(LATT_HASCHILDREN);
#endif
#ifdef LATT_HASNOCHILDREN
    HHVM_RC_INT_SAME(LATT_HASNOCHILDREN);
#endif

    HHVM_RC_INT_SAME(SE_UID);
    HHVM_RC_INT_SAME(SE_FREE);
    HHVM_RC_INT_SAME(SE_NOPREFETCH);

    HHVM_RC_INT_SAME(SO_FREE);
    HHVM_RC_INT_SAME(SO_NOSERVER);

    HHVM_RC_INT_SAME(SA_MESSAGES);
    HHVM_RC_INT_SAME(SA_RECENT);
    HHVM_RC_INT_SAME(SA_UNSEEN);
    HHVM_RC_INT_SAME(SA_UIDNEXT);
    HHVM_RC_INT_SAME(SA_UIDVALIDITY);
    HHVM_RC_INT(SA_ALL, SA_MESSAGES | SA_RECENT | SA_UNSEEN |
                        SA_UIDNEXT | SA_UIDVALIDITY);

    HHVM_RC_INT(CL_EXPUNGE, PHP_EXPUNGE);

    HHVM_RC_INT_SAME(FT_UID);
    HHVM_RC_INT_SAME(FT_PEEK);
    HHVM_RC_INT_SAME(FT_NOT);
    HHVM_RC_INT_SAME(FT_INTERNAL);
    HHVM_RC_INT_SAME(FT_PREFETCHTEXT);

    HHVM_RC_INT_SAME(ST_UID);
    HHVM_RC_INT_SAME(ST_SILENT);
    HHVM_RC_INT_SAME(ST_SET);

    HHVM_RC_INT_SAME(CP_UID);
    HHVM_RC_INT_SAME(CP_MOVE);

    HHVM_RC_INT_SAME(SORTDATE);
    HHVM_RC_INT_SAME(SORTARRIVAL);
    HHVM_RC_INT_SAME(SORTFROM);
    HHVM_RC_INT_SAME(SORTSUBJECT);
    HHVM_RC_INT_SAME(SORTTO);
    HHVM_RC_INT_SAME(SORTCC);
    HHVM_RC_INT_SAME(SORTSIZE);

    HHVM_RC_INT_SAME(TYPETEXT);
    HHVM_RC_INT_SAME(TYPEMULTIPART);
    HHVM_RC_INT_SAME(TYPEMESSAGE);
    HHVM_RC_INT_SAME(TYPEAPPLICATION);
    HHVM_RC_INT_SAME(TYPEAUDIO);
    HHVM_RC_INT_SAME(TYPEIMAGE);
    HHVM_RC_INT_SAME(TYPEVIDEO);
    HHVM_RC_INT_SAME(TYPEMODEL);
    HHVM_RC_INT_SAME(TYPEOTHER);

    HHVM_RC_INT_SAME(ENC7BIT);
    HHVM_RC_INT_SAME(ENC8BIT);
    HHVM_RC_INT_SAME(ENCBINARY);
    HHVM_RC_INT_SAME(ENCBASE64);
    HHVM_RC_INT_SAME(ENCQUOTEDPRINTABLE);
    HHVM_RC_INT_SAME(ENCOTHER);

    HHVM_FE(imap_8bit);
    HHVM_FE(imap_alerts);
    HHVM_FE(imap_base64);
    HHVM_FE(imap_binary);
    HHVM_FE(imap_body);
    HHVM_FE(imap_bodystruct);
    HHVM_FE(imap_check);
    HHVM_FE(imap_clearflag_full);
    HHVM_FE(imap_close);
    HHVM_FE(imap_createmailbox);
    HHVM_FE(imap_delete);
    HHVM_FE(imap_deletemailbox);
    HHVM_FE(imap_errors);
    HHVM_FE(imap_expunge);
    HHVM_FE(imap_fetch_overview);
    HHVM_FE(imap_fetchbody);
    HHVM_FE(imap_fetchheader);
    HHVM_FE(imap_fetchstructure);
    HHVM_FE(imap_gc);
    HHVM_FE(imap_header);
    HHVM_FE(imap_headerinfo);
    HHVM_FE(imap_last_error);
    HHVM_FE(imap_list);
    HHVM_FE(imap_listmailbox);
    HHVM_FE(imap_mail_copy);
    HHVM_FE(imap_mail_move);
    HHVM_FE(imap_mail);
    HHVM_FE(imap_mailboxmsginfo);
    HHVM_FE(imap_msgno);
    HHVM_FE(imap_num_msg);
    HHVM_FE(imap_num_recent);
    HHVM_FE(imap_open);
    HHVM_FE(imap_ping);
    HHVM_FE(imap_qprint);
    HHVM_FE(imap_renamemailbox);
    HHVM_FE(imap_reopen);
    HHVM_FE(imap_search);
    HHVM_FE(imap_setflag_full);
    HHVM_FE(imap_status);
    HHVM_FE(imap_subscribe);
    HHVM_FE(imap_timeout);
    HHVM_FE(imap_uid);
    HHVM_FE(imap_undelete);
    HHVM_FE(imap_unsubscribe);
    HHVM_FE(imap_utf8);
  }

} s_imap_extension;

}
