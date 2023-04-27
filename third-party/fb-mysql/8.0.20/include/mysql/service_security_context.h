/* Copyright (c) 2015, 2017, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MYSQL_SERVICE_SECURITY_CONTEXT
#define MYSQL_SERVICE_SECURITY_CONTEXT

/**
  @file include/mysql/service_security_context.h

  Definitions for the password validation service.

  @sa security_context_service_st
*/

#include "mysql/plugin.h"

#ifdef __cplusplus
class Security_context;
/** an opaque class reference hiding the actual security context object. */
#define MYSQL_SECURITY_CONTEXT Security_context *
#else
#define MYSQL_SECURITY_CONTEXT void *
#endif
typedef char my_svc_bool;

/**
  @ingroup group_ext_plugin_services

  This service provides functions for plugins and storage engines to
  manipulate the thread's security context.

  The service allows creation, copying, filling in by user accout and
  destruction of security context objects.
  It also allows getting and setting the security context for a thread.
  And it also allows reading and setting security context properties.

  The range of the above services allows plugins to inspect the security
  context they're running it, impersonate a user account of their choice
  (a.k.a. sudo in Unix) and craft a security context not related to an
  existing user account.

  No authentication is done in any of the above services. Authentication
  is specific to the media and does not belong to the security context,
  that's used mostly for authorization.

  Make sure you keep the original security context of a thread or restore
  it when done, as leaving a different security context active may lead to
  various kinds of problems.

  @sa Security_context, THD, MYSQL_SECURITY_CONTEXT
*/
extern "C" struct security_context_service_st {
  /**
    Retrieves a handle to the current security context for a thread.
    @sa ::thd_get_security_context
  */
  my_svc_bool (*thd_get_security_context)(MYSQL_THD,
                                          MYSQL_SECURITY_CONTEXT *out_ctx);
  /**
    Sets a new security context for a thread
    @sa ::thd_set_security_context
  */
  my_svc_bool (*thd_set_security_context)(MYSQL_THD,
                                          MYSQL_SECURITY_CONTEXT in_ctx);

  /**
    Creates a new security context
    @sa ::security_context_create
  */
  my_svc_bool (*security_context_create)(MYSQL_SECURITY_CONTEXT *out_ctx);
  /**
    Creates a new security context
    @sa ::security_context_create
  */
  my_svc_bool (*security_context_destroy)(MYSQL_SECURITY_CONTEXT);
  /**
    Creates a copy of a security context
    @sa ::security_context_copy
  */
  my_svc_bool (*security_context_copy)(MYSQL_SECURITY_CONTEXT in_ctx,
                                       MYSQL_SECURITY_CONTEXT *out_ctx);

  /**
    Fills in a security context with the attributes of a user account
    @sa ::security_context_lookup
  */
  my_svc_bool (*security_context_lookup)(MYSQL_SECURITY_CONTEXT ctx,
                                         const char *user, const char *host,
                                         const char *ip, const char *db);

  /**
    Retrieves the value for a named attribute of a security context
    @sa ::security_context_get_option
  */
  my_svc_bool (*security_context_get_option)(MYSQL_SECURITY_CONTEXT,
                                             const char *name,
                                             void *inout_pvalue);
  /**
    Sets a new value for a named attribute of a security context
    @sa ::security_context_set_option
  */
  my_svc_bool (*security_context_set_option)(MYSQL_SECURITY_CONTEXT,
                                             const char *name, void *pvalue);
} * security_context_service;

#ifdef MYSQL_DYNAMIC_PLUGIN

#define thd_get_security_context(_THD, _CTX) \
  security_context_service->thd_get_security_context(_THD, _CTX)
#define thd_set_security_context(_THD, _CTX) \
  security_context_service->thd_set_security_context(_THD, _CTX)

#define security_context_create(_CTX) \
  security_context_service->security_context_create(_CTX)
#define security_context_destroy(_CTX) \
  security_context_service->security_context_destroy(_CTX)
#define security_context_copy(_CTX1, _CTX2) \
  security_context_service->security_context_copy(_CTX1, _CTX2)

#define security_context_lookup(_CTX, _U, _H, _IP, _DB) \
  security_context_service->security_context_lookup(_CTX, _U, _H, _IP, _DB)

#define security_context_get_option(_SEC_CTX, _NAME, _VALUE) \
  security_context_service->security_context_get_option(_SEC_CTX, _NAME, _VALUE)
#define security_context_set_option(_SEC_CTX, _NAME, _VALUE) \
  security_context_service->security_context_set_option(_SEC_CTX, _NAME, _VALUE)
#else
my_svc_bool thd_get_security_context(MYSQL_THD,
                                     MYSQL_SECURITY_CONTEXT *out_ctx);
my_svc_bool thd_set_security_context(MYSQL_THD, MYSQL_SECURITY_CONTEXT in_ctx);

my_svc_bool security_context_create(MYSQL_SECURITY_CONTEXT *out_ctx);
my_svc_bool security_context_destroy(MYSQL_SECURITY_CONTEXT ctx);
my_svc_bool security_context_copy(MYSQL_SECURITY_CONTEXT in_ctx,
                                  MYSQL_SECURITY_CONTEXT *out_ctx);

my_svc_bool security_context_lookup(MYSQL_SECURITY_CONTEXT ctx,
                                    const char *user, const char *host,
                                    const char *ip, const char *db);

my_svc_bool security_context_get_option(MYSQL_SECURITY_CONTEXT,
                                        const char *name, void *inout_pvalue);
my_svc_bool security_context_set_option(MYSQL_SECURITY_CONTEXT,
                                        const char *name, void *pvalue);
#endif /* !MYSQL_DYNAMIC_PLUGIN */

#endif /* !MYSQL_SERVICE_SECURITY_CONTEXT */
