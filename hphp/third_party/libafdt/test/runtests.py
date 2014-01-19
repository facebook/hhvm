#!/usr/bin/env python
import sys
import contextlib
import traceback
import unittest
import time
import os
import subprocess
import errno
import signal
import urllib2
import threading
import Queue



PREFIX = os.environ.get("AFDT_TEST_PREFIX", "").split()



class SubprocessTestCase(unittest.TestCase):
  def setUp(self):
    def sigchld_handler(signum, frame):
      while True:
        status = os.waitpid(-1, os.WNOHANG | os.WUNTRACED | os.WCONTINUED)
        if status == (0, 0):
          break
        if os.WIFSTOPPED(status[1]) or os.WIFCONTINUED(status[1]):
          # Ignore SIGCHLDs due to stopping and starting a child
          continue
        raise Exception("child died unexpectedly: %r" % (status,))
    signal.signal(signal.SIGCHLD, sigchld_handler)

  def killChildren(self, children):
    signal.signal(signal.SIGCHLD, signal.SIG_DFL)
    for proc in children:
      try:
        if proc is not None:
          os.kill(proc.pid, signal.SIGTERM)
      except OSError, err:
        if err.errno != errno.ESRCH:
          traceback.print_exc()


class EvhttpTest(SubprocessTestCase):
  prod_port = 8080
  port0 = 9090
  port1 = 9191
  # Number of requests to send to the production port to verify
  # the set of servers that are listening on it.
  # This is nondeterministic, but we take our chances.
  iterations = 0x10000

  def setUp(self):
    SubprocessTestCase.setUp(self)

    def startserver(port):
      return subprocess.Popen(PREFIX +
          ["./server", "-a", str(port), "-s", "p" + str(port)])
    self.proc0 = None
    self.proc1 = None
    self.proc0 = startserver(self.port0)
    self.proc1 = startserver(self.port1)

    # TODO(dreiss): Check statuses so we can stop sleeping early
    time.sleep(1.0/2)
    status = os.waitpid(-1, os.WNOHANG)
    assert status == (0, 0)

  def tearDown(self):
    self.killChildren([self.proc0, self.proc1])

  def testServers(self):

    def openurl(port, path):
      with contextlib.closing(urllib2.urlopen(
        "http://localhost:%d/%s" % (port, path))) as handle:
        return handle.read()

    def checkret(port, path, content):
      self.assertEqual(openurl(port, path), content)

    def putret(port, path, q):
      q.put(openurl(port, path))

    def checkset(port, path, expect):
      results = set()
      iter = 0
      while iter < self.iterations:
        results.add(openurl(port, path))
        self.assert_(results <= expect)
        if results == expect:
          break
        iter += 1
      self.assertNotEqual(iter, self.iterations)

    # Check basic status responses
    checkret(self.port0, "status", "p%d" % self.port0)
    checkret(self.port1, "status", "p%d" % self.port1)
    # Have one server bind to production
    checkret(self.port0, "bind_prod", "bind")
    # Verify production
    checkret(self.prod_port, "status", "p%d" % self.port0)
    # Rebind detection
    checkret(self.port0, "bind_prod", "already_open")
    # Close production
    checkret(self.port0, "close_prod", "closed")
    # Verify close production
    checkret(self.port0, "close_prod", "no_prod")
    # Repeat with the other server
    checkret(self.port1, "bind_prod", "bind")
    checkret(self.prod_port, "status", "p%d" % self.port1)
    checkret(self.port1, "bind_prod", "already_open")
    checkret(self.port1, "close_prod", "closed")
    checkret(self.port1, "close_prod", "no_prod")

    # Have one server bind to production
    checkret(self.port0, "bind_prod", "bind")
    # Verify production
    checkret(self.prod_port, "status", "p%d" % self.port0)
    # Have the other server grab the socket
    checkret(self.port1, "bind_prod", "afdt")

    # Verify that both are listening
    checkset(self.prod_port, "status",
        set(["p%d" % port for port in [self.port0, self.port1]]))

    # Close the socket on the original server
    checkret(self.port0, "close_prod", "closed")

    # Verify that only the second is listening
    checkset(self.prod_port, "status",
        set(["p%d" % port for port in [self.port1]]))

    # Have the first server get the socket back
    checkret(self.port0, "bind_prod", "afdt")

    # Verify that both are listening
    checkset(self.prod_port, "status",
        set(["p%d" % port for port in [self.port0, self.port1]]))

    # Close the socket on the second server
    checkret(self.port1, "close_prod", "closed")

    # Verify that only the first is listening
    checkset(self.prod_port, "status",
        set(["p%d" % port for port in [self.port0]]))

    # Close the socket on the first server
    checkret(self.port0, "close_prod", "closed")

    # Repeat the simple case with the second server
    checkret(self.port1, "bind_prod", "bind")
    checkret(self.prod_port, "status", "p%d" % self.port1)
    checkret(self.port1, "bind_prod", "already_open")
    checkret(self.port1, "close_prod", "closed")
    checkret(self.port1, "close_prod", "no_prod")

    # Have the first server bind to production
    checkret(self.port0, "bind_prod", "bind")
    # Verify production
    checkret(self.prod_port, "status", "p%d" % self.port0)
    # Suspend that process
    self.proc0.send_signal(signal.SIGSTOP)
    # Use a background thread to have the second server grab the socket
    q = Queue.Queue()
    t = threading.Thread(target=putret, args=(self.port1, "bind_prod", q))
    t.start()
    # After a half second, we should still be waiting
    time.sleep(0.5)
    self.assert_(q.empty())
    # The second server should still be able to respond to requests
    checkret(self.port1, "status", "p%d" % self.port1)
    # Let the first server wake up and transfer the socket
    self.proc0.send_signal(signal.SIGCONT)
    # The second server should receive the socket quickly
    self.assertEqual(q.get(timeout=1.0/16), "afdt")
    t.join(1.0/16)
    self.assertFalse(t.isAlive())
    # Close the socket on the first server
    checkret(self.port0, "close_prod", "closed")
    # Verify that the second is listening
    checkret(self.prod_port, "status", "p%d" % self.port1)

    # Remove the signal handler
    signal.signal(signal.SIGCHLD, signal.SIG_DFL)
    # Shut both servers down
    checkret(self.port0, "shutdown", "shutting_down")
    checkret(self.port1, "shutdown", "shutting_down")
    # Make sure they both go down in a reasonable time
    def sigalrm_handler(signum, frame):
      raise Exception("waitpid timed out")
    signal.signal(signal.SIGALRM, sigalrm_handler)
    signal.alarm(1)
    self.assertEqual(self.proc0.wait(), 0)
    self.assertEqual(self.proc1.wait(), 0)
    self.proc0 = None
    self.proc1 = None
    signal.alarm(0)
    signal.signal(signal.SIGALRM, signal.SIG_DFL)


class CatterTest(SubprocessTestCase):
  svport = 9090

  client = "catter"

  def setUp(self):
    SubprocessTestCase.setUp(self)

  def tearDown(self):
    self.killChildren([self.svproc, self.clproc])

  def testCatter(self):
    self.svproc = None
    self.clproc = None
    self.svproc = subprocess.Popen(PREFIX +
        ["./catter", "-s"], stdout=subprocess.PIPE)
    time.sleep(1.0/4)
    self.clproc = subprocess.Popen(PREFIX +
        ["./" + self.client], stdin=subprocess.PIPE)
    time.sleep(1.0/4)
    self.clproc.stdin.write("TEST1")
    time.sleep(1.0/4)

    # Remove the signal handler
    signal.signal(signal.SIGCHLD, signal.SIG_DFL)
    # Let the server exit
    time.sleep(1.0/2)

    self.clproc.stdin.write("TEST2")
    self.clproc.stdin.close()
    time.sleep(1.0/4)

    self.assertEqual(self.svproc.stdout.read(), "TEST1TEST2")

    # Make sure they both go down in a reasonable time
    # TODO(dreiss): Factor out subprocs?
    def sigalrm_handler(signum, frame):
      raise Exception("waitpid timed out")
    signal.signal(signal.SIGALRM, sigalrm_handler)
    signal.alarm(1)
    self.assertEqual(self.svproc.wait(), 0)
    self.assertEqual(self.clproc.wait(), 0)
    self.svproc = None
    self.clproc = None
    signal.alarm(0)
    signal.signal(signal.SIGALRM, signal.SIG_DFL)


class SyncCatterTest(CatterTest):
  client = "sync_catter"



# The evhttp test relies on some really new features of libevent,
# so allow it to be disabled independently.
if os.environ.get("NO_HTTP_TEST", False):
  del EvhttpTest



if __name__ == "__main__":
    unittest.main()
