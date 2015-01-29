#ifdef ENABLE_THREAD_SAFE_SETLOCALE
#ifndef incl_HPHP_SETLOCALE_H_
#define incl_HPHP_SETLOCALE_H_

#include <vector>
#include <string>
#include <locale.h>
#include "hphp/util/thread-local.h"

namespace HPHP {

class ThreadSafeLocaleHandler {
private:
  typedef struct {
    int category;
    int category_mask;
    std::string category_str;
    std::string locale_str;
  } CategoryAndLocaleMap;

public:
  ThreadSafeLocaleHandler();
  ~ThreadSafeLocaleHandler();
  void reset();
  const char* actuallySetLocale(int category, const char* locale);

private:
  void generate_LC_ALL_String();

  std::vector<CategoryAndLocaleMap> m_category_locale_map;
  locale_t m_locale;
};

extern DECLARE_THREAD_LOCAL(ThreadSafeLocaleHandler, g_thread_safe_locale_handler);

}

#endif // incl_HPHP_SETLOCALE_H_
#endif // ENABLE_THREAD_SAFE_SETLOCALE
