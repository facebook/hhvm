/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_LIBMEMCACHED_PORTABILITY_H_
#define incl_HPHP_EXT_LIBMEMCACHED_PORTABILITY_H_

#include <libmemcached/memcached.h>

#if defined(LIBMEMCACHED_VERSION_HEX) && LIBMEMCACHED_VERSION_HEX >= 0x01000017

#define LMCD_SERVER_BY_KEY_INSTANCE_TYPE   const memcached_instance_st*
#define LMCD_SERVER_CALLBACK_INSTANCE_TYPE const memcached_instance_st*
#define LMCD_SERVER_POSITION_INSTANCE_TYPE const memcached_instance_st*

#define LMCD_SERVER_MAJOR_VERSION(server) memcached_server_major_version(server)
#define LMCD_SERVER_MINOR_VERSION(server) memcached_server_minor_version(server)
#define LMCD_SERVER_MICRO_VERSION(server) memcached_server_micro_version(server)
#define LMCD_SERVER_HOSTNAME(server)      memcached_server_name(server)
#define LMCD_SERVER_PORT(server)          memcached_server_port(server)

#elif defined(LIBMEMCACHED_VERSION_HEX) && LIBMEMCACHED_VERSION_HEX>=0x01000009

#define LMCD_SERVER_BY_KEY_INSTANCE_TYPE   const memcached_server_instance_st
#define LMCD_SERVER_CALLBACK_INSTANCE_TYPE memcached_server_instance_st
#define LMCD_SERVER_POSITION_INSTANCE_TYPE memcached_server_instance_st

#define LMCD_SERVER_MAJOR_VERSION(server) memcached_server_major_version(server)
#define LMCD_SERVER_MINOR_VERSION(server) memcached_server_minor_version(server)
#define LMCD_SERVER_MICRO_VERSION(server) memcached_server_micro_version(server)
#define LMCD_SERVER_HOSTNAME(server)      memcached_server_name(server)
#define LMCD_SERVER_PORT(server)          memcached_server_port(server)

#else

#define LMCD_SERVER_BY_KEY_INSTANCE_TYPE   const memcached_server_instance_st
#define LMCD_SERVER_CALLBACK_INSTANCE_TYPE memcached_server_instance_st
#define LMCD_SERVER_POSITION_INSTANCE_TYPE memcached_server_instance_st

#define LMCD_SERVER_MAJOR_VERSION(server) server->major_version
#define LMCD_SERVER_MINOR_VERSION(server) server->minor_version
#define LMCD_SERVER_MICRO_VERSION(server) server->micro_version
#define LMCD_SERVER_HOSTNAME(server)      server->hostname
#define LMCD_SERVER_PORT(server)          server->port

#define LMCD_SERVER_QUERY_INCLUDES_WEIGHT

#endif

#endif // incl_HPHP_EXT_LIBMEMCACHED_PORTABILITY_H_
