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

#include <runtime/base/propagate_tainting.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/tainted_metadata.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool propagate_tainting_aux(CStrRef orig, String& dest, bool do_propag_meta) {
  // do_propag_meta states if we should propagate the metadata; the returned
  // bool is the do_propag_meta for the next step
  bitstring b = orig.getTaint();
  // this test is to avoid deleting the dest's information
  // whenever it's exactly the same as the orig
  if(!(orig.get() == dest.get())){
    dest.setTaint(b);
  }
  if(do_propag_meta && is_tainting_metadata(b)){
    dest.getTaintedMetadata()->setTaintedOriginal(
        orig.getTaintedMetadata()->getTaintedOriginal());
    dest.getTaintedMetadata()->setTaintedPlace(
        *(orig.getTaintedMetadata()->getTaintedPlace()));
    dest.getTaintedMetadata()->setTaintedChanged(
        orig.getTaintedMetadata()->getTaintedChanged());
    dest.getTaintedMetadata()->addTaintedChanged();
    return false;
  }
  return do_propag_meta;
}
void propagate_tainting1(CStrRef orig, String& dest) {
  bool do_propag_meta = true;
  do_propag_meta = propagate_tainting_aux(orig, dest, do_propag_meta);
}
void propagate_tainting2(CStrRef orig1, CStrRef orig2,
                                  String& dest) {
  bool do_propag_meta = true;
  do_propag_meta = propagate_tainting_aux(orig1, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig2, dest, do_propag_meta);
}
void propagate_tainting3(CStrRef orig1, CStrRef orig2,
                                  CStrRef orig3,
                                  String& dest) {
  bool do_propag_meta = true;
  do_propag_meta = propagate_tainting_aux(orig1, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig2, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig3, dest, do_propag_meta);

}
void propagate_tainting4(CStrRef orig1, CStrRef orig2,
                                  CStrRef orig3, CStrRef orig4,
                                  String& dest) {
  bool do_propag_meta = true;
  do_propag_meta = propagate_tainting_aux(orig1, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig2, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig3, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig4, dest, do_propag_meta);
}
void propagate_tainting5(CStrRef orig1, CStrRef orig2,
                                  CStrRef orig3, CStrRef orig4,
                                  CStrRef orig5,
                                  String& dest) {
  bool do_propag_meta = true;
  do_propag_meta = propagate_tainting_aux(orig1, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig2, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig3, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig4, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig5, dest, do_propag_meta);

}
void propagate_tainting6(CStrRef orig1, CStrRef orig2,
                                  CStrRef orig3, CStrRef orig4,
                                  CStrRef orig5, CStrRef orig6,
                                  String& dest) {
  bool do_propag_meta = true;
  do_propag_meta = propagate_tainting_aux(orig1, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig2, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig3, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig4, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig5, dest, do_propag_meta);
  do_propag_meta = propagate_tainting_aux(orig6, dest, do_propag_meta);
}

bool propagate_tainting2_buf_aux1(CStrRef orig, StringBuffer& dest,
                                  bool do_propag_meta) {
  bitstring b = orig.getTaint();
  dest.setTaint(b);
  if(do_propag_meta && is_tainting_metadata(b)){
    dest.getTaintedMetadata()->setTaintedOriginal(
        orig.getTaintedMetadata()->getTaintedOriginal());
    dest.getTaintedMetadata()->setTaintedPlace(
        *(orig.getTaintedMetadata()->getTaintedPlace()));
    dest.getTaintedMetadata()->setTaintedChanged(
        orig.getTaintedMetadata()->getTaintedChanged());
    dest.getTaintedMetadata()->addTaintedChanged();
    return false;
  }
  return do_propag_meta;
}
bool propagate_tainting2_buf_aux2(StringBuffer const &orig,
                                  StringBuffer& dest, bool do_propag_meta) {
  bitstring b = orig.getTaint();
  dest.setTaint(b);
  if(do_propag_meta && is_tainting_metadata(b)){
    dest.getTaintedMetadata()->setTaintedOriginal(
        orig.getTaintedMetadata()->getTaintedOriginal());
    dest.getTaintedMetadata()->setTaintedPlace(
        *(orig.getTaintedMetadata()->getTaintedPlace()));
    dest.getTaintedMetadata()->setTaintedChanged(
        orig.getTaintedMetadata()->getTaintedChanged());
    dest.getTaintedMetadata()->addTaintedChanged();
    return false;
  }
  return do_propag_meta;
}
void propagate_tainting2_buf(CStrRef orig1, StringBuffer const &orig2,
                                 StringBuffer& dest){
  bool do_propag_meta = true;
  do_propag_meta = propagate_tainting2_buf_aux1(orig1, dest, do_propag_meta);
  do_propag_meta = propagate_tainting2_buf_aux2(orig2, dest, do_propag_meta);
}

bool propagate_tainting1_buf_aux(StringBuffer const &orig, String& dest,
    bool do_propag_meta) {
  bitstring b = orig.getTaint();
  dest.setTaint(b);
  if(do_propag_meta && is_tainting_metadata(b)){
    dest.getTaintedMetadata()->setTaintedOriginal(
        orig.getTaintedMetadata()->getTaintedOriginal());
    dest.getTaintedMetadata()->setTaintedPlace(
        *(orig.getTaintedMetadata()->getTaintedPlace()));
    dest.getTaintedMetadata()->setTaintedChanged(
        orig.getTaintedMetadata()->getTaintedChanged());
    dest.getTaintedMetadata()->addTaintedChanged();
    return false;
  }
  return do_propag_meta;
}
void propagate_tainting1_buf(const StringBuffer &orig, String& dest){
  bool do_propag_meta = true;
  do_propag_meta = propagate_tainting1_buf_aux(orig, dest, do_propag_meta);
}
void propagate_tainting1_bufbuf(const StringBuffer &orig, StringBuffer& dest){
  bool do_propag_meta = true;
  do_propag_meta = propagate_tainting2_buf_aux2(orig, dest, do_propag_meta);
}

}

#endif
