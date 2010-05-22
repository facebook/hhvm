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

#include <test/test_ext_ipc.h>
#include <runtime/ext/ext_ipc.h>
#include <sys/wait.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtIpc::RunTests(const std::string &which) {
  bool ret = true;

  /*
  RUN_TEST(test_ftok);
  RUN_TEST(test_msg_get_queue);
  RUN_TEST(test_msg_send);
  RUN_TEST(test_msg_receive);
  RUN_TEST(test_msg_remove_queue);
  RUN_TEST(test_msg_set_queue);
  RUN_TEST(test_msg_stat_queue);
  RUN_TEST(test_sem_acquire);
  RUN_TEST(test_sem_get);
  RUN_TEST(test_sem_release);
  RUN_TEST(test_sem_remove);
  RUN_TEST(test_shm_attach);
  RUN_TEST(test_shm_detach);
  RUN_TEST(test_shm_remove);
  RUN_TEST(test_shm_get_var);
  RUN_TEST(test_shm_put_var);
  RUN_TEST(test_shm_remove_var);
  */
  RUN_TEST(test_message_queue);
  RUN_TEST(test_semaphore);
  RUN_TEST(test_shared_memory);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtIpc::test_message_queue() {
  char filename[64];
  strcpy(filename, "/tmp/XXXXXX");
  close(mkstemp(filename));

  int64 token = f_ftok(filename, "a");
  Object queue = f_msg_get_queue(token);
  VERIFY(queue.get());

  int pid = fork();
  if (pid == 0) {
    Object q = f_msg_get_queue(token);
    assert(q.get());
    assert(f_msg_send(q, 2, "start"));
    Variant type, msg;
    assert(f_msg_receive(q, 1, ref(type), 100, ref(msg)));
    assert(f_msg_send(q, 2, msg)); // echo
    _exit(-1);
  }

  Variant type, msg;
  VERIFY(f_msg_receive(queue, 2, ref(type), 100, ref(msg)));
  VERIFY(same(msg, "start"));

  VERIFY(f_msg_send(queue, 1, "ok"));
  VERIFY(f_msg_receive(queue, 2, ref(type), 100, ref(msg)));
  VERIFY(same(msg, "ok"));

  Array ret = f_msg_stat_queue(queue);
  VS(ret["msg_qnum"], 0);
  f_msg_set_queue(queue, CREATE_MAP1("msg_perm.mode", 0666));

  f_msg_remove_queue(queue);
  int status = -1;
  wait(&status);
  return Count(true);
}

bool TestExtIpc::test_ftok()             { return test_message_queue();}
bool TestExtIpc::test_msg_get_queue()    { return test_message_queue();}
bool TestExtIpc::test_msg_send()         { return test_message_queue();}
bool TestExtIpc::test_msg_receive ()     { return test_message_queue();}
bool TestExtIpc::test_msg_remove_queue() { return test_message_queue();}
bool TestExtIpc::test_msg_set_queue()    { return test_message_queue();}
bool TestExtIpc::test_msg_stat_queue()   { return test_message_queue();}

///////////////////////////////////////////////////////////////////////////////

bool TestExtIpc::test_semaphore() {
  Variant ret = f_sem_get(0xDEADBEEF);
  VERIFY(!same(ret, false));
  Object sem = ret.toObject();
  time_t now = time(0);
  VERIFY(f_sem_acquire(sem));

  int pid = fork();
  if (pid == 0) {
    Variant ret = f_sem_get(0xDEADBEEF);
    assert(!same(ret, false));
    Object sem = ret.toObject();

    assert(f_sem_acquire(sem));

    // This isn't a sure test, but may be false if f_sem_acquire() didn't work
    time_t then = time(0);
    assert(then - now > 1);

    assert(f_sem_release(sem));
    VERIFY(f_sem_remove(sem));
    _exit(-1);
  }

  sleep(3); // aha
  VERIFY(f_sem_release(sem));
  int status = -1;
  wait(&status);
  return Count(true);
}

bool TestExtIpc::test_sem_acquire() { return test_semaphore();}
bool TestExtIpc::test_sem_get()     { return test_semaphore();}
bool TestExtIpc::test_sem_release() { return test_semaphore();}
bool TestExtIpc::test_sem_remove()  { return test_semaphore();}

///////////////////////////////////////////////////////////////////////////////

bool TestExtIpc::test_shared_memory() {
  Variant ret = f_shm_attach(0xDEADBEEF);
  VERIFY(!same(ret, false));
  int64 index = ret.toInt64();
  VERIFY(f_shm_put_var(index, 1234, "test"));

  int pid = fork();
  if (pid == 0) {
    Variant ret = f_shm_attach(index);
    assert(!same(ret, false));
    Object mem = ret.toObject();

    ret = f_shm_get_var(index, 1234);
    assert(same(ret, "test"));

    assert(f_shm_remove_var(index, 1234));
    assert(f_shm_detach(index));
    _exit(-1);
  }

  // Verifying f_shm_remove_var worked, this is not sure test though.
  ret = f_shm_get_var(index, 1234);
  for (int i = 0; i < 1000; i++) {
    if (same(ret, false)) break;
    usleep(1000);
    ret = f_shm_get_var(index, 1234);
  }
  VERIFY(same(ret, false));

  VERIFY(f_shm_remove(index));
  int status = -1;
  wait(&status);
  return Count(true);
}

bool TestExtIpc::test_shm_attach()     { return test_shared_memory();}
bool TestExtIpc::test_shm_detach()     { return test_shared_memory();}
bool TestExtIpc::test_shm_remove()     { return test_shared_memory();}
bool TestExtIpc::test_shm_get_var()    { return test_shared_memory();}
bool TestExtIpc::test_shm_put_var()    { return test_shared_memory();}
bool TestExtIpc::test_shm_remove_var() { return test_shared_memory();}
