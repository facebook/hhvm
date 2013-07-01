/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/job_queue.h"
#include "gtest/gtest.h"

namespace HPHP {

TEST(JobQueue, Ordering) {
  {
    // FIFO only.
    JobQueue<int> job_queue(1, false, 0, false);
    for (int i = 0; i < 100; ++i) {
      job_queue.enqueue(i);
    }

    EXPECT_EQ(100, job_queue.getQueuedJobs());

    for (int i = 0; i < 100; ++i) {
      EXPECT_EQ(i, job_queue.dequeue(0));
    }
  }

  {
    // LIFO only.
    JobQueue<int> job_queue(1, false, 0, false, 0);
    for (int i = 0; i < 100; ++i) {
      job_queue.enqueue(i);
    }

    EXPECT_EQ(100, job_queue.getQueuedJobs());

    for (int i = 0; i < 100; ++i) {
      EXPECT_EQ(100 - i - 1, job_queue.dequeue(0));
    }
  }

  {
    // Hybrid. First do 50 LIFO, then 50 FIFO.
    JobQueue<int> job_queue(1, false, 0, false, 50);
    for (int i = 0; i < 100; ++i) {
      job_queue.enqueue(i);
    }

    EXPECT_EQ(100, job_queue.getQueuedJobs());

    for (int i = 0; i < 50; ++i) {
      EXPECT_EQ(100 - i - 1, job_queue.dequeue(0));
    }
    for (int i = 0; i < 50; ++i) {
      EXPECT_EQ(i, job_queue.dequeue(0));
    }
  }
}

}
