#include <sys/mman.h>
#include <cstring>
#include <thread>
#include <cassert>
#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdint>

// Allocate a page of read-write-execute memory
uint8_t* alloc_executable(size_t size) {
  void* mem = mmap(nullptr, size,
                    PROT_READ | PROT_WRITE | PROT_EXEC,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (mem == MAP_FAILED) return nullptr;
  assert(((uintptr_t)mem & (64 - 1)) == 0);
  return reinterpret_cast<uint8_t*>(mem);
}

const uint8_t code2[] = {
  0x66, 0xB9, 0x88, 0x77,                                     // movw %cx, 0x7788
  0x66, 0xBA, 0x11, 0x22,                                     // movw %dx, 0x2211
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // nops to make the imm of the next movl start at 64 bytes alignment
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90,
  0x66, 0xB8, 0x11, 0x22,                                     // movw %ax, 0x2211
  0x66, 0x39, 0xC8,                                           // cmpw %cx, %ax
  0x74, 0xF7,                                                 // je -9
  0x66, 0x39, 0xD0,                                           // cmpw %dx, %ax
  0x74, 0xF2,                                                 // je -14
  0xC3                                                        // ret
};

const uint8_t code4[] = {
  0xB9, 0x88, 0x77, 0x66, 0x55,                               // movl %ecx, 0x55667788
  0xBA, 0x11, 0x22, 0x33, 0x44,                               // movl %edx, 0x44332211
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // nops to make the imm of the next movl start at 64 bytes alignment
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90,
  0xB8, 0x11, 0x22, 0x33, 0x44,                               // movl %eax, 0x44332211
  0x39, 0xC8,                                                 // cmpl %ecx, %eax
  0x74, 0xF7,                                                 // je -9
  0x39, 0xD0,                                                 // cmpl %edx, %eax
  0x74, 0xF3,                                                 // je -13
  0xC3                                                        // ret
};

const uint8_t code8[] = {
  0x48, 0xB9, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // movq %rcx, 0x1122334455667788
  0x48, 0xBA, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, // movq %rdx, 0x8877665544332211
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, // nops to make the imm of the next movq start at 64 bytes alignment
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90,
  0x90, 0x90,
  0x48, 0xB8, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, // movq %rax, 0x8877665544332211
  0x48, 0x39, 0xC8,                                           // cmpq %rcx, %rax
  0x74, 0xF1,                                                 // je -15
  0x48, 0x39, 0xD0,                                           // cmpq %rdx, %rax
  0x74, 0xEC,                                                 // je -20
  0xC3                                                        // ret
};

struct Data {
  const uint8_t* code;
  size_t codeLen;
  size_t cmpLen;
};

template <typename T>
void instruction_stress(const Data& d, uint8_t* mem, size_t reps) {
  printf("Width %zu\n", sizeof(T));

  for (size_t offset = 0; offset < 64; offset++) {
    for (size_t j = 0; j < offset; j++) {
      mem[j] = 0x90;
    }
    std::memcpy(mem + offset, d.code, d.codeLen);

    std::atomic<bool> done{false};

    // Cast to a function pointer
    using Func = uint64_t(*)();
    Func fn = reinterpret_cast<Func>(mem);

    // Launch another thread to execute it
    std::thread reader([&]() {
      uint64_t result = fn();
      if ((T)result != (T)0x8877665544332211ll && (T)result != (T)0x1122334455667788ll) {
        printf("%zu: 0x%016lx REPRO!!!\n", offset, (uint64_t)(T)result);
      } else {
        printf("%zu: Good\n", offset);
      }
      done.store(true, std::memory_order_relaxed);
    });

    std::thread writer([&]() {
      uint64_t vals[2] = {
        0x1122334455667788ll,
        0x8877665544332211ll,
      };
      for (size_t i = 0; i < reps && !done; i++) {
        if (i % 999 == 0) {
          usleep(1);
        }
        *reinterpret_cast<T*>(mem + offset + 64) = vals[i % 2];
      }
      // We set one je to jne so that the reader thread returns
      mem[offset + 64 + sizeof(T) + d.cmpLen + 2 + d.cmpLen] = 0x75;
    });

    reader.join();
    writer.join();
    usleep(10);
  }
}

template <typename T>
void read_stress(size_t reps) {
  printf("Width %zu\n", sizeof(T));

  alignas(64) uint8_t mem[128] = {};
  uint64_t vals[2] = {
    0x1122334455667788ll,
    0x8877665544332211ll,
  };

  for (size_t offset = 0; offset < 64; offset++) {
    volatile T* ptr = reinterpret_cast<volatile T*>(&mem[offset]);
    *ptr = vals[0];

    std::atomic<bool> done{false};

    std::thread reader([&]() {
      bool repro = false;
      while (!done.load(std::memory_order_relaxed)) {
        auto v = *ptr;
        if (v != (T)vals[0] && v != (T)vals[1]) {
          printf("%zu: 0x%016lx REPRO!!!\n", offset, (uint64_t)(T)v);
          repro = true;
          break;
        }
      }
      done.store(true, std::memory_order_relaxed);
      if (!repro) {
        printf("%zu: Good\n", offset);
      }
    });

    std::thread writer([&]() {
      for (size_t i = 0; i < reps && !done; i++) {
        if (i % 999 == 0) {
          usleep(1);
        }
        *ptr = vals[i % 2];
      }
      done.store(true, std::memory_order_relaxed);
    });

    reader.join();
    writer.join();
    usleep(10);
  }
}

int main(int argc, char* argv[]) {
  size_t reps = 1000000000;
  if (argc > 1) {
    reps = atoi(argv[1]);
  }

  printf("Read through instruction pipeline\n");
  const size_t code_size = 4096;
  auto mem = alloc_executable(code_size);
  {
    Data d = {
      code8,
      83,
      3
    };
    instruction_stress<uint64_t>(d, mem, reps);
  }
  {
    Data d = {
      code4,
      77,
      2
    };
    instruction_stress<uint32_t>(d, mem, reps);
  }
  {
    Data d = {
      code2,
      77,
      3
    };
    instruction_stress<uint16_t>(d, mem, reps);
  }
  munmap(mem, code_size);

  printf("\nRead through memory read\n");

  read_stress<uint64_t>(reps);
  read_stress<uint32_t>(reps);
  read_stress<uint16_t>(reps);

  printf("Done\n");
}
