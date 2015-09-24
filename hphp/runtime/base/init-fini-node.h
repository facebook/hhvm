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

#ifndef incl_HPHP_THREAD_INIT_FINI_H_
#define incl_HPHP_THREAD_INIT_FINI_H_

#include "hphp/util/portability.h"
#include "hphp/util/assertions.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct InitFiniNode {
  enum class When {
    RequestInit,
      RequestFini,
      ThreadInit,
      ThreadFini,
      ProcessInit,
      ProcessExit,
      ServerPreInit,
      ServerInit,
      ServerExit,
      GlobalsInit,

      Sentinel
  };

  const static unsigned NumNodes = static_cast<unsigned>(When::Sentinel);

  InitFiniNode(void(*f)(), When when) {
    InitFiniNode *&n = node(when);
    func = f;
    next = n;
    n = this;
  }

  static void RequestInit()   { iterate(When::RequestInit);   }
  static void RequestFini()   { iterate(When::RequestFini);   }
  static void ThreadInit()    { iterate(When::ThreadInit);    }
  static void ThreadFini()    { iterate(When::ThreadFini);    }
  static void ProcessInit()   { iterate(When::ProcessInit);   }
  static void ProcessFini()   { iterate(When::ProcessExit);   }
  static void ServerPreInit() { iterate(When::ServerPreInit); }
  static void ServerInit()    { iterate(When::ServerInit);    }
  static void ServerFini()    { iterate(When::ServerExit);    }
  static void GlobalsInit()   { iterate(When::GlobalsInit);   }

 private:
  static InitFiniNode*& node(When when) {
    auto idx = static_cast<unsigned>(when);
    assert(idx < NumNodes);
    return s_nodes[idx];
  }
  void (*func)();
  InitFiniNode *next;

  static void iterate(When when) { iterate(node(when)); }
  static void iterate(InitFiniNode* node);

  static InitFiniNode *s_nodes[NumNodes];
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_THREAD_INIT_FINI_H_
