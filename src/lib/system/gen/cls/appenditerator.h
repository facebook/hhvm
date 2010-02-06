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

#ifndef __GENERATED_CLS_APPENDITERATOR_H__
#define __GENERATED_CLS_APPENDITERATOR_H__

#include <cls/outeriterator.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/iterator.php line 145 */
class c_appenditerator : virtual public c_outeriterator {
  BEGIN_CLASS_MAP(appenditerator)
    PARENT_CLASS(outeriterator)
  END_CLASS_MAP(appenditerator)
  DECLARE_CLASS(appenditerator, AppendIterator, ObjectData)
  DECLARE_INVOKES_FROM_EVAL
  void init();
  public: p_arrayiterator m_iterators;
  Variant doCall(Variant v_name, Variant v_arguments, bool fatal);
  public: void t___construct();
  public: ObjectData *create();
  public: ObjectData *dynCreate(CArrRef params, bool init = true);
  public: void dynConstruct(CArrRef params);
  public: void t_append(p_iterator v_it);
  public: Variant t_getinneriterator();
  public: void t_rewind();
  public: bool t_valid();
  public: Variant t_current();
  public: Variant t_key();
  public: void t_next();
  public: Variant t___call(Variant v_func, Variant v_params);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_CLS_APPENDITERATOR_H__
