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

#ifndef __HPHP_SWEEPABLE_H__
#define __HPHP_SWEEPABLE_H__

#include <util/base.h>
#include <util/thread_local.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Raw pointers that need to be deleted during garbage collection time.
 */
class Sweepable {
public:
  static void SweepAll();

public:
  Sweepable();
  virtual ~Sweepable();

  virtual void sweep() { delete this;}

  void incPersistent() { ++m_persistentCount;}
  void decPersistent() { --m_persistentCount;}

  /**
   * Excluding this from being swept(). This is useful for child Sweepable
   * inside a parent Sweepable, when parent's destructor will delete this
   * object. For example, ZipFile containing PlainFile.
   */
  void unregister();

private:
  typedef hphp_hash_set<Sweepable*, pointer_hash<Sweepable> > SweepableSet;
  class SweepData {
  public:
    SweepData() : sweeping(false) {}
    bool sweeping;
    SweepableSet sweepables;
  };
  static DECLARE_THREAD_LOCAL(SweepData, s_sweep_data);

  int m_persistentCount;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __HPHP_SWEEPABLE_H__ */
