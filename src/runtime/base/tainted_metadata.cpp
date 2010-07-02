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

#ifdef TAINTED

#include <runtime/base/tainted_metadata.h>
#include <runtime/base/frame_injection.h>

namespace HPHP{
  void TaintedMetadata::setTaintedOriginal(const String* str){
    ASSERT(str != NULL);
    tainted_original = *str;
  }
  const String* TaintedMetadata::getTaintedOriginal() const {
    return &tainted_original;
  }
  std::string TaintedMetadata::stringOfTaintedOriginal() const {
    return tainted_original.data();
  }

  void TaintedMetadata::setTaintedPlace(){
    tainted_place = FrameInjection::GetBacktrace(false, true);
    // false, true allows to have as much detail as possible
  }
  void TaintedMetadata::setTaintedPlace(const Variant place){
    tainted_place = place;
  }
  const Variant* TaintedMetadata::getTaintedPlace() const {
    return &tainted_place;
  }
  std::string TaintedMetadata::stringOfTaintedPlace() const {
    if(tainted_place.isArray()){
      return ExtendedLogger::StringOfStackTrace(tainted_place.toArray());
    }
    else if (tainted_place.isString()) {
      return ((std::string)"   ") + tainted_place.toString().data() + "\n";
    }
    else {
      return "   No information on the place where the string was tainted\n";
    }
  }

  void TaintedMetadata::addTaintedChanged(){
    tainted_changed.append(FrameInjection::GetBacktrace(false,true));
    // false, true allows to have as much detail as possible
  }
  void TaintedMetadata::addTaintedChanged(Array* st){
    ASSERT(st != NULL);
    tainted_changed.append(*st);
  }
  void TaintedMetadata::setTaintedChanged(const Array* st){
    ASSERT(st != NULL);
    tainted_changed = *st;
  }
  const Array* TaintedMetadata::getTaintedChanged() const {
    return &tainted_changed;
  }
  std::string TaintedMetadata::stringOfTaintedChanged() const {
    if(tainted_changed.size() != 0){
      std::ostringstream res;
      for (ArrayIter iter(tainted_changed); !iter.end(); iter.next()){
        res << ExtendedLogger::StringOfStackTrace(iter.second())
            << "    ---\n";
      }
      std::string buffer(res.str());
      return buffer;
    }
    else{
      return "    <never>\n";
    }
  }

  std::string TaintedMetadata::stringOfTaintedMetadata() const {
    std::ostringstream res;
    res << "Echoing a tainted string:\n"
        << "  the originally tainted string was : '"
        << stringOfTaintedOriginal() << "'\n"
        << "  it was tainted at :\n"
        << stringOfTaintedPlace()
        << "  it was successively modified at:\n"
        << stringOfTaintedChanged()
        << "  it was echoed at :\n"
        << ExtendedLogger::StringOfStackTrace(
               FrameInjection::GetBacktrace(false,true));
    // false, true allows to have as much detail as possible
    std::string buffer(res.str());
    return buffer;
  }
}

#endif
