/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SECURITY_CONTEXT_H
#define SECURITY_CONTEXT_H

#include <mysql/components/service.h>
#include <mysql/components/services/dynamic_privilege.h>

/**
  Below are the set of services provides methods for components to manipulate
  the thread's security context.
  * mysql_thd_security_context
  * mysql_security_context_factory
  * mysql_account_database_security_context_lookup
  * mysql_security_context_options

  These services allows creation, copying, filling in by user accout and
  destruction of security context objects. It also allows getting and setting
  the security context for a thread. And it also allows reading and setting
  security context properties.

  The range of the above services allows components to inspect the security
  context they're running it, impersonate a user account of their choice
  (a.k.a. sudo in Unix) and craft a security context not related to an
  existing user account.

  No authentication is done in any of the above services. Authentication is
  specific to the media and does not belong to the security context, that's
  used mostly for authorization.

  Make sure you keep the original security context of a thread or restore it
  when done, as leaving a different security context active may lead to various
  kinds of problems.
*/

/* manipulates the THD relationship to the security context */
BEGIN_SERVICE_DEFINITION(mysql_thd_security_context)
/**
  Gets the security context for the thread.

  @param[in] _thd      The thread to get the context from
  @param[out] out_ctx  placeholder for the security context handle
  @retval true    failure
  @retval false   success
*/
DECLARE_BOOL_METHOD(get, (void *_thd, Security_context_handle *out_ctx));

/**
  Sets a new security context for the thread.

  @param[in] _thd    The thread to set the context to
  @param[in] in_ctx  The handle of the new security context
  @retval true    failure
  @retval false   success
*/
DECLARE_BOOL_METHOD(set, (void *_thd, Security_context_handle in_ctx));
END_SERVICE_DEFINITION(mysql_thd_security_context)

/* factory methods: allocate, deallocate, copy */
BEGIN_SERVICE_DEFINITION(mysql_security_context_factory)
/**
  Creates a new security context and initializes it with the defaults
  (no access, no user etc).

  @param[out] out_ctx  placeholder for the newly created security context
                       handle
  @retval true    failure
  @retval false   success
*/
DECLARE_BOOL_METHOD(create, (Security_context_handle * out_ctx));

/**
  Deallocates a security context.

  @param[in] ctx  The handle of the security context to destroy
  @retval true    failure
  @retval false   success
*/
DECLARE_BOOL_METHOD(destroy, (Security_context_handle ctx));

/**
  Duplicates a security context.

  @param[in]  in_ctx  The handle of the security context to copy
  @param[out] out_ctx  placeholder for the handle of the copied
                       security context
  @retval true    failure
  @retval false   success
*/
DECLARE_BOOL_METHOD(copy, (Security_context_handle in_ctx,
                           Security_context_handle *out_ctx));
END_SERVICE_DEFINITION(mysql_security_context_factory)

/* interact with the user account database */
BEGIN_SERVICE_DEFINITION(mysql_account_database_security_context_lookup)
/**
  Looks up in the defined user accounts an account based on
  the user\@host[ip] combo supplied and checks if the user
  has access to the database requested.
  The lookup is done in exactly the same way as at login time.
  The new security context need to checkout additional privileges using
  the checkout_acl method.
  @param[in]  ctx   The handle of the security context to update
  @param[in]  user  The user name to look up
  @param[in]  host  The host name to look up
  @param[in]  ip    The ip of the incoming connection
  @param[in]  db    The database to check access to
  @retval true    failure
  @retval false   success
*/
DECLARE_BOOL_METHOD(lookup, (Security_context_handle ctx, const char *user,
                             const char *host, const char *ip, const char *db));
END_SERVICE_DEFINITION(mysql_account_database_security_context_lookup)

/* options */
BEGIN_SERVICE_DEFINITION(mysql_security_context_options)
/**
  Reads a named security context attribute and retuns its value.
  Currently defined names are:

  - user  MYSQL_LEX_CSTRING *  login user (a.k.a. the user's part of USER())
  - host  MYSQL_LEX_CSTRING *  login host (a.k.a. the host's part of USER())
  - ip    MYSQL_LEX_CSTRING *  login client ip
  - host_or_ip MYSQL_LEX_CSTRING *  host, if present, ip if not.
  - priv_user  MYSQL_LEX_CSTRING *  authenticated user
               (a.k.a. the user's part of CURRENT_USER())
  - priv_host  MYSQL_LEX_CSTRING * authenticated host
               (a.k.a. the host's part of CURRENT_USER())
  - proxy_user  MYSQL_LEX_CSTRING *  the proxy user used in authenticating

  - privilege_super DECLARE_BOOL_METHOD *  1 if the user account has
    supper privilege, 0 otherwise
  - privilege_execute DECLARE_BOOL_METHOD *  1 if the user account has
    execute privilege, 0 otherwise

  @param[in]  ctx   The handle of the security context to read from
  @param[in]  name  The option name to read
  @param[out] inout_pvalue The value of the option. Type depends on the name.
  @retval true    failure
  @retval false   success
*/
DECLARE_BOOL_METHOD(get, (Security_context_handle ctx, const char *name,
                          void *inout_pvalue));

/**
  Sets a value for a named security context attribute
  Currently defined names are:

  - user  MYSQL_LEX_CSTRING *  login user (a.k.a. the user's part of USER())
  - host  MYSQL_LEX_CSTRING *  login host (a.k.a. the host's part of USER())
  - ip    MYSQL_LEX_CSTRING *  login client ip
  - priv_user  MYSQL_LEX_CSTRING *  authenticated user
               (a.k.a. the user's part of CURRENT_USER())
  - priv_host  MYSQL_LEX_CSTRING *  authenticated host
               (a.k.a. the host's part of CURRENT_USER())
  - proxy_user MYSQL_LEX_CSTRING *  the proxy user used in authenticating

  - privilege_super  DECLARE_BOOL_METHOD * 1 if the user account has
                     supper privilege, 0 otherwise
  - privilege_execute DECLARE_BOOL_METHOD *  1 if the user account has
    execute privilege, 0 otherwise

  @param[in]  ctx   The handle of the security context to set into
  @param[in]  name  The option name to set
  @param[in]  pvalue The value of the option. Type depends on the name.
  @retval true    failure
  @retval false   success
*/
DECLARE_BOOL_METHOD(set, (Security_context_handle ctx, const char *name,
                          void *pvalue));
END_SERVICE_DEFINITION(mysql_security_context_options)

#endif /* SECURITY_CONTEXT_H */
