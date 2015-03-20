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

#ifndef incl_HPHP_BITREF_SURVEY_
#define incl_HPHP_BITREF_SURVEY_

#include "hphp/runtime/base/countable.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct BitrefSurvey {

public:
  void cow_check_occurred(RefCount refcount, bool bitref);
  void survey_request_end();

private:
  uint64_t check_count;
  uint64_t refcounting_copy_count;
  uint64_t bitref_copy_count;

};

/*
 * Log when a copy-on-write check takes place. 
 */
void cow_check_occurred(RefCount refcount, bool bitref);

/*
 * Indicate that the request has ended and results should be printed
 */
void survey_request_end();

BitrefSurvey *survey();

extern BitrefSurvey *g_survey;

///////////////////////////////////////////////////////////////////////////////

}

#endif