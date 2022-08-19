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

#ifndef SECURITY_CONTEXT_IMP_H
#define SECURITY_CONTEXT_IMP_H

#include <mysql/components/service_implementation.h>
#include <mysql/components/services/security_context.h>

/**
  An implementation of security_context service methods
*/
class mysql_security_context_imp {
 public:
  /**
    Gets the security context for the thread.

    @sa mysql_thd_security_context::get()
  */
  static DEFINE_BOOL_METHOD(get,
                            (void *_thd, Security_context_handle *out_ctx));

  /**
    Sets a new security context for the thread.

    @sa mysql_thd_security_context::set()
  */
  static DEFINE_BOOL_METHOD(set, (void *_thd, Security_context_handle in_ctx));

  /**
    Creates a new security context and initializes it with the defaults
    (no access, no user etc).

    @sa mysql_security_context_factory::create()
  */
  static DEFINE_BOOL_METHOD(create, (Security_context_handle * out_ctx));

  /**
    Deallocates a security context.

    @sa mysql_security_context_factory::destroy()
  */
  static DEFINE_BOOL_METHOD(destroy, (Security_context_handle ctx));

  /**
    Duplicates a security context.

    @sa mysql_security_context_factory::copy()
  */
  static DEFINE_BOOL_METHOD(copy, (Security_context_handle in_ctx,
                                   Security_context_handle *out_ctx));

  /**
    Looks up in the defined user accounts.

    @sa mysql_account_database_security_context_lookup::lookup()
  */
  static DEFINE_BOOL_METHOD(lookup,
                            (Security_context_handle ctx, const char *user,
                             const char *host, const char *ip, const char *db));

  /**
    Reads a named security context attribute and retuns its value.

    @sa mysql_security_context_options::get()
  */
  static DEFINE_BOOL_METHOD(get, (Security_context_handle ctx, const char *name,
                                  void *inout_pvalue));

  /**
    Sets a value for a named security context attribute

    @sa mysql_security_context_options::set()
  */
  static DEFINE_BOOL_METHOD(set, (Security_context_handle ctx, const char *name,
                                  void *pvalue));
};
#endif /* SECURITY_CONTEXT_IMP_H */
