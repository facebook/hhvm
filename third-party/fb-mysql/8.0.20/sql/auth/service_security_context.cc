/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_SECURITY_CTX_SERVICE_INCLUDED
#define SQL_SECURITY_CTX_SERVICE_INCLUDED

#include <string.h>

#include "lex_string.h"
#include "my_inttypes.h"
#include "mysql/mysql_lex_string.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_security_context.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"
#include "sql/protocol_classic.h"
#include "sql/sql_class.h"
#include "sql/sql_thd_internal_api.h"  // create_thd

#define MY_SVC_TRUE 1
#define MY_SVC_FALSE 0

/**
  Gets the security context for the thread.

  @param[in]  _thd      The thread to get the context from
  @param[out] out_ctx  placeholder for the security context handle
  @retval true    failure
  @retval false   success

  @sa security_context_service_st
*/
my_svc_bool thd_get_security_context(MYSQL_THD _thd,
                                     MYSQL_SECURITY_CONTEXT *out_ctx) {
  THD *thd = dynamic_cast<THD *>(_thd);

  try {
    if (out_ctx)
      *out_ctx = thd->security_context();
    else
      return MY_SVC_TRUE;
    return MY_SVC_FALSE;
  } catch (...) {
    return MY_SVC_FALSE;
  }
}

/**
  Sets a new security context for the thread.

  @param[in]  _thd  The thread to set the context to
  @param[in]  in_ctx  The handle of the new security context
  @retval true    failure
  @retval false   success

  @sa security_context_service_st
*/
my_svc_bool thd_set_security_context(MYSQL_THD _thd,
                                     MYSQL_SECURITY_CONTEXT in_ctx) {
  THD *thd = dynamic_cast<THD *>(_thd);
  try {
    if (in_ctx) {
      thd->set_security_context(in_ctx);
      // Turn ON the flag in THD iff the user is granted SYSTEM_USER privilege
      set_system_user_flag(thd);
    }
    return MY_SVC_FALSE;
  } catch (...) {
    return MY_SVC_TRUE;
  }
}

/**
  Creates a new security context and initializes it with the defaults
  (no access, no user etc).

  @param[out] out_ctx  placeholder for the newly created security context handle
  @retval true    failure
  @retval false   success

  @sa security_context_service_st
*/
my_svc_bool security_context_create(MYSQL_SECURITY_CONTEXT *out_ctx) {
  try {
    if (out_ctx) *out_ctx = new Security_context();
    return MY_SVC_FALSE;
  } catch (...) {
    return MY_SVC_TRUE;
  }
}

/**
  Deallocates a security context.

  @param[in]  ctx  The handle of the security context to destroy
  @retval true    failure
  @retval false   success

  @sa security_context_service_st
*/
my_svc_bool security_context_destroy(MYSQL_SECURITY_CONTEXT ctx) {
  try {
    delete ctx;
    return MY_SVC_FALSE;
  } catch (...) {
    return MY_SVC_TRUE;
  }
}

/**
  Duplicates a security context.

  @param[in]  in_ctx  The handle of the security context to copy
  @param[out] out_ctx  placeholder for the handle of the copied security context
  @retval true    failure
  @retval false   success

  @sa security_context_service_st
*/
my_svc_bool security_context_copy(MYSQL_SECURITY_CONTEXT in_ctx,
                                  MYSQL_SECURITY_CONTEXT *out_ctx) {
  try {
    if (out_ctx) {
      *out_ctx = new Security_context();
      if (in_ctx) **out_ctx = *in_ctx;
    }

    return MY_SVC_FALSE;
  } catch (...) {
    return MY_SVC_TRUE;
  }
}

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

  @sa security_context_service_st
*/
my_svc_bool security_context_lookup(MYSQL_SECURITY_CONTEXT ctx,
                                    const char *user, const char *host,
                                    const char *ip, const char *db) {
  THD *tmp_thd = nullptr;
  bool retval;
  if (current_thd == nullptr) {
    tmp_thd = create_thd(false, true, false, PSI_NOT_INSTRUMENTED);
    if (!tmp_thd) return true;
  }

  retval = acl_getroot(tmp_thd ? tmp_thd : current_thd, ctx, user, host, ip, db)
               ? true
               : false;

  if (tmp_thd) {
    destroy_thd(tmp_thd);
    tmp_thd = nullptr;
  }
  return retval;
}

/**
  Reads a named security context attribute and retuns its value.
  Currently defined names are:

  - user        MYSQL_LEX_CSTRING *  login user (a.k.a. the user's part of
  USER())
  - host        MYSQL_LEX_CSTRING *  login host (a.k.a. the host's part of
  USER())
  - ip          MYSQL_LEX_CSTRING *  login client ip
  - host_or_ip  MYSQL_LEX_CSTRING *  host, if present, ip if not.
  - priv_user   MYSQL_LEX_CSTRING *  authenticated user (a.k.a. the user's part
  of CURRENT_USER())
  - priv_host   MYSQL_LEX_CSTRING *  authenticated host (a.k.a. the host's part
  of CURRENT_USER())
  - proxy_user  MYSQL_LEX_CSTRING *  the proxy user used in authenticating

  - privilege_super   my_svc_bool *  1 if the user account has supper privilege,
  0 otherwise
  - privilege_execute my_svc_bool *  1 if the user account has execute
  privilege, 0 otherwise

  @param[in]  ctx   The handle of the security context to read from
  @param[in]  name  The option name to read
  @param[out] inout_pvalue The value of the option. Type depends on the name.
  @retval true    failure
  @retval false   success

  @sa security_context_service_st

*/
my_svc_bool security_context_get_option(MYSQL_SECURITY_CONTEXT ctx,
                                        const char *name, void *inout_pvalue) {
  try {
    if (inout_pvalue) {
      if (!strcmp(name, "user")) {
        *((MYSQL_LEX_CSTRING *)inout_pvalue) = ctx->user();
      } else if (!strcmp(name, "host")) {
        *((MYSQL_LEX_CSTRING *)inout_pvalue) = ctx->host();
      } else if (!strcmp(name, "ip")) {
        *((MYSQL_LEX_CSTRING *)inout_pvalue) = ctx->ip();
      } else if (!strcmp(name, "host_or_ip")) {
        *((MYSQL_LEX_CSTRING *)inout_pvalue) = ctx->host_or_ip();
      } else if (!strcmp(name, "priv_user")) {
        *((MYSQL_LEX_CSTRING *)inout_pvalue) = ctx->priv_user();
      } else if (!strcmp(name, "priv_host")) {
        *((MYSQL_LEX_CSTRING *)inout_pvalue) = ctx->priv_host();
      } else if (!strcmp(name, "proxy_user")) {
        *((MYSQL_LEX_CSTRING *)inout_pvalue) = ctx->proxy_user();
      } else if (!strcmp(name, "external_user")) {
        *((MYSQL_LEX_CSTRING *)inout_pvalue) = ctx->external_user();
      } else if (!strcmp(name, "privilege_super")) {
        bool checked = ctx->check_access(SUPER_ACL);
        *((my_svc_bool *)inout_pvalue) = checked ? MY_SVC_TRUE : MY_SVC_FALSE;
      } else if (!strcmp(name, "privilege_execute")) {
        bool checked = ctx->check_access(EXECUTE_ACL);
        *((my_svc_bool *)inout_pvalue) = checked ? MY_SVC_TRUE : MY_SVC_FALSE;
      } else
        return MY_SVC_TRUE; /* invalid option */
    }
    return MY_SVC_FALSE;
  } catch (...) {
    return MY_SVC_TRUE;
  }
}

/**
  Sets a value for a named security context attribute
  Currently defined names are:

  - user        MYSQL_LEX_CSTRING *  login user (a.k.a. the user's part of
  USER())
  - host        MYSQL_LEX_CSTRING *  login host (a.k.a. the host's part of
  USER())
  - ip          MYSQL_LEX_CSTRING *  login client ip
  - priv_user   MYSQL_LEX_CSTRING *  authenticated user (a.k.a. the user's part
  of CURRENT_USER())
  - priv_host   MYSQL_LEX_CSTRING *  authenticated host (a.k.a. the host's part
  of CURRENT_USER())
  - proxy_user  MYSQL_LEX_CSTRING *  the proxy user used in authenticating

  - privilege_super   my_svc_bool *  1 if the user account has supper privilege,
  0 otherwise
  - privilege_execute my_svc_bool *  1 if the user account has execute
  privilege, 0 otherwise

  @param[in]  ctx   The handle of the security context to set into
  @param[in]  name  The option name to set
  @param[in]  pvalue The value of the option. Type depends on the name.
  @retval true    failure
  @retval false   success

  @sa security_context_service_st
*/
my_svc_bool security_context_set_option(MYSQL_SECURITY_CONTEXT ctx,
                                        const char *name, void *pvalue) {
  try {
    if (!strcmp(name, "user")) {
      LEX_CSTRING *value = (LEX_CSTRING *)pvalue;
      ctx->assign_user(value->str, value->length);
    } else if (!strcmp(name, "host")) {
      LEX_CSTRING *value = (LEX_CSTRING *)pvalue;
      ctx->assign_host(value->str, value->length);
    } else if (!strcmp(name, "ip")) {
      LEX_CSTRING *value = (LEX_CSTRING *)pvalue;
      ctx->assign_ip(value->str, value->length);
    } else if (!strcmp(name, "priv_user")) {
      LEX_CSTRING *value = (LEX_CSTRING *)pvalue;
      ctx->assign_priv_user(value->str, value->length);
    } else if (!strcmp(name, "priv_host")) {
      LEX_CSTRING *value = (LEX_CSTRING *)pvalue;
      ctx->assign_priv_host(value->str, value->length);
    } else if (!strcmp(name, "proxy_user")) {
      LEX_CSTRING *value = (LEX_CSTRING *)pvalue;
      ctx->assign_proxy_user(value->str, value->length);
    } else if (!strcmp(name, "privilege_super")) {
      my_svc_bool value = *(my_svc_bool *)pvalue;
      if (value)
        ctx->set_master_access(ctx->master_access() | (SUPER_ACL));
      else
        ctx->set_master_access(ctx->master_access() & ~(SUPER_ACL));

    } else if (!strcmp(name, "privilege_execute")) {
      my_svc_bool value = *(my_svc_bool *)pvalue;
      if (value)
        ctx->set_master_access(ctx->master_access() | (EXECUTE_ACL));
      else
        ctx->set_master_access(ctx->master_access() & ~(EXECUTE_ACL));
    } else
      return MY_SVC_TRUE; /* invalid option */
    return MY_SVC_FALSE;
  } catch (...) {
    return MY_SVC_TRUE;
  }
}

#endif /* !SQL_SECURITY_CTX_SERVICE_INCLUDED */
