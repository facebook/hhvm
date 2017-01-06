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

 * oprof-jitdump.h: jitted code info; encapsulation file format
 *
 * Adapted from the Oprofile code in jitdump.h
 * Copyright 2007 OProfile authors
 * Jens Wilke
 * Daniel Hansel
 * Copyright IBM Corporation 2007
 */
#ifndef JITDUMP_H
#define JITDUMP_H

#include <stdint.h>

/* JiTD */
#define JITHEADER_MAGIC         0x4A695444
#define JITHEADER_MAGIC_SW      0x4454694A

#define PADDING_8ALIGNED(x) ((((x) + 7) & 7) ^ 7)

#define JITHEADER_VERSION 1

enum JitdumpFlagBits {
  JITDUMP_FLAGS_ARCH_TIMESTAMP_BIT,
  JITDUMP_FLAGS_MAX_BIT,
};

#define JITDUMP_FLAGS_ARCH_TIMESTAMP  (1ULL << JITDUMP_FLAGS_ARCH_TIMESTAMP_BIT)

#define JITDUMP_FLAGS_RESERVED (JITDUMP_FLAGS_MAX_BIT < 64 ? \
                               (~((1ULL << JITDUMP_FLAGS_MAX_BIT) - 1)) : 0)

struct JitHeader {
  uint32_t magic;       /* characters "jItD" */
  uint32_t version;     /* header version */
  uint32_t total_size;  /* total size of header */
  uint32_t elf_mach;    /* elf mach target */
  uint32_t pad1;        /* reserved */
  uint32_t pid;         /* JIT process id */
  uint64_t timestamp;   /* timestamp */
  uint64_t flags;       /* flags */
};

enum class JitRecordType : uint32_t {
  JIT_CODE_LOAD,
  JIT_CODE_MOVE,
  JIT_CODE_DEBUG_INFO,
  JIT_CODE_CLOSE,
  JIT_CODE_MAX,
};

/* record prefix (mandatory in each record) */
struct JitRecPrefix {
  JitRecordType id;
  uint32_t total_size;
  uint64_t timestamp;
};

struct JitRecCodeLoad {
  JitRecPrefix p;
  uint32_t pid;
  uint32_t tid;
  uint64_t vma;
  uint64_t code_addr;
  uint64_t code_size;
  uint64_t code_index;
};

struct JitRecCodeClose {
  JitRecPrefix p;
};

struct JitRecCodeMove {
  JitRecPrefix p;
  uint32_t pid;
  uint32_t tid;
  uint64_t vma;
  uint64_t old_code_addr;
  uint64_t new_code_addr;
  uint64_t code_size;
  uint64_t code_index;
};

struct DebugEntry {
  uint64_t addr;
  /* source line number starting at 1 */
  int lineno;
  /* column discriminator, 0 is default */
  int discrim;
  /* null terminated filename, \xff\0 if same as previous entry */
  const char name[0];
};

struct JitRecCodeDebugInfo {
  JitRecPrefix p;
  uint64_t code_addr;
  uint64_t nr_entry;
  DebugEntry entries[0];
};

union JitRecEntry {
  JitRecCodeDebugInfo info;
  JitRecCodeClose close;
  JitRecCodeLoad load;
  JitRecCodeMove move;
  JitRecPrefix prefix;
};

#endif /* !JITDUMP_H */
