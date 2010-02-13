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

#ifndef __GENERATED_cls_recursiveiteratoriterator_h__
#define __GENERATED_cls_recursiveiteratoriterator_h__

#include <cls/outeriterator.h>
#include <cls/traversable.h>
#include <cls/iterator.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/* SRC: classes/iterator.php line 28 */
class c_recursiveiteratoriterator : virtual public c_outeriterator, virtual public c_traversable, virtual public c_iterator {
  BEGIN_CLASS_MAP(recursiveiteratoriterator)
    PARENT_CLASS(outeriterator)
    PARENT_CLASS(traversable)
    PARENT_CLASS(traversable)
    PARENT_CLASS(iterator)
  END_CLASS_MAP(recursiveiteratoriterator)
  DECLARE_CLASS(recursiveiteratoriterator, RecursiveIteratorIterator, ObjectData)
  DECLARE_INVOKES_FROM_EVAL
  void init();
  public: void t___construct(Variant v_iterator, Variant v_mode = 0LL /* recursiveiteratoriterator::LEAVES_ONLY */, Variant v_flags = 0LL);
  public: ObjectData *create(Variant v_iterator, Variant v_mode = 0LL /* recursiveiteratoriterator::LEAVES_ONLY */, Variant v_flags = 0LL);
  public: ObjectData *dynCreate(CArrRef params, bool init = true);
  public: void dynConstruct(CArrRef params);
  public: Object t_getinneriterator();
  public: Variant t_current();
  public: Variant t_key();
  public: Variant t_next();
  public: Variant t_rewind();
  public: bool t_valid();
  public: Variant m_rsrc;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_cls_recursiveiteratoriterator_h__
