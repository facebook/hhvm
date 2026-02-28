#ifndef OPT_COSTCONSTANTS_INCLUDED
#define OPT_COSTCONSTANTS_INCLUDED

/*
   Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <stddef.h>
#include <sys/types.h>

#include "lex_string.h"
#include "my_dbug.h"
#include "prealloced_array.h"

class THD;
struct TABLE;

/**
  Error codes returned from the functions that do updates of the
  cost constants.
*/
enum cost_constant_error {
  COST_CONSTANT_OK,
  UNKNOWN_COST_NAME,
  UNKNOWN_ENGINE_NAME,
  INVALID_COST_VALUE,
  INVALID_DEVICE_TYPE
};

/**
  The cost model should support different types of storage devices each with
  different cost constants. Due to that we in the current version does not
  have a way to know which storage device a given table is stored on, the
  initial version of the cost model will only have one set of cost constants
  per storage engine.
*/
const unsigned int MAX_STORAGE_CLASSES = 1;

/**
  Cost constants for operations done by the server
*/

class Server_cost_constants {
 public:
  /**
    Creates a server cost constants object using the default values
    defined in this class.
  */
  Server_cost_constants()
      : m_row_evaluate_cost(ROW_EVALUATE_COST),
        m_key_compare_cost(KEY_COMPARE_COST),
        m_memory_temptable_create_cost(MEMORY_TEMPTABLE_CREATE_COST),
        m_memory_temptable_row_cost(MEMORY_TEMPTABLE_ROW_COST),
        m_disk_temptable_create_cost(DISK_TEMPTABLE_CREATE_COST),
        m_disk_temptable_row_cost(DISK_TEMPTABLE_ROW_COST) {}

  /**
    Cost for evaluating the query condition on a row.
  */
  double row_evaluate_cost() const { return m_row_evaluate_cost; }

  /**
    Cost for comparing two keys.
  */
  double key_compare_cost() const { return m_key_compare_cost; }

  /**
    Cost for creating an internal temporary table in memory.
  */
  double memory_temptable_create_cost() const {
    return m_memory_temptable_create_cost;
  }

  /**
    Cost for retrieving or storing a row in an internal temporary table
    stored in memory.
  */
  double memory_temptable_row_cost() const {
    return m_memory_temptable_row_cost;
  }

  /**
    Cost for creating an internal temporary table in a disk resident
    storage engine.
  */
  double disk_temptable_create_cost() const {
    return m_disk_temptable_create_cost;
  }

  /**
    Cost for retrieving or storing a row in an internal disk resident
    temporary table.
  */
  double disk_temptable_row_cost() const { return m_disk_temptable_row_cost; }

  /**
    Set the value of one of the cost constants.

    @param name  name of cost constant
    @param value new value

    @return Status for updating the cost constant
  */

  cost_constant_error set(const LEX_CSTRING &name, const double value);

 private:
  /*
    This section declares constants for the default values. The actual
    default values are found in the .cc file.
  */

  /// Default cost for evaluation of the query condition for a row.
  static const double ROW_EVALUATE_COST;

  /// Default cost for comparing row ids.
  static const double KEY_COMPARE_COST;

  /*
    Constants related to the use of temporary tables in query execution.
    Lookup and write operations are currently assumed to be equally costly
    (concerns MEMORY_TEMPTABLE_ROW_COST and DISK_TEMPTABLE_ROW_COST).
  */

  /// Cost for creating a memory temporary table.
  static const double MEMORY_TEMPTABLE_CREATE_COST;

  /// Cost for inserting or reading a row in a memory temporary table.
  static const double MEMORY_TEMPTABLE_ROW_COST;

  /// Cost for creating a disk temporary table
  static const double DISK_TEMPTABLE_CREATE_COST;

  /// Cost for inserting or reading a row in a disk temporary table.
  static const double DISK_TEMPTABLE_ROW_COST;

  /*
    This section specifies cost constants for server operations
  */

  /// Cost for evaluating the query condition on a row
  double m_row_evaluate_cost;

  /// Cost for comparing two keys
  double m_key_compare_cost;

  /// Cost for creating an internal temporary table in memory
  double m_memory_temptable_create_cost;

  /**
    Cost for retrieving or storing a row in an internal temporary table
    stored in memory.
  */
  double m_memory_temptable_row_cost;

  /**
    Cost for creating an internal temporary table in a disk resident
    storage engine.
  */
  double m_disk_temptable_create_cost;

  /**
    Cost for retrieving or storing a row in an internal disk resident
    temporary table.
  */
  double m_disk_temptable_row_cost;
};

/**
  Cost constants for a storage engine.

  Storage engines that want to add new cost constants should make
  a subclass of this class.
*/

class SE_cost_constants {
 public:
  SE_cost_constants()
      : m_memory_block_read_cost(MEMORY_BLOCK_READ_COST),
        m_io_block_read_cost(IO_BLOCK_READ_COST),
        m_memory_block_read_cost_default(true),
        m_io_block_read_cost_default(true) {}

  virtual ~SE_cost_constants() {}

  /**
    Cost of reading one random block from an in-memory database buffer.
  */

  double memory_block_read_cost() const { return m_memory_block_read_cost; }

  /**
    Cost of reading one random block from disk.
  */

  double io_block_read_cost() const { return m_io_block_read_cost; }

 protected:
  /**
    Set the value of one of the cost constants.

    If a storage engine wants to introduce a new cost constant, it should
    provide an implementation of this function. If the cost constant
    is not recognized by the function in the subclass, then this function
    should be called to allow the cost constant in the base class to be
    given the updated value.

    @param name    name of cost constant
    @param value   new value
    @param default_value specify whether the new value is a default value or
                   an engine specific value

    @return Status for updating the cost constant
  */

  virtual cost_constant_error set(const LEX_CSTRING &name, const double value,
                                  bool default_value);

 protected:
  friend class Cost_model_constants;

  /**
    Update the value of a cost constant.

    @param name  name of the cost constant
    @param value the new value this cost constant should take

    @return Status for updating the cost constant
  */

  cost_constant_error update(const LEX_CSTRING &name, const double value);

  /**
    Update the default value of a cost constant.

    If this const constant already has been given a non-default value,
    then calling this will have no effect on the current value for the
    cost constant.

    @param name  name of the cost constant
    @param value the new value this cost constant should take

    @return Status for updating the cost constant
  */

  cost_constant_error update_default(const LEX_CSTRING &name,
                                     const double value);

  /**
    Utility function for changing the value of a cost constant.

    The cost constant will be updated to the new value iff:
    a) the current value is the default value, or
    b) the current value is not the default value and the new value
       is not a default value

    @param[out] cost_constant               pointer to the cost constant that
                                            should be updated
    @param[in,out] cost_constant_is_default whether the current value has the
                                            default value or not
    @param new_value                        the new value for the cost constant
    @param new_value_is_default             whether this is a new default value
                                            or not
  */

  void update_cost_value(double *cost_constant, bool *cost_constant_is_default,
                         double new_value, bool new_value_is_default);

 private:
  /*
    This section specifies default values for cost constants.
  */

  /// Default cost for reading a random block from an in-memory buffer
  static const double MEMORY_BLOCK_READ_COST;

  /// Default cost for reading a random disk block
  static const double IO_BLOCK_READ_COST;

  /*
    This section specifies cost constants for the table
  */

  /// Cost constant for reading a random block from an in-memory buffer
  double m_memory_block_read_cost;

  /// Cost constant for reading a random disk block.
  double m_io_block_read_cost;

  /*
    This section has boolean variables that is used for knowing whether
    the above cost variables is using the default value or not.
  */

  /// Whether the memory_block_read_cost is a default value or not
  bool m_memory_block_read_cost_default;

  /// Whether the io_block_read_cost is a default value or not
  bool m_io_block_read_cost_default;
};

/**
  Class that keeps all cost constants for a storage engine. Since
  storage engines can use different types of storage devices, each
  device type can have its own set of cost constants.

  @note In the initial implementation there will only be one
  set of cost constants per storage engine.
*/

class Cost_model_se_info {
 public:
  /**
    Constructor that just initializes the class.
  */
  Cost_model_se_info();

  /**
    Desctructor. Deletes the allocated cost constant objects.
  */
  ~Cost_model_se_info();

 private:
  /*
    Since this object owns the cost constant objects, we must prevent that we
    create copies of this object that share the cost constant objects.
  */
  Cost_model_se_info(const Cost_model_se_info &);
  Cost_model_se_info &operator=(const Cost_model_se_info &rhs);

 protected:
  friend class Cost_model_constants;

  /**
    Set the storage constants to be used for a given storage type for
    this storage engine.

    @param cost_constants cost constants for the storage engine
    @param storage_class  the storage class these cost constants should be
                          used for
  */

  void set_cost_constants(SE_cost_constants *cost_constants,
                          unsigned int storage_class) {
    DBUG_ASSERT(cost_constants != nullptr);
    DBUG_ASSERT(storage_class < MAX_STORAGE_CLASSES);
    DBUG_ASSERT(m_se_cost_constants[storage_class] == nullptr);

    m_se_cost_constants[storage_class] = cost_constants;
  }

  /**
    Retrieve the cost constants to be used for this storage engine for
    a specified storage class.

    @param storage_class the storage class these cost constants should be
                         used for
  */

  const SE_cost_constants *get_cost_constants(
      unsigned int storage_class) const {
    DBUG_ASSERT(storage_class < MAX_STORAGE_CLASSES);
    DBUG_ASSERT(m_se_cost_constants[storage_class] != nullptr);

    return m_se_cost_constants[storage_class];
  }

  /**
    Retrieve the cost constants to be used for this storage engine for
    a specified storage class.

    @param storage_class the storage class these cost constants should be
                         used for
  */

  SE_cost_constants *get_cost_constants(unsigned int storage_class) {
    DBUG_ASSERT(storage_class < MAX_STORAGE_CLASSES);
    DBUG_ASSERT(m_se_cost_constants[storage_class] != nullptr);

    return m_se_cost_constants[storage_class];
  }

 private:
  /**
    Array of cost constant sets for this storage engine. There will
    be one set of cost constants for each device type defined for the
    storage engine.
  */
  SE_cost_constants *m_se_cost_constants[MAX_STORAGE_CLASSES];
};

/**
  Set of all cost constants used by the server and all storage engines.
*/

class Cost_model_constants {
 public:
  /**
    Creates a set with cost constants using the default values defined in
    the source code.
  */

  Cost_model_constants();

  /**
    Destructor.

    @note The only reason for making this virtual is to be able to make
    a sub-class for use in unit testing.
  */

  virtual ~Cost_model_constants();

  /**
    Get the cost constants that should be used for server operations.

    @return the cost constants for the server
  */

  const Server_cost_constants *get_server_cost_constants() const {
    return &m_server_constants;
  }

  /**
    Return the cost constants that should be used for a given table.

    @param table the table to find cost constants for

    @return the cost constants to use for the table
  */

  const SE_cost_constants *get_se_cost_constants(const TABLE *table) const;

  /**
    Update the value for one of the server cost constants.

    @param name  name of the cost constant
    @param value new value

    @return Status for updating the cost constant
  */

  cost_constant_error update_server_cost_constant(const LEX_CSTRING &name,
                                                  double value);

  /**
    Update the value for one of the storage engine cost constants.

    @param thd              the THD
    @param se_name          name of storage engine
    @param storage_category storage device type
    @param name             name of cost constant
    @param value            new value

    @return Status for updating the cost constant
  */

  cost_constant_error update_engine_cost_constant(THD *thd,
                                                  const LEX_CSTRING &se_name,
                                                  uint storage_category,
                                                  const LEX_CSTRING &name,
                                                  double value);

 protected:
  friend class Cost_constant_cache;

  /**
    Increment the reference counter for this cost constant set
  */

  void inc_ref_count() { m_ref_counter++; }

  /**
    Decrement the reference counter for this cost constant set

    When the returned value is zero, there is nobody using this object
    and it can be deleted by the caller.

    @return the updated reference count
  */

  unsigned int dec_ref_count() {
    DBUG_ASSERT(m_ref_counter > 0);

    m_ref_counter--;
    return m_ref_counter;
  }

 private:
  /**
    Utility function for finding the slot number for a storage engine
    based on the storage engine name.

    The only reason for making this function virtual is to be able to
    override it in unit tests.

    @param thd  the THD
    @param name name of storage engine

    @return slot number for the storage engine, HA_SLOT_UNDEF if there
            is no handler for this name
  */

  virtual uint find_handler_slot_from_name(THD *thd,
                                           const LEX_CSTRING &name) const;

  /**
    Update the default value for a storage engine cost constant.

    @param name             name of cost constant
    @param storage_category storage device type
    @param value            new value

    @return Status for updating the cost constant
  */

  cost_constant_error update_engine_default_cost(const LEX_CSTRING &name,
                                                 uint storage_category,
                                                 double value);

  /// Cost constants for server operations
  Server_cost_constants m_server_constants;

  /**
    Cost constants for storage engines
    15 should be enough for most use cases, see PREALLOC_NUM_HA.
  */
  Prealloced_array<Cost_model_se_info, 15> m_engines;

  /// Reference counter for this set of cost constants.
  unsigned int m_ref_counter;
};

#endif /* OPT_COSTCONSTANTS_INCLUDEDED */
