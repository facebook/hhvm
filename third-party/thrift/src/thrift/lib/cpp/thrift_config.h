/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#ifndef _LIB_CPP_THRIFT_CONFIG_H
#define _LIB_CPP_THRIFT_CONFIG_H 1

/* Define to 1 if you have the `clock_gettime' function. */
#if defined(__linux__) || defined(__FreeBSD__)
#ifndef THRIFT_HAVE_CLOCK_GETTIME
#define THRIFT_HAVE_CLOCK_GETTIME 1
#endif
#endif // defined(__linux__) || defined(__FreeBSD__)

/* Define to 1 if you have the `snappy' library (-lsnappy). */
#ifndef THRIFT_HAVE_LIBSNAPPY
#define THRIFT_HAVE_LIBSNAPPY 1
#endif

/* Version number of package */
#ifndef THRIFT_VERSION
#define THRIFT_VERSION "1.0"
#endif

/* once: _LIB_CPP_THRIFT_CONFIG_H */
#endif
