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

#include "ext_icu.h"
#include <unicode/translit.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Need to have a valid installation of the transliteration data in /lib64.
// Initialization will be taken care of by ext_array which also uses icu.

class TransliteratorWrapper {
public:
  TransliteratorWrapper() {
    UnicodeString basicID("Any-Latin ; NFKD; [:nonspacing mark:] Remove");
    UnicodeString basicIDAccent("Any-Latin ; NFKC");
    UErrorCode status = U_ZERO_ERROR;
    m_tl = Transliterator::createInstance(basicID, UTRANS_FORWARD, status);
    // Note that if the first createInstance fails, the status will cause the
    // second createInstance to also fail.
    m_tl_accent =
      Transliterator::createInstance(basicIDAccent, UTRANS_FORWARD, status);

    if (U_FAILURE(status)) {
      raise_warning(u_errorName(status));
      //m_tl should be NULL if createInstance fails but better safe than sorry.
      m_tl = NULL;
      m_tl_accent = NULL;
    }
  }

  void transliterate(UnicodeString& u_str) {
    if (m_tl) {
      m_tl->transliterate(u_str);
    } else {
      raise_warning("Transliterator not initialized.");
    }
  }

  void transliterate_with_accents(UnicodeString& u_str) {
    if (m_tl_accent) {
      m_tl_accent->transliterate(u_str);
    } else {
      raise_warning("Transliterator not initialized.");
    }
  }

private:
  Transliterator* m_tl;
  Transliterator* m_tl_accent;
};

IMPLEMENT_THREAD_LOCAL(TransliteratorWrapper, s_transliterator);

String f_icu_transliterate(CStrRef str, bool remove_accents) {
  UnicodeString u_str = UnicodeString::fromUTF8(str.data());
  if (remove_accents) {
    s_transliterator->transliterate(u_str);
  } else {
    s_transliterator->transliterate_with_accents(u_str);
  }

  // Convert the UnicodeString back into a UTF8 String.
  int32_t capacity = u_str.countChar32() * sizeof(UChar) + 1;
  char* out = (char *)malloc(capacity);
  CheckedArrayByteSink bs(out, capacity);
  u_str.toUTF8(bs);

  return String(out, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
}
