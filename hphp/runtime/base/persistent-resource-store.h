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
#ifndef incl_HPHP_PERSISTENT_RESOURCE_STORE_H_
#define incl_HPHP_PERSISTENT_RESOURCE_STORE_H_

#include <string>
#include <map>

#include "hphp/util/thread-local.h"

namespace HPHP {

struct ResourceData;

//////////////////////////////////////////////////////////////////////

/*
 * The PersistentResourceStore is used to allow some extension resources
 * to last across requests.  These "persistent resources" are
 * thread-local, but not request local, and are managed using this
 * singleton.
 *
 * Take care when creating these resources that they do not maintain
 * pointers into the request local ("smart") heap across requests.
 * The resources themselves also must be allocated outside of the
 * request local heap.
 */
struct PersistentResourceStore {
  PersistentResourceStore() = default;
  PersistentResourceStore(const PersistentResourceStore&) = delete;
  PersistentResourceStore& operator=(const PersistentResourceStore&) = delete;
  ~PersistentResourceStore();

  int size() const;

  void set(const char* type, const char* name, ResourceData* obj);
  ResourceData* get(const char* type, const char* name);
  void remove(const char* type, const char* name);

  const std::map<std::string,ResourceData*>& getMap(const char* type);

private:
  void removeObject(ResourceData* data);

private:
  std::map<std::string,std::map<std::string,ResourceData*>>
    m_objects;
};

//////////////////////////////////////////////////////////////////////

extern DECLARE_THREAD_LOCAL_NO_CHECK(PersistentResourceStore,
                                     g_persistentResources);

//////////////////////////////////////////////////////////////////////

}


#endif
