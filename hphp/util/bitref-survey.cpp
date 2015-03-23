/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/bitref-survey.h"
#include "hphp/runtime/base/countable.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(bitref);
BitrefSurvey *g_survey;

inline BitrefSurvey *survey() {
  if (!g_survey) {
    g_survey = new BitrefSurvey();
  }
  return g_survey;
}

/*
 * Log when a copy-on-write check takes place. 
 */
void cow_check_occurred(RefCount refcount, bool bitref) {
  survey()->cow_check_occurred(refcount, bitref);
}

void survey_request_end() {
  survey()->survey_request_end();
}

void BitrefSurvey::cow_check_occurred(RefCount refcount, bool bitref) {
  check_count++;
  // TODO counts non-counted objects
  //FTRACE(1, "rc={}, bitref={}\n", refcount, bitref);
  if (refcount > 1) {
    assert(bitref);
    //'necessary' copy
    refcounting_copy_count++;
  }
  if (refcount == 1 && bitref) {
    //'unnecessary' copy
    bitref_copy_count++;
  }
}

void BitrefSurvey::survey_request_end() {
  FTRACE(1, "final cow checks: {}\n", check_count);
  FTRACE(1, "necessary copies: {}\n", refcounting_copy_count);
  FTRACE(1, "unnecessary copies: {}\n", bitref_copy_count);
  FTRACE(1, "copy avoided: {}\n",
      check_count - refcounting_copy_count - bitref_copy_count);
}

///////////////////////////////////////////////////////////////////////////////


}