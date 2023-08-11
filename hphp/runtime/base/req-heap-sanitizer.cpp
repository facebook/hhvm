/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/req-heap-sanitizer.h"

#include "hphp/runtime/base/crash-reporter.h"
#include "hphp/runtime/base/request-info.h"
#include "hphp/util/stack-trace.h"

#ifdef __x86_64__
#include "hphp/util/asm-x64.h"
#endif

extern "C" {
#if defined(__x86_64__)
#include <xed-interface.h>
#endif
}

#include <folly/portability/SysMman.h>

namespace HPHP {

struct sigaction HeapObjectSanitizer::oldHandler{};

__thread HeapObjectSanitizer* tl_heap_sanitizer;

void* HeapObjectSanitizer::alloc(size_t size, uint32_t offset) {
  offset = (offset + 0xf) & ~0xf;       // first few pages
  auto const remaining = size - offset; // remaining pages
  auto const firstPageSize = ru(offset);
  auto const remainingPageSize = ru(remaining);
  auto const totalSize = firstPageSize + remainingPageSize;
  auto ret = (char*)mmap(nullptr, totalSize,
                         PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (ret == (char*)-1) {
    fprintf(stderr,
            "mmap() failed from HeapObjectSanitizer, errno = %d", errno);
    return nullptr;
  }
  auto const skip = (kPageSize - offset) & kPageSizeMask;
  pages[reinterpret_cast<uintptr_t>(ret)] = totalSize;
  return ret + skip;
}

void HeapObjectSanitizer::free(void* ptr) {
  auto iter = find_page_containing(ptr);
  if (iter != pages.end()) {
    pages.erase(iter);
  } else {
    // Probably a double free.
    fprintf(stderr, "double free at %p?\n", ptr);
    auto p = (void**)ptr;
    for (unsigned i = 0; i < 8; i = i + 2) {
      fprintf(stderr, "\t%p %p\n", p[i], p[i + 1]);
    }
  }
}

uint8_t* HeapObjectSanitizer::find_next_inst(uint8_t* ip) {
#if defined(__x86_64__)
  xed_machine_mode_enum_t mmode = XED_MACHINE_MODE_LONG_64;
  xed_address_width_enum_t stack_addr_width = XED_ADDRESS_WIDTH_64b;
  xed_decoded_inst_t xedd{};
  xed_decoded_inst_zero(&xedd);
  xed_decoded_inst_set_mode(&xedd, mmode, stack_addr_width);
  auto xed_error = xed_decode(&xedd,
                              XED_STATIC_CAST(const xed_uint8_t*, ip),
                              15);
  if (xed_error != XED_ERROR_NONE) {
    fprintf(stderr,
            "error xed_decode instruction at %p, xed_error = %s\n",
            ip, xed_error_enum_t2str(xed_error));
    for (unsigned i = 0; i < 15; ++i) {
      fprintf(stderr, "\t\t%x\n", *(ip + i));
    }
    return nullptr;
  }
  return ip + xed_decoded_inst_get_length(&xedd);
#else
  return nullptr;
#endif
}

bool HeapObjectSanitizer::set_page_protection(void* ptr, int prot) {
  uintptr_t pageStart = reinterpret_cast<uintptr_t>(ptr) & ~kPageSizeMask;
  int r = mprotect(reinterpret_cast<void*>(pageStart), kPageSize, prot);
  if (r == -1) {
    fprintf(stderr, "mprotect() failure on %p, prot = %x, errno = %d\n",
            ptr, prot, errno);
    return false;
  }
  return true;
}

uint8_t HeapObjectSanitizer::patch(uint8_t* addr, uint8_t byte) {
  auto ato_addr = reinterpret_cast<std::atomic_uint8_t*>((void*)addr);
  // There are exactly two values for the byte at addr, either the original one,
  // or the trap instruction. The current value must be different from the new
  // one. We wait till the expected value shows up before actually patching it
  // to allow handling of racing properly.
  auto curr = ato_addr->load();
  size_t iter = 0;
  do {
    ++iter;
    if (iter % 256 == 0) {
      // This is unusual
      fprintf(stderr,
              "unable to patch after 256 iterations, continue spinning.\n");
    }
    if (curr == byte) {
      // multiple threads racing.
      /* sleep_override */ usleep(16 * iter);
      curr = ato_addr->load();
      continue;
    }
    if (ato_addr->compare_exchange_strong(curr, byte)) {
      return curr;
    }
  } while (true);
  not_reached();
}

void HeapObjectSanitizer::install_signal_handler() {
  struct sigaction sa{};
  sigemptyset(&sa.sa_mask);
  sa.sa_flags |= SA_ONSTACK | SA_SIGINFO | SA_NODEFER;
  sa.sa_sigaction = &HeapObjectSanitizer::access_handler;
  sigaction(SIGTRAP, &sa, &oldHandler);
  sigaction(SIGSEGV, &sa, &oldHandler);
}

#ifdef __x86_64__
void
HeapObjectSanitizer::access_handler(int signo, siginfo_t* info, void* extra) {
  auto ctx = (ucontext_t *)extra;
  auto volatile ip = (uint8_t*)(ctx->uc_mcontext.gregs[REG_RIP]);
  auto dataAddr = info->si_addr;
  auto const threadid = (void*)pthread_self();

  if (signo == SIGTRAP) {
    // If we reach here, we consider it caused by a trap inserted to sanitize
    // access. We restore the page protection when appropriate.
    --ip;
    // Go back and retry the instruction after we restore the instruction
    // stream.
    ctx->uc_mcontext.gregs[REG_RIP] = reinterpret_cast<greg_t>(ip);

    if (*ip != 0xCC) {
      // We encountered int3 and entered this signal hander, but by the time we
      // look at it here, the instruction stream has already been restored, so
      // just retry the current instruction.
      if (!tl_heap_sanitizer || (tl_heap_sanitizer->trapAddr != ip)) {
        return;
      } else {
        // Maybe our thread and another thread raced to set the trap. It is
        // still safe to keep going, but print out some information to help
        // understand why.
        fprintf(stderr, "!! [%p] racy trap at %p?!!\n", threadid, ip);
        void** rip = (void**)(ctx->uc_mcontext.gregs[REG_RIP]);
        void** rbp = (void**)(ctx->uc_mcontext.gregs[REG_RBP]);
        for (int i = 0; i < 4; ++i) {
          fprintf(stderr, "\t RIP=%p \t RBP=%p\n", rip, rbp);
          if (!rbp) break;
          rip = (void**)*(rbp + 1);
          rbp = (void**)*rbp;
        }
        fprintf(stderr, "\t\t end stack\n");
        tl_heap_sanitizer->trapAddr = nullptr;
        return;
      }
    }

    // Caution: multiple threads are racing to modify the shared instruction
    // stream. If another thread sets up the trap, we don't want to do anything
    // other than waiting.
    if (tl_heap_sanitizer == nullptr || tl_heap_sanitizer->trapAddr != ip) {
      /* sleep_override */ usleep(100);
      return;
    }

    // 1. restore the instruction.
    auto const trapByte = patch(tl_heap_sanitizer->trapAddr,
                                tl_heap_sanitizer->origByte);
    if (trapByte != 0xCC) {
      fprintf(stderr, "!!!! [%p] the instruction at %p wasn't a int3? "
              "This shouldn't happen, maybe another thread restored the "
              "instruction stream for us?\n",
              threadid, tl_heap_sanitizer->trapAddr);
    }
    tl_heap_sanitizer->trapAddr = nullptr;

    // 2. mark the page read-only again.
    if (tl_heap_sanitizer->dataPage) {
      set_page_protection(tl_heap_sanitizer->dataPage, PROT_READ);
      tl_heap_sanitizer->dataPage = nullptr;
    } else {
      abort();
    }
    return;
  }

  if (signo == SIGSEGV) {
    if (dataAddr == nullptr) {
      return bt_handler(signo, info, extra);
    }
    if (tl_heap_sanitizer == nullptr) {
      return bt_handler(signo, info, extra);
    }
    if (tl_heap_sanitizer->trapAddr || tl_heap_sanitizer->dataPage) {
      // We already allowed writes to the protected page, and it still cause a
      // SIGSEGV?
      return bt_handler(signo, info, extra);
    }

    // Check if the accessed address is really under our control.
    auto iter = tl_heap_sanitizer->find_page_containing(dataAddr);
    if (iter == tl_heap_sanitizer->pages.end()) {
      // We are accessing something that is not in the watched page list of the
      // current thread. Here we rely on the assumption that an object in the
      // request heap of one thread is never accessed from another thread. So
      // the accessed object isn't being sanitized. Fallback to the default
      // handler.
      return bt_handler(signo, info, extra);
    }

    // 1. Check if dataAddr is being watched and print stack trace if so.
    char buf[4096];
    char* curr = buf;
    int remaining = sizeof(buf);
    if (tl_heap_sanitizer->addresses.count(dataAddr)) {
      auto n = snprintf(curr, remaining,
                        "[%p] changing value at %p: curr value = %x, ip =",
                        threadid, dataAddr, *(int*)(dataAddr));
      curr += n;
      remaining -= n;
      void** rip = (void**)ip;
      void** rbp = (void**)(ctx->uc_mcontext.gregs[REG_RBP]);
      // Obtain the stacktrace. Note that we don't want to resolve symbols in
      // this signal handler, so just print the raw address.
      for (unsigned i = 0; i < 12; ++i) {
        n = snprintf(curr, remaining, " %p", rip);
        curr +=n;
        remaining -=n;
        if (reinterpret_cast<uintptr_t>(rbp) < 0x7f00ffffffffull ||
            reinterpret_cast<uintptr_t>(rbp) > 0x8000ffffffffull ||
            (reinterpret_cast<uintptr_t>(rbp) & 0xf)) {
          break;
        }
        rip = (void**)*(rbp + 1);
        rbp = (void**)*rbp;
      }
      fprintf(stderr, "%s\n", buf);
    }

    // 2. Allow the write to go through.
    set_page_protection(dataAddr, PROT_READ | PROT_WRITE);

    // 3. Trap the next instruction, but another thread may be doing the same
    // thing. If they win the race, we must wait.
    auto next = find_next_inst(ip);
    if (next == nullptr) {
      fprintf(stderr, "!!!! failed to parse insts accessing %p: "
              "curr value = %d, thread %p, ip = %p\n",
              dataAddr, *(int*)(dataAddr), (void*)threadid, ip);
      next = find_next_inst(ip);
    }

    tl_heap_sanitizer->trapAddr = next;
    tl_heap_sanitizer->dataPage = dataAddr;
    set_page_protection(next, PROT_READ | PROT_WRITE | PROT_EXEC);
    tl_heap_sanitizer->origByte = patch(next, 0xCC);
    return;
  }
  // We shouldn't reach here, but just in case.
  not_reached();
  signal(signo, SIG_DFL);
  raise(signo);
}
#else // __x86_64__
void HeapObjectSanitizer::access_handler(int, siginfo_t*, void*) {
  assertx(false && "port me");
}
#endif // __x86_64__

}
