/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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
/* Internals */

#ifndef AUTH_INTERNAL_INCLUDED
#define AUTH_INTERNAL_INCLUDED

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "mysql_time.h" /* MYSQL_TIME */
#include "sql/auth/auth_common.h"
#include "sql/auth/dynamic_privilege_table.h"
#include "sql/auth/partitioned_rwlock.h"
#include "sql/auth/user_table.h"
#include "sql/sql_audit.h"
#include "sql/table.h"
#include "violite.h" /* SSL_type */

class ACL_USER;
class ACL_PROXY_USER;
class GRANT_NAME;
class GRANT_TABLE;
class GRANT_COLUMN;
class Json_object;
class Json_wrapper;
class Restrictions;
struct TABLE;
class Rewrite_params;

typedef struct user_resources USER_RESOURCES;
void append_identifier(const THD *thd, String *packet, const char *name,
                       size_t length);
typedef std::map<std::string, unsigned long> Column_map;
struct Grant_table_aggregate {
  Grant_table_aggregate() : table_access(0l), cols(0l) {}
  ulong table_access;
  ulong cols;
  Column_map columns;
};
typedef std::map<std::string, unsigned long> SP_access_map;
typedef std::map<std::string, unsigned long> Db_access_map;
typedef std::map<std::string, Grant_table_aggregate> Table_access_map_storage;
class Table_access_map {
 public:
  Table_access_map() : m_thd(nullptr) {}

  typedef Table_access_map_storage::iterator iterator;
  typedef Table_access_map_storage::value_type value_type;
  typedef Table_access_map_storage::mapped_type mapped_type;
  mapped_type &operator[](const Table_access_map_storage::key_type &key) {
    return m_values[key];
  }
  iterator begin() { return m_values.begin(); }
  iterator end() { return m_values.end(); }
  iterator find(const Table_access_map_storage::key_type &key) {
    return m_values.find(key);
  }
  void set_thd(THD *thd) { m_thd = thd; }
  THD *get_thd() { return m_thd; }

 private:
  THD *m_thd;
  Table_access_map_storage m_values;
};
typedef std::unordered_set<std::string> Grant_acl_set;

std::string create_authid_str_from(const LEX_USER *user);
std::string create_authid_str_from(const ACL_USER *user);
std::string create_authid_str_from(const Auth_id_ref &user);
Auth_id_ref create_authid_from(const LEX_USER *user);
Auth_id_ref create_authid_from(const ACL_USER *user);

std::string get_one_priv(ulong &revoke_privs);
/* sql_authentication */
class Rsa_authentication_keys;
extern Rsa_authentication_keys *g_sha256_rsa_keys;
extern Rsa_authentication_keys *g_caching_sha2_rsa_keys;
extern char *caching_sha2_rsa_private_key_path;
extern char *caching_sha2_rsa_public_key_path;
extern bool caching_sha2_auto_generate_rsa_keys;
class Auth_id;
template <typename K, typename V>
class Map_with_rw_lock;
extern Map_with_rw_lock<Auth_id, uint> *unknown_accounts;

void optimize_plugin_compare_by_pointer(LEX_CSTRING *plugin_name);
bool auth_plugin_is_built_in(const char *plugin_name);
bool auth_plugin_supports_expiration(const char *plugin_name);

const ACL_internal_table_access *get_cached_table_access(
    GRANT_INTERNAL_INFO *grant_internal_info, const char *schema_name,
    const char *table_name);

/* sql_auth_cache */
ulong get_sort(uint count, ...);

/*sql_authentication */
bool sha256_rsa_auth_status();

/* sql_auth_cache */
void rebuild_check_host(void);
ACL_USER *find_acl_user(const char *host, const char *user, bool exact);
ACL_PROXY_USER *acl_find_proxy_user(const char *user, const char *host,
                                    const char *ip, char *authenticated_as,
                                    bool *proxy_used);
void acl_insert_proxy_user(ACL_PROXY_USER *new_value);

void acl_update_user(const char *user, const char *host, enum SSL_type ssl_type,
                     const char *ssl_cipher, const char *x509_issuer,
                     const char *x509_subject, USER_RESOURCES *mqh,
                     ulong privileges, const LEX_CSTRING &plugin,
                     const LEX_CSTRING &auth, const std::string &second_auth,
                     const MYSQL_TIME &password_change_time,
                     const LEX_ALTER &password_life, Restrictions &restrictions,
                     acl_table::Pod_user_what_to_update &what_to_update,
                     uint failed_login_attempts, int password_lock_time);
void acl_users_add_one(const char *user, const char *host,
                       enum SSL_type ssl_type, const char *ssl_cipher,
                       const char *x509_issuer, const char *x509_subject,
                       USER_RESOURCES *mqh, ulong privileges,
                       const LEX_CSTRING &plugin, const LEX_CSTRING &auth,
                       const LEX_CSTRING &second_auth,
                       const MYSQL_TIME &password_change_time,
                       const LEX_ALTER &password_life, bool add_role_vertex,
                       Restrictions &restrictions, uint failed_login_attempts,
                       int password_lock_time, THD *thd MY_ATTRIBUTE((unused)));
void acl_insert_user(THD *thd, const char *user, const char *host,
                     enum SSL_type ssl_type, const char *ssl_cipher,
                     const char *x509_issuer, const char *x509_subject,
                     USER_RESOURCES *mqh, ulong privileges,
                     const LEX_CSTRING &plugin, const LEX_CSTRING &auth,
                     const MYSQL_TIME &password_change_time,
                     const LEX_ALTER &password_life, Restrictions &restrictions,
                     uint failed_login_attempts, int password_lock_time);
void acl_update_proxy_user(ACL_PROXY_USER *new_value, bool is_revoke);
void acl_update_db(const char *user, const char *host, const char *db,
                   ulong privileges);
void acl_insert_db(const char *user, const char *host, const char *db,
                   ulong privileges);
bool update_sctx_cache(Security_context *sctx, ACL_USER *acl_user_ptr,
                       bool expired);

bool do_update_sctx(Security_context *sctx, LEX_USER *from_user);
void update_sctx(Security_context *sctx, LEX_USER *to_user);

void clear_and_init_db_cache();
bool acl_reload(THD *thd, bool mdl_locked);
bool grant_reload(THD *thd, bool mdl_locked);
void clean_user_cache();
bool set_user_salt(ACL_USER *acl_user);

/* sql_user_table */
ulong get_access(TABLE *form, uint fieldnr, uint *next_field);
int replace_db_table(THD *thd, TABLE *table, const char *db,
                     const LEX_USER &combo, ulong rights, bool revoke_grant);
int replace_proxies_priv_table(THD *thd, TABLE *table, const LEX_USER *user,
                               const LEX_USER *proxied_user,
                               bool with_grant_arg, bool revoke_grant);
int replace_column_table(THD *thd, GRANT_TABLE *g_t, TABLE *table,
                         const LEX_USER &combo, List<LEX_COLUMN> &columns,
                         const char *db, const char *table_name, ulong rights,
                         bool revoke_grant);
int replace_table_table(THD *thd, GRANT_TABLE *grant_table,
                        std::unique_ptr<GRANT_TABLE, Destroy_only<GRANT_TABLE>>
                            *deleted_grant_table,
                        TABLE *table, const LEX_USER &combo, const char *db,
                        const char *table_name, ulong rights, ulong col_rights,
                        bool revoke_grant);
int replace_routine_table(THD *thd, GRANT_NAME *grant_name, TABLE *table,
                          const LEX_USER &combo, const char *db,
                          const char *routine_name, bool is_proc, ulong rights,
                          bool revoke_grant);
int open_grant_tables(THD *thd, TABLE_LIST *tables, bool *transactional_tables);
void grant_tables_setup_for_open(
    TABLE_LIST *tables, thr_lock_type lock_type = TL_WRITE,
    enum_mdl_type mdl_type = MDL_SHARED_NO_READ_WRITE);

void acl_print_ha_error(int handler_error);
bool check_engine_type_for_acl_table(TABLE_LIST *tables, bool report_error);
bool log_and_commit_acl_ddl(THD *thd, bool transactional_tables,
                            std::set<LEX_USER *> *extra_users = nullptr,
                            Rewrite_params *rewrite_params = nullptr,
                            bool extra_error = false,
                            bool log_to_binlog = true);
void acl_notify_htons(THD *thd, enum_sql_command operation,
                      const List<LEX_USER> *users,
                      std::set<LEX_USER *> *rewrite_users = nullptr,
                      const List<LEX_CSTRING> *dynamic_privs = nullptr);

/* sql_authorization */
bool is_privileged_user_for_credential_change(THD *thd);
void rebuild_vertex_index(THD *thd);
void default_roles_init(void);
void default_roles_delete(void);
void roles_graph_init(void);
void roles_graph_delete(void);
void roles_init(void);
void roles_delete(void);
void dynamic_privileges_init(void);
void dynamic_privileges_delete(void);
bool grant_dynamic_privilege(const LEX_CSTRING &str_priv,
                             const LEX_CSTRING &str_user,
                             const LEX_CSTRING &str_host,
                             bool with_grant_option,
                             Update_dynamic_privilege_table &func);
bool revoke_dynamic_privilege(const LEX_CSTRING &str_priv,
                              const LEX_CSTRING &str_user,
                              const LEX_CSTRING &str_host,
                              Update_dynamic_privilege_table &update_table);
bool revoke_all_dynamic_privileges(const LEX_CSTRING &user,
                                   const LEX_CSTRING &host,
                                   Update_dynamic_privilege_table &func);
bool rename_dynamic_grant(const LEX_CSTRING &old_user,
                          const LEX_CSTRING &old_host,
                          const LEX_CSTRING &new_user,
                          const LEX_CSTRING &new_host,
                          Update_dynamic_privilege_table &update_table);
bool grant_grant_option_for_all_dynamic_privileges(
    const LEX_CSTRING &str_user, const LEX_CSTRING &str_host,
    Update_dynamic_privilege_table &func);
bool revoke_grant_option_for_all_dynamic_privileges(
    const LEX_CSTRING &str_user, const LEX_CSTRING &str_host,
    Update_dynamic_privilege_table &func);
bool grant_dynamic_privileges_to_auth_id(
    const Role_id &id, const std::vector<std::string> &priv_list);
void revoke_dynamic_privileges_from_auth_id(
    const Role_id &id, const std::vector<std::string> &priv_list);
bool operator==(const Role_id &a, const Auth_id_ref &b);
bool operator==(const Auth_id_ref &a, const Role_id &b);
bool operator==(const std::pair<const Role_id, const Role_id> &a,
                const Auth_id_ref &b);
bool operator==(const Role_id &a, const Role_id &b);
bool operator==(std::pair<const Role_id, std::pair<std::string, bool>> &a,
                const std::string &b);
typedef std::vector<std::pair<Role_id, bool>> List_of_granted_roles;

struct role_id_hash {
  std::size_t operator()(const Role_id &k) const {
    using std::hash;
    using std::size_t;
    using std::string;
    return ((hash<string>()(k.user()) ^ (hash<string>()(k.host()) << 1)) >> 1);
  }
};

typedef std::unordered_multimap<const Role_id, const Role_id, role_id_hash>
    Default_roles;
typedef std::map<std::string, bool> Dynamic_privileges;

void get_privilege_access_maps(
    ACL_USER *acl_user, const List_of_auth_id_refs *using_roles, ulong *access,
    Db_access_map *db_map, Db_access_map *db_wild_map,
    Table_access_map *table_map, SP_access_map *sp_map, SP_access_map *func_map,
    List_of_granted_roles *granted_roles, Grant_acl_set *with_admin_acl,
    Dynamic_privileges *dynamic_acl, Restrictions &restrictions);
bool clear_default_roles(THD *thd, TABLE *table,
                         const Auth_id_ref &user_auth_id,
                         std::vector<Role_id> *default_roles);
void get_granted_roles(LEX_USER *user, List_of_granted_roles *granted_roles);
bool drop_default_role_policy(THD *thd, TABLE *table,
                              const Auth_id_ref &default_role_policy,
                              const Auth_id_ref &user);
void revoke_role(THD *thd, ACL_USER *role, ACL_USER *user);
bool revoke_all_roles_from_user(THD *thd, TABLE *edge_table,
                                TABLE *defaults_table, LEX_USER *user);
bool drop_role(THD *thd, TABLE *edge_table, TABLE *defaults_table,
               const Auth_id_ref &authid_user);
bool modify_role_edges_in_table(THD *thd, TABLE *table,
                                const Auth_id_ref &from_user,
                                const Auth_id_ref &to_user,
                                bool with_admin_option, bool delete_option);
Auth_id_ref create_authid_from(const Role_id &user);
Auth_id_ref create_authid_from(const LEX_CSTRING &user,
                               const LEX_CSTRING &host);
bool roles_rename_authid(THD *thd, TABLE *edge_table, TABLE *defaults_table,
                         LEX_USER *user_from, LEX_USER *user_to);
bool set_and_validate_user_attributes(
    THD *thd, LEX_USER *Str, acl_table::Pod_user_what_to_update &what_to_set,
    bool is_privileged_user, bool is_role, TABLE_LIST *history_table,
    bool *history_check_done, const char *cmd, Userhostpassword_list &);
typedef std::pair<std::string, bool> Grant_privilege;
typedef std::unordered_multimap<const Role_id, Grant_privilege, role_id_hash>
    User_to_dynamic_privileges_map;
User_to_dynamic_privileges_map *get_dynamic_privileges_map();
User_to_dynamic_privileges_map *swap_dynamic_privileges_map(
    User_to_dynamic_privileges_map *map);
bool populate_roles_caches(THD *thd, TABLE_LIST *tablelst);
void grant_role(ACL_USER *role, const ACL_USER *user, bool with_admin_opt);
void get_mandatory_roles(std::vector<Role_id> *mandatory_roles);
extern std::vector<Role_id> *g_mandatory_roles;
void create_role_vertex(ACL_USER *role_acl_user);
void activate_all_granted_roles(const ACL_USER *acl_user,
                                Security_context *sctx);
void activate_all_granted_and_mandatory_roles(const ACL_USER *acl_user,
                                              Security_context *sctx);

bool alter_user_set_default_roles(THD *thd, TABLE *table, LEX_USER *user,
                                  const List_of_auth_id_refs &new_auth_ids);

bool alter_user_set_default_roles_all(THD *thd, TABLE *def_role_table,
                                      LEX_USER *user);
/*
  Checks if any of the users has SYSTEM_USER privilege then current user
  must also have SYSTEM_USER privilege.
  It is a wrapper over the  Privilege_checker class that does
  privilege checks for one user at a time.
*/
bool check_system_user_privilege(THD *thd, List<LEX_USER> list);

#endif /* AUTH_INTERNAL_INCLUDED */
