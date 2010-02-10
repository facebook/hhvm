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

#ifndef __GENERATED_CLS_ARRAYITERATOR_H__
#define __GENERATED_CLS_ARRAYITERATOR_H__

#include <cls/iterator.h>
#include <cls/arrayaccess.h>
#include <cls/seekableiterator.h>
#include <cls/countable.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/iterator.php line 33 */
class c_arrayiterator : virtual public c_iterator, virtual public c_arrayaccess, virtual public c_seekableiterator, virtual public c_countable {
  BEGIN_CLASS_MAP(arrayiterator)
    PARENT_CLASS(traversable)
    PARENT_CLASS(iterator)
    PARENT_CLASS(arrayaccess)
    PARENT_CLASS(seekableiterator)
    PARENT_CLASS(countable)
  END_CLASS_MAP(arrayiterator)
  DECLARE_CLASS(arrayiterator, ArrayIterator, ObjectData)
  DECLARE_INVOKES_FROM_EVAL
  void init();
  public: Variant m_arr;
  public: Variant m_flags;
  public: void t___construct(Variant v_array, Variant v_flags = 0LL /* SORT_REGULAR */);
  public: ObjectData *create(Variant v_array, Variant v_flags = 0LL /* SORT_REGULAR */);
  public: ObjectData *dynCreate(CArrRef params, bool init = true);
  public: void dynConstruct(CArrRef params);
  public: void t_append(CVarRef v_value);
  public: bool t_asort();
  public: int t_count();
  public: Variant t_current();
  public: Variant t_getarraycopy();
  public: Variant t_getflags();
  public: Variant t_key();
  public: bool t_ksort();
  public: Variant t_natcasesort();
  public: Variant t_natsort();
  public: Variant t_next();
  public: bool t_offsetexists(CVarRef v_index);
  public: Variant t_offsetget(Variant v_index);
  public: virtual Variant &___offsetget_lval(Variant v_index);
  public: void t_offsetset(CVarRef v_index, CVarRef v_newval);
  public: void t_offsetunset(CVarRef v_index);
  public: Variant t_rewind();
  public: void t_seek(CVarRef v_position);
  public: void t_setflags(CVarRef v_flags);
  public: bool t_uasort(CVarRef v_cmp_function);
  public: bool t_uksort(Variant v_cmp_function);
  public: bool t_valid();
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_CLS_ARRAYITERATOR_H__
