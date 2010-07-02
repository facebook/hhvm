/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_TAINTED_METADATA_H__
#define __HPHP_TAINTED_METADATA_H__

#ifdef TAINTED

#include <runtime/base/types.h>
#include <runtime/base/util/extended_logger.h>
#include <runtime/base/frame_injection.h>
#include <runtime/base/complex_types.h>

namespace HPHP {

/* This file provides a data structure for storing all the metadata
 * related to the tainting of a string. For a tainted string, we store:
 * - what the string was when it got tainted (tainted_original). This is
 *   useful when the echoed string is really big. For instance, if I write
 *   $foo='bad'; fb_taint($foo); echo('very long string'.$foo.'other str');
 *   I want to know which part of that echoed string is dangerous.
 * - the stack trace of where it was tainted (tainted_place when an Array)
 *   or a message indicating that it was tainted directly in the HPHP code
 *   (tainted_place when a String).
 * - all the stack traces of where it was modified (i.e., concatenated)
 * We also provide the stack trace of the place where the string was used
 * (usually echoed), but this information is calculated and output directly,
 * thus it is never stored.
 *
 * Such metadata is present in a string_data and in a string_buffer.
 * A type_string contains a string_data which contains the metadata.
 */

/* Here is a problem we encountered implementing TaintedMetadata:
 * TaintedMetadata uses stack traces which are implemented as Arrays;
 * Arrays use String which in turn use StringData, and StringData
 * has to contain a TaintedMetadata. This makes a circular reference.
 * We broke this circle between StringData and TaintedMetadata by
 * putting a TaintedMetadata* instead of a TaintedMetadata in StringData.
 * This problem is the very reason why we need to declare the class
 * TaintedMetadata at the beginning of files string_data.h and
 * string_buffer.h
 */

class TaintedMetadata {

 public:
  TaintedMetadata() : tainted_changed(Array::Create()) {
  }
  ~TaintedMetadata(){
  // tainted_place_st and tainted_place_msg don't need to be freed
  }

  std::string stringOfTaintedMetadata() const;

  void setTaintedOriginal(const String* str);
  const String* getTaintedOriginal() const;

  // sets the current stacktrace as the tainted place
  void setTaintedPlace();
  void setTaintedPlace(const Variant place);
  const Variant*  getTaintedPlace() const;

  void addTaintedChanged();
  void addTaintedChanged(Array* st);
  void setTaintedChanged(const Array* st);
  const Array* getTaintedChanged() const;

 private:
  std::string stringOfTaintedOriginal() const;
  std::string stringOfTaintedPlace() const;
  std::string stringOfTaintedChanged() const;

 private:
  String tainted_original;
  // tainted_place is always either a String or an Array:
  // if the string was tainted in the PHP code, it is an Array and it
  // contains the stack trace of where it was tainted; if the string was
  // tainted outside the PHP code, for example when generating the _GET
  // array in the hphp server code, tainted_place_msg explains it. */
  Variant tainted_place;
  Array tainted_changed;
};

}

#endif

#endif
