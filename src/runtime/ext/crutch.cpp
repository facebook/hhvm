/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include <runtime/ext/crutch.h>
#include <runtime/ext/ext_ipc.h>
#include <signal.h>
#include <sys/wait.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// OpaqueObject

Object OpaqueObject::GetObject(int index) {
  if (index) {
    return Object(new OpaqueObject(index));
  }
  return Object();
}

int OpaqueObject::GetIndex(Object obj) {
  OpaqueObject *oo = dynamic_cast<OpaqueObject *>(obj.get());
  return oo ? oo->m_index : 0;
}

OpaqueObject::OpaqueObject(int index) : m_index(index) {
}

///////////////////////////////////////////////////////////////////////////////
// statics

bool Crutch::Enabled = false;
Mutex Crutch::s_mutex;
Crutch Crutch::s_singleton;

Array Crutch::Invoke(String func, Array schema, Array params) {
  Lock lock(s_mutex);
  if (!Enabled) {
    throw NotImplementedException(func);
  }
  return s_singleton.invoke(func, schema, params);
}

///////////////////////////////////////////////////////////////////////////////
// construction and destruction

Crutch::Crutch() : m_php(0) {
}

Crutch::~Crutch() {
  terminate();
}

void Crutch::init() {
  char filename[64];
  strcpy(filename, "/tmp/hphp_crutch_XXXXXX");
  int fd = mkstemp(filename);
  if (!fd) {
    throw FatalErrorException("unable to create a temporary file for crutch");
  }
  close(fd);

  m_queue = f_msg_get_queue(f_ftok(filename, "a"));
  if (m_queue.get() == NULL) {
    throw FatalErrorException("unable to create a message queue for crutch");
  }

  int pid = fork();
  if (pid == 0) {
    const char *argv[] = {"/usr/local/bin/hphp/crutch.php", filename, NULL};
    execvp(argv[0], const_cast<char**>(argv));
    _exit(-1); // something wrong
  }

  m_php = pid;
  Variant type, ret;
  if (!f_msg_receive(m_queue, 2, ref(type), MSG_MAX_SIZE, ref(ret)) ||
      !same(ret, "CRUTCH")) {
    terminate();
    throw FatalErrorException("unable to hear startup signal from crutch");
  }
}

void Crutch::terminate() {
  if (m_queue.get()) {
    f_msg_remove_queue(m_queue);
    m_queue.reset();
  }
  if (m_php) {
    kill(m_php, SIGKILL); // not needed normally but be safe
    int status = -1;
    wait(&status);
    m_php = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////
// RMI

Array Crutch::invoke(String func, Array schema, Array params) {
  if (!m_php) init();

  Array message = CREATE_VECTOR4(func, schema, params.size(), params);
  if (!f_msg_send(m_queue, 1, message)) {
    terminate();
    throw SystemCallFailure("f_msg_send");
  }

  Variant type, ret;
  if (!f_msg_receive(m_queue, 2, ref(type), MSG_MAX_SIZE, ref(ret))) {
    terminate();
    throw SystemCallFailure("f_msg_receive");
  }

  return ret.toArray();
}

///////////////////////////////////////////////////////////////////////////////
}
