#ifndef incl_HPHP_PARSE_TIME_FATAL_EXCEPTION_H
#define incl_HPHP_PARSE_TIME_FATAL_EXCEPTION_H

#include "hphp/util/exception.h"

namespace HPHP {
/////////////////////////////////////////////////////////////////////////////

class ParseTimeFatalException : public Exception {
public:
  ParseTimeFatalException(const std::string& file, int line,
                          const char* msg, ...) ATTRIBUTE_PRINTF(4,5)
  : m_file(file), m_line(line) {
    va_list ap; va_start(ap, msg); format(msg, ap); va_end(ap);
  }

  EXCEPTION_COMMON_IMPL(ParseTimeFatalException);

  void setParseFatal(bool b = true) { m_parseFatal = b; }

  std::string m_file;
  int m_line;
  bool m_parseFatal;
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP
#endif // incl_HPHP_PARSE_TIME_FATAL_EXCEPTION_H
