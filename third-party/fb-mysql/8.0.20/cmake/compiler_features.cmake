# Copyright (c) 2020, Percona and/or its affiliates. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */


# Functions to detect features supported by compiler

include(CheckCXXSourceCompiles)

set(CMAKE_REQUIRED_FLAGS "-msse4.2 --std=c++11 -Wno-error")
CHECK_CXX_SOURCE_COMPILES("
#include <cstdint>
#include <nmmintrin.h>
int main() {
  auto x = _mm_crc32_u32(0, 0);
}
" HAVE_SSE42)


set(CMAKE_REQUIRED_FLAGS "-mpclmul --std=c++11 -Wno-error")
CHECK_CXX_SOURCE_COMPILES("
#include <cstdint>
#include <wmmintrin.h>
int main() {
  const auto a = _mm_set_epi64x(0, 0);
  const auto b = _mm_set_epi64x(0, 0);
  const auto c = _mm_clmulepi64_si128(a, b, 0x00);
  auto d = _mm_cvtsi128_si64(c);
}
" HAVE_PCLMUL)


set(CMAKE_REQUIRED_FLAGS "-mavx2 --std=c++11 -Wno-error")
CHECK_CXX_SOURCE_COMPILES("
#include <cstdint>
#include <immintrin.h>
int main() {
  const auto a = _mm256_setr_epi32(0, 1, 2, 3, 4, 7, 6, 5);
  const auto b = _mm256_permutevar8x32_epi32(a, a);
}
" HAVE_AVX2)


set(CMAKE_REQUIRED_FLAGS "-mbmi --std=c++11 -Wno-error")
CHECK_CXX_SOURCE_COMPILES("
#include <cstdint>
#include <immintrin.h>
#include <cstdint>
#include <immintrin.h>
int main(int argc, char *argv[]) {
  return (int)_tzcnt_u64((uint64_t)argc);
}
" HAVE_BMI)


set(CMAKE_REQUIRED_FLAGS "-mlzcnt --std=c++11 -Wno-error")
CHECK_CXX_SOURCE_COMPILES("
#include <cstdint>
#include <immintrin.h>
int main(int argc, char *argv[]) {
  return (int)_lzcnt_u64((uint64_t)argc);
}
" HAVE_LZCNT)


set(CMAKE_REQUIRED_FLAGS "-faligned-new -Wno-error")
CHECK_CXX_SOURCE_COMPILES("
struct alignas(1024) t {int a;};
int main() {}
" HAVE_ALIGNED_NEW)


set(CMAKE_REQUIRED_FLAGS "--std=c++11 -Wno-error")
CHECK_CXX_SOURCE_COMPILES("
#include <cstdint>
int main() {
  uint64_t a = 0xffffFFFFffffFFFF;
  __uint128_t b = __uint128_t(a) * a;
  a = static_cast<uint64_t>(b >> 64);
}
" HAVE_UINT128_EXTENSION)


set(CMAKE_REQUIRED_FLAGS "${CMAKE_CXX_LINK_FLAGS} -Wno-error")
FIND_LIBRARY(URING_LIBRARY NAMES liburing.a)
IF (URING_LIBRARY)
	set(CMAKE_REQUIRED_LIBRARIES ${URING_LIBRARY})
	CHECK_CXX_SOURCE_COMPILES("
	#include <liburing.h>
	int main() {
	  struct io_uring ring;
	  io_uring_queue_init(1, &ring, 0);
	  return 0;
	}
	" HAVE_URING)
ENDIF()


FIND_LIBRARY(MEMKIND_LIBRARY NAMES libmemkind.a)
IF (MEMKIND_LIBRARY)
	set(CMAKE_REQUIRED_LIBRARIES ${MEMKIND_LIBRARY} -ldl -lpthread -lnuma)
	CHECK_CXX_SOURCE_COMPILES("
	#include <memkind.h>
	int main() {
	  memkind_malloc(MEMKIND_DAX_KMEM, 1024);
	  return 0;
	}
	" HAVE_MEMKIND)
ENDIF()

unset(CMAKE_REQUIRED_LIBRARIES)


set(CMAKE_REQUIRED_FLAGS "-Wno-error")
CHECK_CXX_SOURCE_COMPILES("
#if defined(_MSC_VER) && !defined(__thread)
#define __thread __declspec(thread)
#endif
int main() {
  static __thread int tls;
}
" HAVE_THREAD_LOCAL)


CHECK_CXX_SOURCE_COMPILES("
#include <fcntl.h>
#include <linux/falloc.h>
int main() {
  int fd = open(\"/dev/null\", 0);
  fallocate(fd, FALLOC_FL_KEEP_SIZE, 0, 1024);
}
" HAVE_FALLOCATE)


CHECK_CXX_SOURCE_COMPILES("
#include <pthread.h>
int main() {
  int x = PTHREAD_MUTEX_ADAPTIVE_NP;
}
" HAVE_PTHREAD_MUTEX_ADAPTIVE_NP)


CHECK_CXX_SOURCE_COMPILES("
#include <pthread.h>
#include <execinfo.h>
int main() {
  void* frames[1];
  backtrace_symbols(frames, backtrace(frames, 1));
  return 0;
}
" HAVE_BACKTRACE_SYMBOLS)


CHECK_CXX_SOURCE_COMPILES("
#include <fcntl.h>
int main() {
  int fd = open(\"/dev/null\", 0);
  sync_file_range(fd, 0, 1024, SYNC_FILE_RANGE_WRITE);
}
" HAVE_SYNC_FILE_RANGE_WRITE)

unset(CMAKE_REQUIRED_FLAGS)


include(CheckCXXSymbolExists)

if(CMAKE_SYSTEM_NAME MATCHES "^FreeBSD")
  check_cxx_symbol_exists(malloc_usable_size malloc_np.h HAVE_MALLOC_USABLE_SIZE)
else()
  check_cxx_symbol_exists(malloc_usable_size malloc.h HAVE_MALLOC_USABLE_SIZE)
endif()

check_cxx_symbol_exists(sched_getcpu sched.h ROCKSDB_SCHED_GETCPU_PRESENT)
check_cxx_symbol_exists(getauxval sys/auxv.h HAVE_AUXV_GETAUXVAL)


MACRO(ROCKSDB_SET_DEFINTIONS)
	if(HAVE_SSE42)
	  add_definitions(-DHAVE_SSE42)
	endif()

	IF (HAVE_PCLMUL)
	  add_definitions(-DHAVE_PCLMUL)
	ENDIF ()

	if(HAVE_AVX2 AND NOT ROCKSDB_DISABLE_AVX2)
	  add_definitions(-DHAVE_AVX2)
	endif()

	if(HAVE_BMI)
	  add_definitions(-DHAVE_BMI)
	endif()

	if(HAVE_LZCNT)
	  add_definitions(-DHAVE_LZCNT)
	endif()

	if(HAVE_ALIGNED_NEW AND NOT ROCKSDB_DISABLE_ALIGNED_NEW)
	  add_definitions(-DHAVE_ALIGNED_NEW)
	endif()

	if(HAVE_UINT128_EXTENSION)
	  add_definitions(-DHAVE_UINT128_EXTENSION)
	endif()

	if(HAVE_URING AND ROCKSDB_USE_IO_URING)
	  add_definitions(-DROCKSDB_IOURING_PRESENT)
	endif()

	if(HAVE_MEMKIND AND NOT ROCKSDB_DISABLE_MEMKIND)
	  add_definitions(-DMEMKIND)
	endif()

	if(HAVE_THREAD_LOCAL)
	  add_definitions(-DROCKSDB_SUPPORT_THREAD_LOCAL)
	endif()

	if(HAVE_FALLOCATE AND NOT ROCKSDB_DISABLE_FALLOCATE)
	  add_definitions(-DROCKSDB_FALLOCATE_PRESENT)
	endif()

	if(WITH_NUMA)
	  add_definitions(-DNUMA)
	endif()

	if(WITH_JEMALLOC)
	  add_definitions(-DROCKSDB_JEMALLOC)
	endif()

	if(HAVE_SYNC_FILE_RANGE_WRITE AND NOT ROCKSDB_DISABLE_SYNC_FILE_RANGE)
	  add_definitions(-DROCKSDB_RANGESYNC_PRESENT)
	endif()

	if(HAVE_PTHREAD_MUTEX_ADAPTIVE_NP AND NOT ROCKSDB_DISABLE_PTHREAD_MUTEX_ADAPTIVE_NP)
	  add_definitions(-DROCKSDB_PTHREAD_ADAPTIVE_MUTEX)
	endif()

	if(HAVE_MALLOC_USABLE_SIZE AND ROCKSDB_USE_MALLOC_USABLE_SIZE)
	  add_definitions(-DROCKSDB_MALLOC_USABLE_SIZE)
	endif()

	if(HAVE_BACKTRACE_SYMBOLS AND NOT ROCKSDB_DISABLE_BACKTRACE)
	  add_definitions(-DROCKSDB_BACKTRACE)
	endif()

	if(ROCKSDB_SCHED_GETCPU_PRESENT AND NOT ROCKSDB_DISABLE_SCHED_GETCPU)
	  add_definitions(-DROCKSDB_SCHED_GETCPU_PRESENT -DHAVE_SCHED_GETCPU=1)
	endif()

	if(HAVE_AUXV_GETAUXVAL AND NOT ROCKSDB_DISABLE_AUXV_GETAUXVAL)
	  add_definitions(-DROCKSDB_AUXV_GETAUXVAL_PRESENT)
	endif()
ENDMACRO()
