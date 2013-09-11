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
#include "hphp/util/job-queue.h"

#include <thread>
#include <gtest/gtest.h>

namespace HPHP {

TEST(JobQueue, Ordering) {
  {
    // FIFO only.
    JobQueue<int> job_queue(1, false, 0, false);
    for (int i = 0; i < 100; ++i) {
      job_queue.enqueue(i);
    }

    EXPECT_EQ(100, job_queue.getQueuedJobs());

    bool expired;
    for (int i = 0; i < 100; ++i) {
      EXPECT_EQ(i, job_queue.dequeueMaybeExpired(0, 0, false, &expired));
    }
  }

  {
    // LIFO only.
    JobQueue<int> job_queue(1, false, 0, false, 0);
    for (int i = 0; i < 100; ++i) {
      job_queue.enqueue(i);
    }

    EXPECT_EQ(100, job_queue.getQueuedJobs());

    bool expired;
    for (int i = 0; i < 100; ++i) {
      EXPECT_EQ(100 - i - 1, job_queue.dequeueMaybeExpired(0, 0,
                                                           false, &expired));
    }
  }

  {
    // Hybrid. First do 50 LIFO, then 50 FIFO.
    JobQueue<int> job_queue(1, false, 0, false, 50);
    for (int i = 0; i < 100; ++i) {
      job_queue.enqueue(i);
    }

    EXPECT_EQ(100, job_queue.getQueuedJobs());

    bool expired;
    for (int i = 0; i < 50; ++i) {
      EXPECT_EQ(100 - i - 1, job_queue.dequeueMaybeExpired(0, 0,
                                                           false, &expired));
    }
    for (int i = 0; i < 50; ++i) {
      EXPECT_EQ(i, job_queue.dequeueMaybeExpired(0, 0, false, &expired));
    }
  }
}

TEST(JobQueue, Expiration) {
  timespec timeOk;
  clock_gettime(CLOCK_MONOTONIC, &timeOk);
  timespec timeExpired = timeOk;
  timeExpired.tv_sec += 31;

  {
    JobQueue<int> fifo_queue(1, false, 0, false, INT_MAX, 30000);
    fifo_queue.enqueue(1);
    fifo_queue.enqueue(2);
    fifo_queue.enqueue(3);

    bool expired = false;
    EXPECT_EQ(1, fifo_queue.dequeueMaybeExpiredImpl(0, 0, true,
                                                    timeOk, &expired));
    EXPECT_FALSE(expired);
    EXPECT_EQ(2, fifo_queue.dequeueMaybeExpiredImpl(0, 0, true, timeExpired,
                                                    &expired));
    EXPECT_TRUE(expired);
    EXPECT_EQ(3, fifo_queue.dequeueMaybeExpiredImpl(0, 0, true,
                                                    timeOk, &expired));
    EXPECT_FALSE(expired);
  }

  {
    JobQueue<int> lifo_queue(1, false, 0, false, 0, 30000);
    lifo_queue.enqueue(1);
    lifo_queue.enqueue(2);
    lifo_queue.enqueue(3);

    bool expired = false;
    EXPECT_EQ(3, lifo_queue.dequeueMaybeExpiredImpl(0, 0,
                                                    true, timeOk, &expired));
    EXPECT_FALSE(expired);
    // now we should get a job from the beginning of the queue even though we
    // are in lifo mode before request expiration is enabled.
    EXPECT_EQ(1, lifo_queue.dequeueMaybeExpiredImpl(0, 0, true, timeExpired,
                                                    &expired));
    EXPECT_TRUE(expired);
    EXPECT_EQ(2, lifo_queue.dequeueMaybeExpiredImpl(0, 0,
                                                    true, timeOk, &expired));
    EXPECT_FALSE(expired);
  }

  {
    // job reaper.
    JobQueue<int> lifo_queue(1, false, 0, false, 0, 30000);
    lifo_queue.enqueue(1);
    lifo_queue.enqueue(2);
    lifo_queue.enqueue(3);
    lifo_queue.enqueue(4);
    lifo_queue.enqueue(5);
    lifo_queue.setJobReaperId(1);

    // manipulate m_jobs timestamp to simulate time passing.
    lifo_queue.m_jobQueues[0][0].second.tv_sec -= 32;
    lifo_queue.m_jobQueues[0][1].second.tv_sec -= 31;

    // having job reaper should not affect anything other threads are doing.
    bool expired = false;
    EXPECT_EQ(1, lifo_queue.dequeueMaybeExpired(0, 0, true, &expired));
    EXPECT_TRUE(expired);
    EXPECT_EQ(2, lifo_queue.dequeueMaybeExpired(1, 0, true, &expired));
    EXPECT_TRUE(expired);

    // now no more jobs are expired. job reaper would block.
    std::atomic<int> value(-1);
    std::thread thread([&]() {
        bool expired;
        value.store(lifo_queue.dequeueMaybeExpired(1, 0, true, &expired));
      });
    EXPECT_EQ(-1, value.load());
    // even if you notify it.
    lifo_queue.notify();
    EXPECT_EQ(-1, value.load());

    // but normal workers should proceed.
    EXPECT_EQ(5, lifo_queue.dequeueMaybeExpired(0, 0, true, &expired));
    EXPECT_FALSE(expired);

    // now set the first job to be expired.
    lifo_queue.m_jobQueues[0][0].second.tv_sec -= 32;
    lifo_queue.notify();

    // busy wait until value is updated.
    thread.join();
    EXPECT_EQ(3, value.load());

    // stop case
    bool exceptionCaught = false;
    std::thread thread1([&]() {
        bool expired;
        try {
          lifo_queue.dequeueMaybeExpired(1, 0, true, &expired);
        } catch (const JobQueue<int>::StopSignal&) {
          exceptionCaught = true;
        }
      });
    lifo_queue.stop();
    thread1.join();
    EXPECT_TRUE(exceptionCaught);
  }
}

TEST(JobQueue, Priority) {
  JobQueue<int> fifo_queue(1, false, 0, false, INT_MAX, 30, 3);
  fifo_queue.enqueue(1);
  fifo_queue.enqueue(2);
  fifo_queue.enqueue(3, 2);
  fifo_queue.enqueue(4, 0);
  fifo_queue.enqueue(5, 1);
  fifo_queue.enqueue(6, 2);

  bool expired;
  EXPECT_EQ(3, fifo_queue.dequeueMaybeExpired(0, 0, true, &expired));
  EXPECT_EQ(6, fifo_queue.dequeueMaybeExpired(0, 0, true, &expired));
  EXPECT_EQ(5, fifo_queue.dequeueMaybeExpired(0, 0, true, &expired));
  EXPECT_EQ(1, fifo_queue.dequeueMaybeExpired(0, 0, true, &expired));
  EXPECT_EQ(2, fifo_queue.dequeueMaybeExpired(0, 0, true, &expired));
  EXPECT_EQ(4, fifo_queue.dequeueMaybeExpired(0, 0, true, &expired));
}

}
