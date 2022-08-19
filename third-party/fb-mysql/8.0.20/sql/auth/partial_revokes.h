/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef PARTIAL_REVOKES_INCLUDED
#define PARTIAL_REVOKES_INCLUDED

#include <map>
#include <memory>
#include <set>

#include "map_helpers.h"
#include "memory_debugging.h"
#include "my_alloc.h"
#include "my_inttypes.h"
#include "my_sqlcommand.h"
#include "sql/auth/auth_common.h"
#include "sql/auth/auth_utility.h"
#include "sql/mem_root_allocator.h"

// Forward declarations
class THD;
class ACL_USER;
class Json_array;
class Json_object;
class Restrictions_aggregator;
extern MEM_ROOT global_acl_memory;

// Alias declarations
using db_revocations = mem_root_collation_unordered_map<std::string, ulong>;
using Db_access_map = std::map<std::string, unsigned long>;

/**
  Abstract class for ACL restrictions.
*/
class Abstract_restrictions {
 public:
  explicit Abstract_restrictions(MEM_ROOT *mem_root);
  virtual ~Abstract_restrictions();
  virtual bool is_empty() const = 0;
  virtual size_t size() const = 0;
  virtual void clear() = 0;

 protected:
  /** MEM_ROOT manager */
  Mem_root_base m_mem_root_base;
};

/**
  DB Restrictions representation in memory.
  It uses memroot based, collation aware map to store
  (\<dbname\>, \<restricted_access\>) mapping.

  Each object created in the MEM_ROOT has to be destroyed manually.
  It will be the client's responsibility that create the objects.

  It also provides functions to:
  - Manage DB restrictions
  - Status functions
  - Transformation of in memory db restrictions
*/
class DB_restrictions final : public Abstract_restrictions {
 public:
  DB_restrictions(MEM_ROOT *mem_root);
  virtual ~DB_restrictions() override;

  db_revocations &operator()(void) { return db_restrictions(); }
  DB_restrictions(const DB_restrictions &restrictions);
  DB_restrictions(DB_restrictions &&restrictions) = delete;
  DB_restrictions &operator=(const DB_restrictions &restrictions);
  DB_restrictions &operator=(DB_restrictions &&restrictions);
  bool operator==(const DB_restrictions &restrictions) const;
  void add(const std::string &db_name, const ulong revoke_privs);
  void add(const DB_restrictions &restrictions);
  bool add(const Json_object &json_object);

  void remove(const std::string &db_name, const ulong revoke_privs);
  void remove(const ulong revoke_privs);

  bool find(const std::string &db_name, ulong &access) const;
  bool is_empty() const override;
  bool is_not_empty() const;
  size_t size() const override;
  void clear() override;
  void get_as_json(Json_array &restrictions_array) const;
  const db_revocations &get() const { return m_restrictions; }
  bool has_more_restrictions(const DB_restrictions &, ulong) const;

 private:
  db_revocations &db_restrictions() { return m_restrictions; }
  void remove(const ulong remove_restrictions, ulong &restrictions_mask) const
      noexcept;

 private:
  /** Database restrictions */
  db_revocations m_restrictions;
};

/**
  Container of all restrictions for a given user.

  Each object created in the MEM_ROOT has to be destroyed manually.
  It will be the client's responsibility that create the objects.
*/
class Restrictions {
 public:
  explicit Restrictions(MEM_ROOT *mem_root);

  Restrictions(const Restrictions &);
  Restrictions(Restrictions &&);
  Restrictions &operator=(const Restrictions &);
  Restrictions &operator=(Restrictions &&);
  bool has_more_db_restrictions(const Restrictions &, ulong);

  ~Restrictions();

  const DB_restrictions &db() const;
  void set_db(const DB_restrictions &db_restrictions);
  void clear_db();
  bool is_empty() const;

 private:
  /** Database restrictions */
  DB_restrictions m_db_restrictions;
};

/**
  Factory class that solely creates an object of type Restrictions_aggregator.

  - The concrete implemenations of Restrictions_aggregator cannot be created
    directly since their constructors are private. This class is declared as
    friend in those concrete implementations.
  - It also records the CURRENT_USER in the binlog so that partial_revokes can
    be executed on slave with context of current user
*/
class Restrictions_aggregator_factory {
 public:
  static std::unique_ptr<Restrictions_aggregator> create(
      THD *thd, const ACL_USER *acl_user, const char *db, const ulong rights,
      bool is_grant_revoke_all_on_db);

  static std::unique_ptr<Restrictions_aggregator> create(
      const Auth_id &grantor, const Auth_id &grantee,
      const ulong grantor_access, const ulong grantee_access,
      const DB_restrictions &grantor_restrictions,
      const DB_restrictions &grantee_restrictions, const ulong required_access,
      Db_access_map *db_map);

 private:
  static Auth_id fetch_grantor(const Security_context *sctx);
  static Auth_id fetch_grantee(const ACL_USER *acl_user);
  static ulong fetch_grantor_db_access(THD *thd, const char *db);
  static ulong fetch_grantee_db_access(THD *thd, const ACL_USER *acl_user,
                                       const char *db);
  static void fetch_grantor_access(const Security_context *sctx, const char *db,
                                   ulong &global_access,
                                   Restrictions &restrictions);
  static void fetch_grantee_access(const ACL_USER *grantee, ulong &access,
                                   Restrictions &restrictions);
};

/**
  Base class to perform aggregation of two restriction lists

  Aggregation is required if all of the following requirements are met:
  1. Partial revocation feature is enabled
  2. GRANT/REVOKE operation
  3. Either grantor or grantee or both have restrictions associated with them

  Task of the aggregator is to evaluate updates required for grantee's
  restriction. Based on restrictions associated with grantor/grantee:
  A. Add additional restrictions
     E.g. - GRANT of a new privileges by a grantor who has restrictions for
            privileges being granted
          - Creation of restrictions through REVOKE
  B. Remove some restrictions
     E.g. - GRANT of existing privileges by a grantor without restrictions
          - REVOKE of existing privileges

*/
class Restrictions_aggregator {
 public:
  virtual ~Restrictions_aggregator();

  /* interface methods which derived classes havee to implement */
  virtual bool generate(Abstract_restrictions &restrictions) = 0;
  virtual bool find_if_require_next_level_operation(ulong &rights) const = 0;

 protected:
  Restrictions_aggregator(const Auth_id &grantor, const Auth_id grantee,
                          const ulong grantor_global_access,
                          const ulong grantee_global_access,
                          const ulong requested_access);
  Restrictions_aggregator(const Restrictions_aggregator &) = delete;
  Restrictions_aggregator &operator=(const Restrictions_aggregator &) = delete;
  Restrictions_aggregator(const Restrictions_aggregator &&) = delete;
  Restrictions_aggregator &operator=(const Restrictions_aggregator &&) = delete;

  enum class Status { Error, Warning, Validated, Aggregated, No_op };

  /** Grantor information */
  const Auth_id m_grantor;

  /** Grantee information */
  const Auth_id m_grantee;

  /** Global static privileges of grantor */
  const ulong m_grantor_global_access;

  /** Global static privileges of grantee */
  const ulong m_grantee_global_access;

  /** Privileges that are being granted or revoked */
  const ulong m_requested_access;

  /** Internal status of aggregation process */
  Status m_status;
};

/**
  Restriction aggregator for database restrictions.
  An umbrella class to cover common methods.
  This is ultimately used for privilege aggregation
  in case of GRANT/REVOKE of database level privileges.
*/
class DB_restrictions_aggregator : public Restrictions_aggregator {
 public:
  bool generate(Abstract_restrictions &restrictions) override;

 protected:
  using Status = Restrictions_aggregator::Status;
  DB_restrictions_aggregator(const Auth_id &grantor, const Auth_id grantee,
                             const ulong grantor_global_access,
                             const ulong grantee_global_access,
                             const DB_restrictions &grantor_restrictions,
                             const DB_restrictions &grantee_restrictions,
                             const ulong requested_access,
                             const Security_context *sctx);
  bool find_if_require_next_level_operation(ulong &rights) const override;

  /* Helper methods and members for derived classes */

  bool check_db_access_and_restrictions_collision(
      const ulong grantee_db_access, const ulong grantee_restrictions,
      const std::string &db_name) noexcept;
  void set_if_db_level_operation(const ulong requested_access,
                                 const ulong restrictions_mask) noexcept;
  enum class SQL_OP { SET_ROLE, GLOBAL_GRANT };
  void aggregate_restrictions(SQL_OP sql_op, const Db_access_map *m_db_map,
                              DB_restrictions &restrictions);
  ulong get_grantee_db_access(const std::string &db_name) const;
  void get_grantee_db_access(const std::string &db_name, ulong &access) const;

  /** Privileges that needs to be checked further through DB grants */
  ulong m_privs_not_processed = 0;

  /** Database restrictions for grantor */
  DB_restrictions m_grantor_rl;

  /** Database restrictions for grantee */
  DB_restrictions m_grantee_rl;

  /** Security context of the current user */
  const Security_context *m_sctx;

 private:
  virtual Status validate() = 0;
  virtual void aggregate(DB_restrictions &restrictions) = 0;
};

/**
  Database restriction aggregator for SET ROLE statement.
*/
class DB_restrictions_aggregator_set_role final
    : public DB_restrictions_aggregator {
  DB_restrictions_aggregator_set_role(
      const Auth_id &grantor, const Auth_id grantee,
      const ulong grantor_global_access, const ulong grantee_global_access,
      const DB_restrictions &grantor_restrictions,
      const DB_restrictions &grantee_restrictions, const ulong requested_access,
      Db_access_map *db_map);

  Status validate() override;
  void aggregate(DB_restrictions &db_restrictions) override;
  friend class Restrictions_aggregator_factory;

 private:
  Db_access_map *m_db_map;
};

/**
  Restriction aggregator for GRANT statement for GLOBAL privileges.
*/
class DB_restrictions_aggregator_global_grant final
    : public DB_restrictions_aggregator {
  DB_restrictions_aggregator_global_grant(
      const Auth_id &grantor, const Auth_id grantee,
      const ulong grantor_global_access, const ulong grantee_global_access,
      const DB_restrictions &grantor_restrictions,
      const DB_restrictions &grantee_restrictions, const ulong requested_access,
      const Security_context *sctx);

  Status validate() override;
  void aggregate(DB_restrictions &restrictions) override;
  friend class Restrictions_aggregator_factory;
};

class DB_restrictions_aggregator_global_revoke
    : public DB_restrictions_aggregator {
 protected:
  DB_restrictions_aggregator_global_revoke(
      const Auth_id &grantor, const Auth_id grantee,
      const ulong grantor_global_access, const ulong grantee_global_access,
      const DB_restrictions &grantor_restrictions,
      const DB_restrictions &grantee_restrictions, const ulong requested_access,
      const Security_context *sctx);
  Status validate_if_grantee_rl_not_empty();

 private:
  Status validate() override;
  void aggregate(DB_restrictions &restrictions) override;
  friend class Restrictions_aggregator_factory;
};

/**
  Restriction aggregator for REVOKE statement over GLOBAL privileges.
*/
class DB_restrictions_aggregator_global_revoke_all final
    : public DB_restrictions_aggregator_global_revoke {
  DB_restrictions_aggregator_global_revoke_all(
      const Auth_id &grantor, const Auth_id grantee,
      const ulong grantor_global_access, const ulong grantee_global_access,
      const DB_restrictions &grantor_restrictions,
      const DB_restrictions &grantee_restrictions, const ulong requested_access,
      const Security_context *sctx);
  Status validate() override;
  void aggregate(DB_restrictions &restrictions) override;
  friend class Restrictions_aggregator_factory;
};

/**
  Restriction aggregator for GRANT statement over database privileges.
*/
class DB_restrictions_aggregator_db_grant final
    : public DB_restrictions_aggregator {
  DB_restrictions_aggregator_db_grant(
      const Auth_id &grantor, const Auth_id grantee,
      const ulong grantor_global_access, const ulong grantee_global_access,
      const ulong grantor_db_access, const ulong grantee_db_access,
      const DB_restrictions &grantor_restrictions,
      const DB_restrictions &grantee_restrictions, const ulong requested_access,
      bool is_grant_all, const std::string &db_name,
      const Security_context *sctx);

  void aggregate(DB_restrictions &restrictions) override;
  Status validate() override;

  /** Aggregator needs to access class members */
  friend class Restrictions_aggregator_factory;

  /** Grantor's database privileges */
  const ulong m_grantor_db_access;

  /** Grantee's database privileges */
  const ulong m_grantee_db_access;

  /** Flag for GRANT ALL ON \<db\>.* TO ... */
  const bool m_is_grant_all;

  /** Target database of GRANT */
  const std::string m_db_name;
};

/**
  Restriction aggregator for REVOKE statement for database privileges.
*/
class DB_restrictions_aggregator_db_revoke final
    : public DB_restrictions_aggregator {
  DB_restrictions_aggregator_db_revoke(
      const Auth_id &grantor, const Auth_id grantee,
      const ulong grantor_global_access, const ulong grantee_global_access,
      const ulong grantor_db_access, const ulong grantee_db_access,
      const DB_restrictions &grantor_restrictions,
      const DB_restrictions &grantee_restrictions, const ulong requested_access,
      bool is_revoke_all, const std::string &db_name,
      const Security_context *sctx);

  void aggregate(DB_restrictions &restrictions) override;
  Status validate() override;

  /** Aggregator needs to access class members */
  friend class Restrictions_aggregator_factory;

  /** Grantor's database privileges */
  const ulong m_grantor_db_access;

  /** Grantee's database privileges */
  const ulong m_grantee_db_access;

  /** Flag for GRANT ALL ON \<db\>.* TO ... */
  const bool m_is_revoke_all;

  /** Target database of REVOKE */
  const std::string m_db_name;
};

#endif /* PARTIAL_REVOKES_INCLUDED */
