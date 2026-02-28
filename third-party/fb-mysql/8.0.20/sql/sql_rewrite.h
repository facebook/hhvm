/* Copyright (c) 2011, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef SQL_REWRITE_INCLUDED
#define SQL_REWRITE_INCLUDED

#include <set>
#include "my_sqlcommand.h"
#include "sql/table.h"

/* Forward declarations */
class THD;
class LEX_GRANT_AS;
/**
  Target types where the rewritten query will be added. Query rewrite might
  vary based on this type.
*/
enum class Consumer_type {
  TEXTLOG, /* General log, slow query log and audit log */
  BINLOG,  /* Binary logs */
  STDOUT   /* Standard output */
};

/**
  An interface to wrap the paramters required by specific Rewriter.
  Parameters required by specific Rewriter must be added in the concrete
  implementation.
  Clients need to wrap the parameters in specific concrete object.
*/
class Rewrite_params {
 protected:
  virtual ~Rewrite_params() {}
};

/**
  Wrapper object for user related parameters required by:
  SET PASSWORD|CREATE USER|ALTER USER statements.
*/
class User_params : public Rewrite_params {
 public:
  User_params(std::set<LEX_USER *> *users_set)
      : Rewrite_params(), users(users_set) {}
  std::set<LEX_USER *> *users;
};

/**
  Wrapper object for parameters required by SHOW CREATE USER statement.
*/
class Show_user_params : public Rewrite_params {
 public:
  Show_user_params(bool hide_password_hash, bool print_identified_with_as_hex)
      : Rewrite_params(),
        hide_password_hash(hide_password_hash),
        print_identified_with_as_hex_(print_identified_with_as_hex) {}
  bool hide_password_hash;
  bool print_identified_with_as_hex_;
};

/**
  Wrapper object for parameters required for GRANT statement.
*/
class Grant_params : public Rewrite_params {
 public:
  Grant_params(bool grant_as_specified, LEX_GRANT_AS *grant_as)
      : Rewrite_params(),
        grant_as_provided(grant_as_specified),
        grant_as_info(grant_as) {}
  bool grant_as_provided;
  LEX_GRANT_AS *grant_as_info;
};

/**
  Provides the default interface to rewrite the SQL statements to
  to obfuscate passwords.
  It either sets the thd->rewritten_query with a rewritten query,
  or clears it if no rewriting took place.
*/
void mysql_rewrite_query(THD *thd, Consumer_type type = Consumer_type::TEXTLOG,
                         Rewrite_params *params = nullptr);
/**
  Provides the default interface to rewrite the ACL query.
  If do_ps_instrument, it sets the thd->rewritten_query with
  a rewritten query.
*/
void mysql_rewrite_acl_query(THD *thd, String &rlb, Consumer_type type,
                             Rewrite_params *params = nullptr,
                             bool do_ps_instrument = true);

/**
  An abstract base class to enable the implementation of various query
  rewriters. It accepts a THD pointer and the intended target type where the
  query will to be written. It either sets the thd->rewritten_query with a
  rewritten query, or clears it if no rewriting took place. Concrete classes
  must implement the rewrite() method to rewrite the query. Despite concrete
  classes may accept additional parameters, it is recommended not to create
  their objects directly.
*/
class I_rewriter {
 public:
  /* Constructors and destructors */
  I_rewriter(THD *thd, Consumer_type type);
  virtual ~I_rewriter();
  /* Prohibit the copy of the object */
  I_rewriter(const I_rewriter &) = delete;
  const I_rewriter &operator=(const I_rewriter &) = delete;
  I_rewriter(const I_rewriter &&) = delete;
  const I_rewriter &operator=(const I_rewriter &&) = delete;
  /* Reset the previous consumer type before rewriting the query */
  void set_consumer_type(Consumer_type type);
  /* Return the current consumer type */
  Consumer_type consumer_type();
  /* Concrete classes must implement the logic to rewrite query here */
  virtual bool rewrite(String &rlb) const = 0;

 protected:
  THD *const m_thd;
  Consumer_type m_consumer_type;
};
/**
  Abstract base class to define the skeleton of rewriting the users, yet
  deferring some steps to the concrete classes. The implementation in specific
  steps might vary according to SQL or the consumer type.
*/
class Rewriter_user : public I_rewriter {
 protected:
  Rewriter_user(THD *thd, Consumer_type target_type);
  /*
    Provides the skeleton to rewrite the users. The actual user is  rewritten
    through the concrete implementation of private methods.
  */
  void rewrite_users(LEX *lex, String *str) const;
  /* Append the literal value <secret> to the str */
  void append_literal_secret(String *str) const;
  /* Append the password hash to the output string */
  virtual void append_auth_str(LEX_USER *lex, String *str) const;
  /* Append the authentication plugin name for the user */
  void append_plugin_name(const LEX_USER *user, String *str) const;
  /*
    Rewrites some of the user specific properties which are common to
    concrete classes.
  */
  virtual bool rewrite(String &rlb) const;
  /*
    Abstract method to be implemented by the concrete classes.
    The implementation methos should add the user authID, plugin info and
    auth str
  */
  virtual void append_user_auth_info(LEX_USER *user, bool comma,
                                     String *str) const = 0;
  /* Append the PASSWORD REUSE OPTIONS clause for users */
  virtual void rewrite_password_history(const LEX *lex, String *str) const = 0;
  /* Append the PASSWORD REUSE OPTIONS clause for users */
  virtual void rewrite_password_reuse(const LEX *lex, String *str) const = 0;

 private:
  /* Append the SSL OPTIONS clause for users */
  void rewrite_ssl_properties(const LEX *lex, String *str) const;
  /* Append the RESOURCES OPTIONS clause for users */
  void rewrite_user_resources(const LEX *lex, String *str) const;
  /* Append the ACCOUNT LOCK OPTIONS clause for users */
  void rewrite_account_lock(const LEX *lex, String *str) const;
  /* Append the PASSWORD EXPIRED OPTIONS clause for users */
  void rewrite_password_expired(const LEX *lex, String *str) const;
  /* Append the PASSWORD REQUIRE CURRENT clause for users */
  void rewrite_password_require_current(LEX *lex, String *str) const;
  /* Append FAILED_LOGIN_ATTEMPTS/PASSWORD_LOCK_TIME */
  void rewrite_account_lock_state(LEX *lex, String *str) const;
  /* Append the DEFAULT ROLE OPTIONS clause */
  void rewrite_default_roles(const LEX *lex, String *str) const;
};
/** Rewrites the CREATE USER statement. */
class Rewriter_create_user final : public Rewriter_user {
  using parent = Rewriter_user;

 public:
  Rewriter_create_user(THD *thd, Consumer_type type);
  bool rewrite(String &rlb) const override;

 private:
  void append_user_auth_info(LEX_USER *user, bool comma,
                             String *str) const override;
  void rewrite_password_history(const LEX *lex, String *str) const override;
  void rewrite_password_reuse(const LEX *lex, String *str) const override;
};
/** Rewrites the ALTER USER statement. */
class Rewriter_alter_user final : public Rewriter_user {
  using parent = Rewriter_user;

 public:
  Rewriter_alter_user(THD *thd, Consumer_type type = Consumer_type::TEXTLOG);
  bool rewrite(String &rlb) const override;

 private:
  void append_user_auth_info(LEX_USER *user, bool comma,
                             String *str) const override;
  void rewrite_password_history(const LEX *lex, String *str) const override;
  void rewrite_password_reuse(const LEX *lex, String *str) const override;
};
/** Rewrites the SHOW CREATE USER statement. */
class Rewriter_show_create_user final : public Rewriter_user {
  using parent = Rewriter_user;

 public:
  Rewriter_show_create_user(THD *thd, Consumer_type type,
                            Rewrite_params *params);
  bool rewrite(String &rlb) const override;

 protected:
  /* Append the password hash to the output string */
  virtual void append_auth_str(LEX_USER *lex, String *str) const override;

 private:
  void append_user_auth_info(LEX_USER *user, bool comma,
                             String *str) const override;
  void rewrite_password_history(const LEX *lex, String *str) const override;
  void rewrite_password_reuse(const LEX *lex, String *str) const override;
  Show_user_params *show_params_;
};
/** Rewrites the SET statement. */
class Rewriter_set : public I_rewriter {
 public:
  Rewriter_set(THD *thd, Consumer_type type);
  bool rewrite(String &rlb) const override;
};
/*
  Rewrites the SET PASSWORD statement
*/
class Rewriter_set_password final : public Rewriter_set {
  using parent = Rewriter_set;

 public:
  Rewriter_set_password(THD *thd, Consumer_type type, Rewrite_params *params);
  bool rewrite(String &rlb) const override;

 private:
  /* Name of the user whose password has to be changed */
  std::set<LEX_USER *> *m_users = nullptr;
};

/** Rewrites the GRANT statement. */
class Rewriter_grant final : public I_rewriter {
 public:
  Rewriter_grant(THD *thd, Consumer_type type, Rewrite_params *params);
  bool rewrite(String &rlb) const override;

 private:
  /* GRANT AS information */
  Grant_params *grant_params = nullptr;
};

/** Rewrites the CHANGE MASTER statement. */
class Rewriter_change_master final : public I_rewriter {
 public:
  Rewriter_change_master(THD *thd, Consumer_type);
  bool rewrite(String &rlb) const override;
};

/** Rewrites the START SLAVE statement. */
class Rewriter_slave_start final : public I_rewriter {
 public:
  Rewriter_slave_start(THD *thd, Consumer_type type);
  bool rewrite(String &rlb) const override;
};
/** Base class for SERVER OPTIONS related statement */
class Rewriter_server_option : public I_rewriter {
 public:
  Rewriter_server_option(THD *thd, Consumer_type type);

 protected:
  // Append the SERVER OPTIONS clause
  void mysql_rewrite_server_options(const LEX *lex, String *str) const;
};
/** Rewrites the CREATE SERVER statement. */
class Rewriter_create_server final : public Rewriter_server_option {
  using parent = Rewriter_server_option;

 public:
  Rewriter_create_server(THD *thd, Consumer_type type);
  bool rewrite(String &rlb) const override;
};
/** Rewrites the ALTER SERVER statement. */
class Rewriter_alter_server final : public Rewriter_server_option {
  using parent = Rewriter_server_option;

 public:
  Rewriter_alter_server(THD *thd, Consumer_type type);
  bool rewrite(String &rlb) const override;
};

/** Rewrites the PREPARE statement.*/
class Rewriter_prepare final : public I_rewriter {
 public:
  Rewriter_prepare(THD *thd, Consumer_type type);
  bool rewrite(String &rlb) const override;
};

/** Rewrites CLONE statement.*/
class Rewriter_clone final : public I_rewriter {
 public:
  Rewriter_clone(THD *thd, Consumer_type type);
  bool rewrite(String &rlb) const override;
};
#endif /* SQL_REWRITE_INCLUDED */
