/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>
#include <stddef.h>

#include "my_inttypes.h"
#include "sql/dd/impl/dictionary_impl.h"
#include "sql/dd/impl/raw/object_keys.h"
#include "sql/dd/impl/raw/raw_record.h"
#include "sql/dd/impl/raw/raw_table.h"
#include "sql/dd/impl/tables/schemata.h"
#include "sql/dd/impl/transaction_impl.h"
#include "sql/dd/impl/types/schema_impl.h"
#include "sql/thd_raii.h"
#include "unittest/gunit/dd.h"
#include "unittest/gunit/test_utils.h"

namespace dd_schema_unittest {

using namespace dd;
using namespace dd_unittest;

using dd_unittest::Mock_dd_field_longlong;
using dd_unittest::Mock_dd_field_varstring;
using dd_unittest::Mock_dd_HANDLER;

using my_testing::Server_initializer;
using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::StrictMock;
using ::testing::WithArgs;

/**
  Test fixture for testing the dd::Schema, Schema_impl and Raw_* classes.
  A fresh instance of this class will be created for each of the TEST_F
  functions below.

  The functions SetUp(), TearDown(), SetUpTestCase(), TearDownTestCase() are
  inherited from ::testing::Test (google naming style differs from MySQL).
*/

class SchemaTest : public ::testing::Test {
 protected:
  SchemaTest() {}

  virtual void SetUp() {
    m_dict = new Dictionary_impl();
    m_def_cat_id = Dictionary_impl::DEFAULT_CATALOG_ID;

    // Set handlerton slot, needed for lookup in handler's ht_info.
    m_hton.slot = 0;

    // Dummy server initialization.
    m_init.SetUp();

    // Ensure that we can use Update_dictionary_tables_ctx without
    // employing Disable_autocommit_guard
    m_init.thd()->variables.option_bits &= ~OPTION_AUTOCOMMIT;
    m_init.thd()->variables.option_bits |= OPTION_NOT_AUTOCOMMIT;
  }

  virtual void TearDown() {
    delete m_dict;

    // Tear down dummy server.
    m_init.TearDown();
  }

  // Return dummy thd.
  THD *thd() { return m_init.thd(); }

  // Return dummy handlerton.
  handlerton *hton() { return &m_hton; }

  // Begin transaction.
  Update_dictionary_tables_ctx *begin_dd_updates() {
    Update_dictionary_tables_ctx *ctx =
        new (std::nothrow) Update_dictionary_tables_ctx(thd());
    EXPECT_TRUE(thd()->variables.option_bits & OPTION_DD_UPDATE_CONTEXT);

    // Add schema table to transaction context.
    ctx->otx.register_tables<Schema>();

    // Fake ctx->open_tables() by assigning fake schema TABLE object directly.
    ctx->otx.get_table<dd::Schema>()->get_table_list()->table =
        get_schema_table(thd(), hton());

    return ctx;
  }

  // Delete instances and commit transaction.
  void commit_transaction(Update_dictionary_tables_ctx *ctx,
                          Fake_TABLE *table) {
    delete ctx;

    // Must destroy fields and handler explicitly to avoid gmock warning
    for (uint i = 0; i < table->s->fields; ++i) destroy(table->field[i]);

    destroy(table->file);
    delete[] table->s->default_values;
    delete[] table->record[1];
    delete table;
  }

  handlerton m_hton;          // Dummy handlerton.
  Dictionary_impl *m_dict;    // Dictionary instance.
  Object_id m_def_cat_id;     // Default catalog id.
  Server_initializer m_init;  // Server initializer.

 private:
  // Declares (but does not define) copy constructor and assignment operator.
  GTEST_DISALLOW_COPY_AND_ASSIGN_(SchemaTest);
};

/**
  When storing an object, Weak_object_impl::store is called.

  The primary key for the object is created, returning a new Primary_id_key
  instance with the object id, given by Entity_object::id(). The primary
  key instance represents the primary key value.

  Then, a Raw_record is created by Raw_table::prepare_record_for_update.
  Here, the record is essentially retrieved by find_record based on the
  primary key. This is done by creating a raw access key, which is the primary
  key value with a physical representation suitable for looking up in a storage
  engine. The raw key is used to get the appropriate record from the dd tables
  by calling ha_index_read_idx_map. By instrumenting this function to return
  1, indicating no record found, we will provoke insert.

  Then, for insert, Weak_object_impl::store will first call
  prepare_record_for_insert, which just creates a new Raw_new_record instance.
  Then, the schema object members are copied into the appropriate table fields
  by calling the function Schema_impl::store_attributes. This, in turn, invokes
  the various Field*::store functions, which are set up to invoke the
  corresponding fake_store methods, and finally, Raw_new_record::insert
  is called, which calls ha_write_row. The write_row function in the handler
  api is instrumented to return 0 (success).

  After storing the schema object, we verify that the values saved in
  the various fields by fake_store are the same as we used to create the schema
  object.
*/

TEST_F(SchemaTest, CreateSchema) {
  // Execution context.
  Schema_impl *schema = nullptr;
  Update_dictionary_tables_ctx *ctx = begin_dd_updates();
  Fake_TABLE *schemata_table =
      static_cast<Fake_TABLE *>(ctx->otx.get_table<Schema>()->get_table());
  Mock_dd_HANDLER *ha = static_cast<Mock_dd_HANDLER *>(schemata_table->file);

  // Schemata table fields.
  Mock_dd_field_longlong *id =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[0]);
  Mock_dd_field_longlong *catalog_id =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[1]);
  Mock_dd_field_varstring *name =
      static_cast<Mock_dd_field_varstring *>(schemata_table->field[2]);
  Mock_dd_field_longlong *collation_id =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[3]);
  Mock_dd_field_longlong *created =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[4]);
  Mock_dd_field_longlong *last_altered =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[5]);

  // Schema properties.
  Object_id real_id = 10;
  const char *real_name = "testdd_schema";
  Object_id real_collation_id = 20;
  ulonglong real_created = 30;
  ulonglong real_last_altered = 40;

  // Create a new schema, set properties.
  schema = new Schema_impl();
  schema->set_id(real_id);
  schema->set_name(real_name);
  schema->set_default_collation_id(real_collation_id);
  schema->set_created(real_created);
  schema->set_last_altered(real_last_altered);

  // Set expectations for insert:

  // ha->index_read_idx_map: Called once, return 1
  ON_CALL(*ha, index_read_idx_map(_, _, _, _, _))
      .WillByDefault(Return(HA_ERR_KEY_NOT_FOUND));
  EXPECT_CALL(*ha, index_read_idx_map(_, _, _, _, _)).Times(1);

  // id->store: Called twice, return 0
  ON_CALL(*id, store(real_id, true))
      .WillByDefault(Invoke(id, &Mock_dd_field_longlong::fake_store));
  EXPECT_CALL(*id, store(real_id, true)).Times(2);

  // catalog_id->store: Called once, return 0
  ON_CALL(*catalog_id, store(m_def_cat_id, true))
      .WillByDefault(Invoke(catalog_id, &Mock_dd_field_longlong::fake_store));
  EXPECT_CALL(*catalog_id, store(m_def_cat_id, true)).Times(1);

  // name->store: Called once, return 0
  ON_CALL(*name, store(schema->name().c_str(), _, _))
      .WillByDefault(
          WithArgs<0>(Invoke(name, &Mock_dd_field_varstring::fake_store)));
  EXPECT_CALL(*name, store(schema->name().c_str(), _, _)).Times(1);

  // collation_id->store: Called once, return 0
  ON_CALL(*collation_id, store(real_collation_id, true))
      .WillByDefault(Invoke(collation_id, &Mock_dd_field_longlong::fake_store));
  EXPECT_CALL(*collation_id, store(real_collation_id, true)).Times(1);

  // created->store: Called once, return 0
  ON_CALL(*created, store(real_created, true))
      .WillByDefault(Invoke(created, &Mock_dd_field_longlong::fake_store));
  EXPECT_CALL(*created, store(real_created, true)).Times(1);

  // last_altered->store: Called once, return 0
  ON_CALL(*last_altered, store(real_last_altered, true))
      .WillByDefault(Invoke(last_altered, &Mock_dd_field_longlong::fake_store));
  EXPECT_CALL(*last_altered, store(real_last_altered, true)).Times(1);

  // ha->write_row: Called once, return 0
  ON_CALL(*ha, write_row(_)).WillByDefault(Return(0));
  EXPECT_CALL(*ha, write_row(_)).Times(1);

  // Store the schema.
  schema->store(&ctx->otx);

  // Verify real contents stored into faked fields.
  EXPECT_TRUE(id->fake_val_uint() == real_id);
  EXPECT_TRUE(catalog_id->fake_val_uint() == m_def_cat_id);
  EXPECT_TRUE(strcmp(name->fake_val_c_str(), real_name) == 0);
  EXPECT_TRUE(collation_id->fake_val_uint() == real_collation_id);
  EXPECT_TRUE(created->fake_val_uint() == real_created);
  EXPECT_TRUE(last_altered->fake_val_uint() == real_last_altered);

  // Commit transaction and cleanup.
  commit_transaction(ctx, schemata_table);
  delete schema;
}

/**
  To provoke an update, the setup is pretty much the same as for insert (see
  above), but we must instrument index_read_idx_map to return 0. This
  makes a new Raw_record be created, and makes ha_update_row be called.
*/

TEST_F(SchemaTest, UpdateSchema) {
  // Execution context.
  Schema_impl *schema = nullptr;
  Update_dictionary_tables_ctx *ctx = begin_dd_updates();
  Fake_TABLE *schemata_table =
      static_cast<Fake_TABLE *>(ctx->otx.get_table<Schema>()->get_table());
  Mock_dd_HANDLER *ha = static_cast<Mock_dd_HANDLER *>(schemata_table->file);

  // Schemata table fields.
  Mock_dd_field_longlong *id =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[0]);
  Mock_dd_field_longlong *catalog_id =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[1]);
  Mock_dd_field_varstring *name =
      static_cast<Mock_dd_field_varstring *>(schemata_table->field[2]);
  Mock_dd_field_longlong *collation_id =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[3]);
  Mock_dd_field_longlong *created =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[4]);
  Mock_dd_field_longlong *last_altered =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[5]);

  // Schema properties.
  Object_id real_id = 11;
  const char *real_name = "testdd_schema";
  Object_id real_collation_id = 21;
  ulonglong real_created = 31;
  ulonglong real_last_altered = 41;

  // Create a schema, set properties.
  schema = new Schema_impl();
  schema->set_id(real_id);
  schema->set_name(real_name);
  schema->set_default_collation_id(real_collation_id);
  schema->set_created(real_created);
  schema->set_last_altered(real_last_altered);

  // Set expectations for update:

  // ha->index_read_idx_map: Called once, return 0
  ON_CALL(*ha, index_read_idx_map(_, _, _, _, _)).WillByDefault(Return(0));
  EXPECT_CALL(*ha, index_read_idx_map(_, _, _, _, _)).Times(1);

  // id->store: Called twice, return 0
  ON_CALL(*id, store(real_id, true))
      .WillByDefault(Invoke(id, &Mock_dd_field_longlong::fake_store));
  EXPECT_CALL(*id, store(real_id, true)).Times(2);

  // catalog_id->store: Called once, return 0
  ON_CALL(*catalog_id, store(m_def_cat_id, true))
      .WillByDefault(Invoke(catalog_id, &Mock_dd_field_longlong::fake_store));
  EXPECT_CALL(*catalog_id, store(m_def_cat_id, true)).Times(1);

  // name->store: Called once, return 0
  ON_CALL(*name, store(schema->name().c_str(), _, _))
      .WillByDefault(
          WithArgs<0>(Invoke(name, &Mock_dd_field_varstring::fake_store)));
  EXPECT_CALL(*name, store(schema->name().c_str(), _, _)).Times(1);

  // collation_id->store: Called once, return 0
  ON_CALL(*collation_id, store(real_collation_id, true))
      .WillByDefault(Invoke(collation_id, &Mock_dd_field_longlong::fake_store));
  EXPECT_CALL(*collation_id, store(real_collation_id, true)).Times(1);

  // created->store: Called once, return 0
  ON_CALL(*created, store(real_created, true))
      .WillByDefault(Invoke(created, &Mock_dd_field_longlong::fake_store));
  EXPECT_CALL(*created, store(real_created, true)).Times(1);

  // last_altered->store: Called once, return 0
  ON_CALL(*last_altered, store(real_last_altered, true))
      .WillByDefault(Invoke(last_altered, &Mock_dd_field_longlong::fake_store));
  EXPECT_CALL(*last_altered, store(real_last_altered, true)).Times(1);

  // ha->update_row: Called once, return 0
  ON_CALL(*ha, update_row(_, _)).WillByDefault(Return(0));
  EXPECT_CALL(*ha, update_row(_, _)).Times(1);

  // Store the schema.
  schema->store(&ctx->otx);

  // Verify real contents stored into faked fields.
  EXPECT_TRUE(id->fake_val_uint() == real_id);
  EXPECT_TRUE(catalog_id->fake_val_uint() == m_def_cat_id);
  EXPECT_TRUE(strcmp(name->fake_val_c_str(), real_name) == 0);
  EXPECT_TRUE(collation_id->fake_val_uint() == real_collation_id);
  EXPECT_TRUE(created->fake_val_uint() == real_created);
  EXPECT_TRUE(last_altered->fake_val_uint() == real_last_altered);

  // Commit transaction and cleanup.
  commit_transaction(ctx, schemata_table);
  delete schema;
}

/**
  For testing getting a schema, the setup is slightly different than for
  insert and update. For a name lookup, the schemata table is first
  read using catalog_id and name to get the primary key id. Then, the
  table is read to retrieve the full schema object.

  To get the expected behavior, we explicitly call the various fake_store
  methods on the table fields to set the contents in advance. This will make
  the val_* calls done when reading the schema object return the expected
  values. After the schema object has been retrieved, we verify that the
  contents of the object are the same as the faked and predefined values
  we stored in the various table fields.
*/

TEST_F(SchemaTest, GetSchema) {
  // Execution context.
  const Schema_impl *schema = nullptr;
  Update_dictionary_tables_ctx *ctx = begin_dd_updates();
  Fake_TABLE *schemata_table =
      static_cast<Fake_TABLE *>(ctx->otx.get_table<Schema>()->get_table());
  Mock_dd_HANDLER *ha = static_cast<Mock_dd_HANDLER *>(schemata_table->file);

  // Schemata table fields.
  Mock_dd_field_longlong *id =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[0]);
  Mock_dd_field_longlong *catalog_id =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[1]);
  Mock_dd_field_varstring *name =
      static_cast<Mock_dd_field_varstring *>(schemata_table->field[2]);
  Mock_dd_field_longlong *collation_id =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[3]);
  Mock_dd_field_longlong *created =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[4]);
  Mock_dd_field_longlong *last_altered =
      static_cast<Mock_dd_field_longlong *>(schemata_table->field[5]);

  // Schema properties.
  Object_id real_id = 12;
  const char *real_name = "testdd_schema";
  Object_id real_collation_id = 22;
  ulonglong real_created = 32;
  ulonglong real_last_altered = 42;

  // Set expectations for read execution:

  // catalog_id->store: Called once, return 0
  ON_CALL(*catalog_id, store(m_def_cat_id, true))
      .WillByDefault(Return(TYPE_OK));
  EXPECT_CALL(*catalog_id, store(m_def_cat_id, true)).Times(1);

  // name->store: Called once, return TYPE_OK
  ON_CALL(*name, store(_, _, _)).WillByDefault(Return(TYPE_OK));
  EXPECT_CALL(*name, store(_, _, _)).Times(1);

  // ha->index_read: Called once, return 0
  ON_CALL(*ha, index_read_idx_map(_, _, _, _, _)).WillByDefault(Return(0));
  EXPECT_CALL(*ha, index_read_idx_map(_, _, _, _, _)).Times(1);

  // id->val_int: Called once, get faked id
  ON_CALL(*id, val_int())
      .WillByDefault(Invoke(id, &Mock_dd_field_longlong::fake_val_int));
  EXPECT_CALL(*id, val_int()).Times(1);

  // catalog_id->val_int: Never called, get faked id
  ON_CALL(*catalog_id, val_int())
      .WillByDefault(Invoke(catalog_id, &Mock_dd_field_longlong::fake_val_int));
  EXPECT_CALL(*catalog_id, val_int()).Times(0);

  // name->val_str: Called once, get faked name
  ON_CALL(*name, val_str(_, _))
      .WillByDefault(
          WithArgs<1>(Invoke(name, &Mock_dd_field_varstring::fake_val_str)));
  EXPECT_CALL(*name, val_str(_, _)).Times(1);

  // collation_id->val_int: Called once, get faked id
  ON_CALL(*collation_id, val_int())
      .WillByDefault(
          Invoke(collation_id, &Mock_dd_field_longlong::fake_val_int));
  EXPECT_CALL(*collation_id, val_int()).Times(1);

  // created->val_int: Called once, get faked value
  ON_CALL(*created, val_int())
      .WillByDefault(Invoke(created, &Mock_dd_field_longlong::fake_val_int));
  EXPECT_CALL(*created, val_int()).Times(1);

  // last_altered->val_int: Called once, get faked value
  ON_CALL(*last_altered, val_int())
      .WillByDefault(
          Invoke(last_altered, &Mock_dd_field_longlong::fake_val_int));
  EXPECT_CALL(*last_altered, val_int()).Times(1);

  // Set faked field contents.
  id->fake_store(real_id, true);
  catalog_id->fake_store(m_def_cat_id, true);
  name->fake_store(real_name);
  collation_id->fake_store(real_collation_id, true);
  created->fake_store(real_created, true);
  last_altered->fake_store(real_last_altered, true);

  // Create a name key.
  Item_name_key key;
  Schema::update_name_key(&key, real_name);

  // Get the raw table and lookup the object by the name key.
  Raw_table *t = ctx->otx.get_table<Schema>();
  std::unique_ptr<Raw_record> r;
  EXPECT_FALSE(t->find_record(key, r));

  // Restore the object from the record.
  Entity_object *new_object = nullptr;
  EXPECT_FALSE(Schema::DD_table::instance().restore_object_from_record(
      &ctx->otx, *r.get(), &new_object));
  schema = dynamic_cast<const Schema_impl *>(new_object);

  // Verify values stored into faked fields are read into schema object.
  EXPECT_TRUE(schema->id() == real_id);

  // Catalog id not exposed in dd api yet
  EXPECT_TRUE(schema->name() == real_name);
  EXPECT_TRUE(schema->default_collation_id() == real_collation_id);
  EXPECT_TRUE(schema->created(false) == real_created);
  EXPECT_TRUE(schema->last_altered(false) == real_last_altered);

  // Commit transaction and cleanup.
  commit_transaction(ctx, schemata_table);
  delete schema;
}
}  // namespace dd_schema_unittest
