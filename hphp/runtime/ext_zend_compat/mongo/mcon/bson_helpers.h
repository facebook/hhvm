/**
 *  Copyright 2009-2013 10gen, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include "str.h"
#include <sys/types.h>

#if PHP_C_BIGENDIAN
/* reverse the bytes in an int, wheeee stupid byte tricks */
# define BYTE1_32(b) ((b & 0xff000000) >> 24)
# define BYTE2_32(b) ((b & 0x00ff0000) >> 8)
# define BYTE3_32(b) ((b & 0x0000ff00) << 8)
# define BYTE4_32(b) ((b & 0x000000ff) << 24)
# define MONGO_32(b) (BYTE4_32(b) | BYTE3_32(b) | BYTE2_32(b) | BYTE1_32(b))

# define BYTE1_64(b) ((b & 0xff00000000000000ll) >> 56)
# define BYTE2_64(b) ((b & 0x00ff000000000000ll) >> 40)
# define BYTE3_64(b) ((b & 0x0000ff0000000000ll) >> 24)
# define BYTE4_64(b) ((b & 0x000000ff00000000ll) >> 8)
# define BYTE5_64(b) ((b & 0x00000000ff000000ll) << 8)
# define BYTE6_64(b) ((b & 0x0000000000ff0000ll) << 24)
# define BYTE7_64(b) ((b & 0x000000000000ff00ll) << 40)
# define BYTE8_64(b) ((b & 0x00000000000000ffll) << 56)
# define MONGO_64(b) (BYTE8_64(b) | BYTE7_64(b) | BYTE6_64(b) | BYTE5_64(b) | BYTE4_64(b) | BYTE3_64(b) | BYTE2_64(b) | BYTE1_64(b))

#else
# define MONGO_32(b) (b)
# define MONGO_64(b) (b)
#endif

void mcon_serialize_int(struct mcon_str *str, int num);
void mcon_serialize_int32(struct mcon_str *str, int num);
void mcon_serialize_int64(struct mcon_str *str, int64_t num);

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
