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

#include "security_context_imp.h"

#include <mysql/components/minimal_chassis.h>
#include "sql/auth/auth_acls.h"
#include "sql/current_thd.h"
#include "sql/sql_class.h"
#include "sql/sql_thd_internal_api.h"  // create_thd

/**
  Gets the security context for the thread.

  @param[in] _thd      The thread to get the context from
  @param[out] out_ctx  placeholder for the security context handle
  @retval true    failure
  @retval false   success
*/
DEFINE_BOOL_METHOD(mysql_security_context_imp::get,
                   (void *_thd, Security_context_handle *out_ctx)) {
  THD *thd = reinterpret_cast<THD *>(_thd);

  if (!out_ctx) return true;

  try {
    *out_ctx =
        reinterpret_cast<Security_context_handle>(thd->security_context());
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Sets a new security context for the thread.

  @param[in] _thd    The thread to set the context to
  @param[in] in_ctx  The handle of the new security context
  @retval true    failure
  @retval false   success
*/
DEFINE_BOOL_METHOD(mysql_security_context_imp::set,
                   (void *_thd, Security_context_handle in_ctx)) {
  THD *thd = reinterpret_cast<THD *>(_thd);

  if (!in_ctx) return true;

  try {
    Security_context *in_sctx = reinterpret_cast<Security_context *>(in_ctx);
    if (in_sctx) {
      thd->set_security_context(in_sctx);
      // Turn ON the flag in THD iff the user is granted SYSTEM_USER privilege
      set_system_user_flag(thd);
    }
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Creates a new security context and initializes it with the defaults
  (no access, no user etc).

  @param[out] out_ctx  placeholder for the newly created security context
                       handle
  @retval true    failure
  @retval false   success
*/
DEFINE_BOOL_METHOD(mysql_security_context_imp::create,
                   (Security_context_handle * out_ctx)) {
  try {
    *out_ctx =
        reinterpret_cast<Security_context_handle>(new Security_context());
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Deallocates a security context.

  @param[in] ctx  The handle of the security context to destroy
  @retval true    failure
  @retval false   success
*/
DEFINE_BOOL_METHOD(mysql_security_context_imp::destroy,
                   (Security_context_handle ctx)) {
  try {
    delete reinterpret_cast<Security_context *>(ctx);
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Duplicates a security context.

  @param[in]  in_ctx  The handle of the security context to copy
  @param[out] out_ctx  placeholder for the handle of the copied
                       security context
  @retval true    failure
  @retval false   success
*/
DEFINE_BOOL_METHOD(mysql_security_context_imp::copy,
                   (Security_context_handle in_ctx,
                    Security_context_handle *out_ctx)) {
  try {
    if (out_ctx) {
      *out_ctx =
          reinterpret_cast<Security_context_handle>(new Security_context());
      if (in_ctx && *out_ctx)
        *out_ctx = reinterpret_cast<Security_context_handle>(in_ctx);
      else
        return true;
      return false;
    }
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

/**
  Looks up in the defined user accounts an account based on
  the user\@host[ip] combo supplied and checks if the user
  has access to the database requested.
  The lookup is done in exactly the same way as at login time.
  The new security context need to checkout additional privileges using
  the checkout_acl method.
  @param[in]  ctx   The handle of the security context to update
  @param[in]  user  The user name to look up, the name has to be in utf8 charset
  @param[in]  host  The host name to look up, the name has to be in utf8 charset
  @param[in]  ip    The ip of the incoming connection
  @param[in]  db    The database to check access to
  @retval true    failure
  @retval false   success
*/
DEFINE_BOOL_METHOD(mysql_security_context_imp::lookup,
                   (Security_context_handle ctx, const char *user,
                    const char *host, const char *ip, const char *db)) {
  try {
    THD *tmp_thd = nullptr;
    bool retval;
    if (current_thd == nullptr) {
      tmp_thd = create_thd(false, true, false, PSI_NOT_INSTRUMENTED);
      if (!tmp_thd) return true;
    }

    retval = acl_getroot(tmp_thd ? tmp_thd : current_thd,
                         reinterpret_cast<Security_context *>(ctx), user, host,
                         ip, db)
                 ? true
                 : false;

    if (tmp_thd) {
      destroy_thd(tmp_thd);
      tmp_thd = nullptr;
    }
    return retval;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

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

  @param[in]  ctx_h   The handle of the security context to read from
  @param[in]  name  The option name to read
  @param[out] inout_pvalue The value of the option. Type depends on the name.
  @retval true    failure
  @retval false   success
*/
DEFINE_BOOL_METHOD(mysql_security_context_imp::get,
                   (Security_context_handle ctx_h, const char *name,
                    void *inout_pvalue)) {
  try {
    Security_context *ctx = reinterpret_cast<Security_context *>(ctx_h);
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
        *((bool *)inout_pvalue) = checked ? true : false;
      } else if (!strcmp(name, "privilege_execute")) {
        bool checked = ctx->check_access(EXECUTE_ACL);
        *((bool *)inout_pvalue) = checked ? true : false;
      } else
        return true; /* invalid option */
    }
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}

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

  @param[in]  ctx_h   The handle of the security context to set into
  @param[in]  name  The option name to set
  @param[in]  pvalue The value of the option. Type depends on the name.
  @retval true    failure
  @retval false   success
*/
DEFINE_BOOL_METHOD(mysql_security_context_imp::set,
                   (Security_context_handle ctx_h, const char *name,
                    void *pvalue)) {
  try {
    Security_context *ctx = reinterpret_cast<Security_context *>(ctx_h);
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
      char value = *(char *)pvalue;
      if (value)
        ctx->set_master_access(ctx->master_access() | (SUPER_ACL),
                               ctx->restrictions());
      else
        ctx->set_master_access(ctx->master_access() & ~(SUPER_ACL),
                               ctx->restrictions());
    } else if (!strcmp(name, "privilege_execute")) {
      char value = *(char *)pvalue;
      if (value)
        ctx->set_master_access(ctx->master_access() | (EXECUTE_ACL),
                               ctx->restrictions());
      else
        ctx->set_master_access(ctx->master_access() & ~(EXECUTE_ACL),
                               ctx->restrictions());
    } else
      return true; /* invalid option */
    return false;
  } catch (...) {
    mysql_components_handle_std_exception(__func__);
  }
  return true;
}
