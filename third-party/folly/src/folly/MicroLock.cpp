/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/MicroLock.h>

#include <thread>

#include <folly/portability/Asm.h>

namespace folly {

uint8_t MicroLockCore::lockSlowPath(
    uint32_t oldWord,
    detail::Futex<>* wordPtr,
    unsigned baseShift,
    unsigned maxSpins,
    unsigned maxYields) noexcept {
  uint32_t newWord;
  unsigned spins = 0;
  uint32_t heldBit = 1 << baseShift;
  uint32_t waitBit = heldBit << 1;
  uint32_t needWaitBit = 0;

retry:
  if ((oldWord & heldBit) != 0) {
    ++spins;
    if (spins > maxSpins + maxYields) {
      // Somebody appears to have the lock.  Block waiting for the
      // holder to unlock the lock.  We set heldbit(slot) so that the
      // lock holder knows to FUTEX_WAKE us.
      newWord = oldWord | waitBit;
      if (newWord != oldWord) {
        if (!wordPtr->compare_exchange_weak(
                oldWord,
                newWord,
                std::memory_order_relaxed,
                std::memory_order_relaxed)) {
          goto retry;
        }
      }
      detail::futexWait(wordPtr, newWord, heldBit);
      needWaitBit = waitBit;
    } else if (spins > maxSpins) {
      // sched_yield(), but more portable
      std::this_thread::yield();
    } else {
      folly::asm_volatile_pause();
    }
    oldWord = wordPtr->load(std::memory_order_relaxed);
    goto retry;
  }

  newWord = oldWord | heldBit | needWaitBit;
  if (!wordPtr->compare_exchange_weak(
          oldWord,
          newWord,
          std::memory_order_acquire,
          std::memory_order_relaxed)) {
    goto retry;
  }
  return decodeDataFromWord(newWord, baseShift);
}
} // namespace folly
