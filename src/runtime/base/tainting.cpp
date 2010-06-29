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

#include <runtime/base/tainting.h>
#include <runtime/base/util/string_buffer.h>

#ifdef TAINTED

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool propagate_tainting_aux(CStrRef orig, String& dest) {
  if(orig.isTainted()){
    dest.taint();
    dest.setPlaceTainted(orig.getPlaceTainted().name,
                         orig.getPlaceTainted().line);
    return true;
  }
  return false;
}
void propagate_tainting1(CStrRef orig, String& dest) {
  if( propagate_tainting_aux(orig, dest) ) { }
  else { dest.untaint(); }
}
void propagate_tainting2(CStrRef orig1, CStrRef orig2,
                                  String& dest) {
  if( propagate_tainting_aux(orig1, dest) ) { }
  else if( propagate_tainting_aux(orig2, dest) ) { }
  else { dest.untaint(); }
}
void propagate_tainting3(CStrRef orig1, CStrRef orig2,
                                  CStrRef orig3,
                                  String& dest) {
  if( propagate_tainting_aux(orig1, dest) ) { }
  else if( propagate_tainting_aux(orig2, dest) ) { }
  else if( propagate_tainting_aux(orig3, dest) ) { }
  else { dest.untaint(); }
}
void propagate_tainting4(CStrRef orig1, CStrRef orig2,
                                  CStrRef orig3, CStrRef orig4,
                                  String& dest) {
  if( propagate_tainting_aux(orig1, dest) ) { }
  else if( propagate_tainting_aux(orig2, dest) ) { }
  else if( propagate_tainting_aux(orig3, dest) ) { }
  else if( propagate_tainting_aux(orig4, dest) ) { }
  else { dest.untaint(); }
}
void propagate_tainting5(CStrRef orig1, CStrRef orig2,
                                  CStrRef orig3, CStrRef orig4,
                                  CStrRef orig5,
                                  String& dest) {
  if( propagate_tainting_aux(orig1, dest) ) { }
  else if( propagate_tainting_aux(orig2, dest) ) { }
  else if( propagate_tainting_aux(orig3, dest) ) { }
  else if( propagate_tainting_aux(orig4, dest) ) { }
  else if( propagate_tainting_aux(orig5, dest) ) { }
  else { dest.untaint(); }
}
void propagate_tainting6(CStrRef orig1, CStrRef orig2,
                                  CStrRef orig3, CStrRef orig4,
                                  CStrRef orig5, CStrRef orig6,
                                  String& dest) {
  if( propagate_tainting_aux(orig1, dest) ) { }
  else if( propagate_tainting_aux(orig2, dest) ) { }
  else if( propagate_tainting_aux(orig3, dest) ) { }
  else if( propagate_tainting_aux(orig4, dest) ) { }
  else if( propagate_tainting_aux(orig5, dest) ) { }
  else if( propagate_tainting_aux(orig6, dest) ) { }
  else { dest.untaint(); }
}

bool propagate_tainting2_buf_aux1(CStrRef orig, StringBuffer& dest) {
  if(orig.isTainted()){
    dest.taint();
    dest.setPlaceTainted(orig.getPlaceTainted().name,
                         orig.getPlaceTainted().line);
    return true;
  }
  return false;
}
bool propagate_tainting2_buf_aux2(StringBuffer const &orig,
                                  StringBuffer& dest) {
  if(orig.isTainted()){
    dest.taint();
    dest.setPlaceTainted(orig.getPlaceTainted().name,
                         orig.getPlaceTainted().line);
    return true;
  }
  return false;
}
void propagate_tainting2_buf(CStrRef orig1, StringBuffer const &orig2,
                                 StringBuffer& dest){
  if( propagate_tainting2_buf_aux1(orig1, dest) ) { }
  else if( propagate_tainting2_buf_aux2(orig2, dest) ) { }
  else { dest.untaint(); }
}

bool propagate_tainting1_buf_aux(StringBuffer const &orig, String& dest) {
  if(orig.isTainted()){
    dest.taint();
    dest.setPlaceTainted(orig.getPlaceTainted().name,
                         orig.getPlaceTainted().line);
    return true;
  }
  return false;
}
void propagate_tainting1_buf(const StringBuffer &orig, String& dest){
  if( propagate_tainting1_buf_aux(orig, dest) ) { }
  else { dest.untaint(); }
}

}

#endif
