/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_VSDEBUG_SERVER_OBJ_H_
#define incl_HPHP_VSDEBUG_SERVER_OBJ_H_

namespace HPHP {
namespace VSDEBUG {

enum ServerObjectType {
  Frame,
  Scope,
  Variable
};

enum ScopeType {
  Locals,
  UserDefinedConstants,
  CoreConstants,
  Superglobals
};

struct ServerObject {
  ServerObject(unsigned int objectId) : m_objectId(objectId) {}
  virtual ~ServerObject() {}
  virtual ServerObjectType objectType() = 0;
  const unsigned int m_objectId;
};

struct FrameObject : public ServerObject {
  FrameObject(unsigned int objectId, int requestId, int frameDepth)
    : ServerObject(objectId),
      m_requestId(requestId),
      m_frameDepth(frameDepth) {}

  ServerObjectType objectType() override {
    return ServerObjectType::Frame;
  }

  const int m_requestId;
  const int m_frameDepth;
};

struct ScopeObject : public ServerObject {
  ScopeObject(unsigned int objectId, int reqId, int depth, ScopeType scopeType)
    : ServerObject(objectId),
      m_requestId(reqId),
      m_frameDepth(depth),
      m_scopeType(scopeType) {}

  ServerObjectType objectType() override {
    return ServerObjectType::Scope;
  }

  const int m_requestId;
  const int m_frameDepth;
  const ScopeType m_scopeType;
};
/*
struct VariableObject : public ServerObject {
  VariableObject(
    unsigned int objectId,
    int reqId,
    int depth,

  ) {}
};
*/

}
}
#endif // incl_HPHP_VSDEBUG_SERVER_OBJ_H_
