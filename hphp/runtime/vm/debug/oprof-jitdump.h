/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <sys/time.h>
#include <time.h>
#include <stdint.h>

/* JiTD */
#define JITHEADER_MAGIC         0x4A695444
#define JITHEADER_MAGIC_SW      0x4454694A

#define PADDING_8ALIGNED(x) ((((x) + 7) & 7) ^ 7)

#define JITHEADER_VERSION 1

enum jitdump_flags_bits {
  JITDUMP_FLAGS_ARCH_TIMESTAMP_BIT,
  JITDUMP_FLAGS_MAX_BIT,
};

#define JITDUMP_FLAGS_ARCH_TIMESTAMP  (1ULL << JITDUMP_FLAGS_ARCH_TIMESTAMP_BIT)

#define JITDUMP_FLAGS_RESERVED (JITDUMP_FLAGS_MAX_BIT < 64 ? \
                               (~((1ULL << JITDUMP_FLAGS_MAX_BIT) - 1)) : 0)

struct jitheader {
  uint32_t magic;       /* characters "jItD" */
  uint32_t version;     /* header version */
  uint32_t total_size;  /* total size of header */
  uint32_t elf_mach;    /* elf mach target */
  uint32_t pad1;        /* reserved */
  uint32_t pid;         /* JIT process id */
  uint64_t timestamp;   /* timestamp */
  uint64_t flags;       /* flags */
};

enum jit_record_type {
  JIT_CODE_LOAD        = 0,
  JIT_CODE_MOVE        = 1,
  JIT_CODE_DEBUG_INFO  = 2,
  JIT_CODE_CLOSE       = 3,
  JIT_CODE_MAX
};

/* record prefix (mandatory in each record) */
struct jr_prefix {
  uint32_t id;
  uint32_t total_size;
  uint64_t timestamp;
};

struct jr_code_load {
  struct jr_prefix p;

  uint32_t pid;
  uint32_t tid;
  uint64_t vma;
  uint64_t code_addr;
  uint64_t code_size;
  uint64_t code_index;
};

struct jr_code_close {
  struct jr_prefix p;
};

struct jr_code_move {
  struct jr_prefix p;

  uint32_t pid;
  uint32_t tid;
  uint64_t vma;
  uint64_t old_code_addr;
  uint64_t new_code_addr;
  uint64_t code_size;
  uint64_t code_index;
};

struct debug_entry {
  uint64_t addr;
  int lineno;      /* source line number starting at 1 */
  int discrim;      /* column discriminator, 0 is default */
  const char name[0]; /* null terminated filename, \xff\0 if same as previous entry */
};

struct jr_code_debug_info {
  struct jr_prefix p;

  uint64_t code_addr;
  uint64_t nr_entry;
  struct debug_entry entries[0];
};

union jr_entry {
  struct jr_code_debug_info info;
  struct jr_code_close close;
  struct jr_code_load load;
  struct jr_code_move move;
  struct jr_prefix prefix;
};

#endif /* !JITDUMP_H */
