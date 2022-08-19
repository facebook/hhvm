/* Copyright (c) 2014, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef DD__SYSTEM_REGISTRY_INCLUDED
#define DD__SYSTEM_REGISTRY_INCLUDED

#include <stdio.h>
#include <map>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include "my_dbug.h"
#include "mysqld_error.h"        // ER_NO_SYSTEM_TABLE_...
#include "sql/dd/string_type.h"  // dd::String_type
#include "sql/dd/types/object_table.h"
#include "sql/dd/types/system_view.h"
#include "sql/plugin_table.h"

namespace dd {

class Object_table;

/**
  Class to wrap an entity object.

  The Entity_element instances are intended used in registries to support
  classification of meta data. The Entity_element template points to an
  instance of some type, and also has a property member that is associated
  with the entity, as well as a key with which the object is associated.

  @note The entity object ownership is decided by the relevant template
        parameter, see below.

  @tparam K    A key type, e.g. a pair of strings.
  @tparam T    An entity type, e.g. an Object_table instance.
  @tparam P    A property type, e.g. an enumeration.
  @tparam F    Function to map from property to name.
  @tparam D    Boolean to decide ownership of wrapped object.
*/

template <typename K, typename T, typename P, const char *F(P), bool D>
class Entity_element {
 private:
  const K m_key;      //!< The key associated with the entity object.
  const T *m_entity;  //!< Entity object pointer, like an Object_table instance.
  const P m_property;  //!< Property of some kind, like an enumeration.

 public:
  Entity_element(const K &key, const T *entity, const P property)
      : m_key(key), m_entity(entity), m_property(property) {}

  ~Entity_element() {
    // Delete the wrapped object depending on the template parameter.
    if (D) delete m_entity;
  }

  const K &key() const { return m_key; }

  const T *entity() const { return m_entity; }

  const P property() const { return m_property; }

  const P *property_ptr() const { return &m_property; }

#ifndef DBUG_OFF
  void dump() const {
    fprintf(stderr, "Key= '%s.%s', property= '%s'\n", m_key.first.c_str(),
            m_key.second.c_str(), F(m_property));
  }
#endif
};

/**
  Class to represent collections of meta data for entities.

  This class is used to represent relevant meta data and properties of
  system related entities, e.g. all the dictionary tables. The meta data
  is associated with a key instance, usually a std::pair containing the
  schema- and entity names.

  The class supports adding and retrieving objects, as well as iteration
  based on the order of inserts.

  @note There is no support for concurrency. We assume that the registry
        is created and that the entities are inserted in a single threaded
        situation, e.g. while the server is being started. Read only access
        can happen from several threads simultaneously, though.

  @tparam   K   Key type.
  @tparam   T   Entity type.
  @tparam   P   Property type.
  @tparam   F   Function to map from property to name.
  @tparam   D   Boolean flag to decide whether the wrapped objects
                are owned by the registry.
*/

template <typename K, typename T, typename P, const char *F(P), bool D>
class Entity_registry {
 private:
  typedef Entity_element<K, T, P, F, D> Entity_element_type;
  typedef std::vector<Entity_element_type *> Entity_list_type;
  typedef std::map<K, Entity_element_type *> Entity_map_type;

  Entity_list_type m_entity_list;  //!< List for ordered access.
  Entity_map_type m_entity_map;    //!< Map for direct key based lookup.

 public:
  // Externally available iteration is based on the order of inserts.
  typedef typename Entity_list_type::const_iterator Const_iterator;

  /**
    Delete the heap memory owned by the entity registry.

    Only the wrapper elements are owned by the registry.
    The std::vector and std::map are not allocated
    dynamically, and their destructors are called implicitly
    when this destructor is called.
  */

  ~Entity_registry() {
    // Delete elements from the map. The objects that are wrapped
    // will be handled by the wrapper destructor.
    for (typename Entity_map_type::iterator it = m_entity_map.begin();
         it != m_entity_map.end(); ++it)
      delete it->second;
  }

  /**
    Add a new entity to the registry.

    This method will create a new key and a new entity wrapper element. The
    wrapper will be added to the map, along with the key, and to the vector
    to keep track of the order of inserts.

    @param schema_name  Schema name used to construct the key.
    @param entity_name  Entity name used to construct the key.
    @param property     Property used to classify the entity.
    @param entity       Entity which is classified and registered.
  */

  void add(const String_type &schema_name, const String_type &entity_name,
           P property, T *entity) {
    // Create a new key and make sure it does not already exist.
    K key(schema_name, entity_name);
    DBUG_ASSERT(m_entity_map.find(key) == m_entity_map.end());

    // Create a new entity wrapper element. The wrapper will be owned by the
    // entity map and deleted along with it.
    Entity_element_type *element =
        new (std::nothrow) Entity_element_type(key, entity, property);

    // Add the key, value pair to the map.
    m_entity_map.insert(typename Entity_map_type::value_type(key, element));

    // Add the entity to the ordered list too.
    m_entity_list.push_back(element);
  }

  /**
    Find an element entity in the registry.

    This method creates a key based on the submitted parameters, and
    looks up in the member map. If the key is found, the object pointed
    to by the wrapper is returned, otherwise, nullptr is returned.

    @param schema_name   Schema containing the entity searched for.
    @param entity_name   Name  of the entity searched for.
    @retval              nullptr if not found, otherwise, the entity
                         object pointed to by the wrapper element.
  */

  const T *find_entity(const String_type &schema_name,
                       const String_type &entity_name) const {
    // Create a new key. This is only used for lookup, so it is allocated
    // on the stack.
    K key(schema_name, entity_name);

    // Lookup in the map based on the key.
    typename Entity_map_type::const_iterator element_it =
        m_entity_map.find(key);

    // Return nullptr if not found, otherwise, return a pointer to the entity.
    if (element_it == m_entity_map.end()) return nullptr;

    return element_it->second->entity();
  }

  /**
    Find the property of an element in the registry.

    This method creates a key based on the submitted parameters, and
    looks up in the member map. If the key is found, the property
    associated with the key is returned, otherwise, nullptr is returned.

    @param schema_name   Schema containing the entity searched for.
    @param entity_name   Name  of the entity searched for.
    @retval              nullptr if not found, otherwise, the a pointer to
                         the property associated with the key.
  */

  const P *find_property(const String_type &schema_name,
                         const String_type &entity_name) const {
    // Create a new key. This is only used for lookup, so it is allocated
    // on the stack.
    K key(schema_name, entity_name);

    // Lookup in the map based on the key.
    typename Entity_map_type::const_iterator element_it =
        m_entity_map.find(key);

    // Return nullptr if not found, otherwise, return a pointer to the property.
    if (element_it == m_entity_map.end()) return nullptr;

    return element_it->second->property_ptr();
  }

  /**
    Get the beginning of the vector of ordered inserts.

    @retval Iterator referring to the first element in the vector,
            or end() if the vector is empty.
  */

  Const_iterator begin() const { return m_entity_list.begin(); }

  /**
    Get the first element with a certain property.

    This method retrieves the first element with a certain property, and
    is used for iterating only over elements having this property.

    @param property  Property that the element should have.
    @retval          Iterator referring to the first element in the vector
                     with the submitted property, or end().
  */

  Const_iterator begin(P property) const {
    Const_iterator it = begin();
    while (it != end()) {
      if ((*it)->property() == property) break;
      ++it;
    }

    return it;
  }

  /**
    Get the end of the vector of ordered inserts.

    @retval Iterator referring to the special element after the last "real"
            element in the vector.
  */

  Const_iterator end() const { return m_entity_list.end(); }

  /**
    Get the next element in the list of ordered inserts.

    @param current  The current iterator.
    @retval         Iterator referring to the next element in the vector.
  */

  Const_iterator next(Const_iterator current) const {
    if (current == end()) return current;

    return ++current;
  }

  /**
    Get the next element in the list of ordered inserts.

    This method retrieves the next element with a certain property, and
    is used for iterating only over elements having this property.

    @param current   The current iterator.
    @param property  Property that the next element should have.
    @retval          Iterator referring to the next element in the vector
                     with the submitted property.
  */

  Const_iterator next(Const_iterator current, P property) const {
    if (current == end()) return current;

    while (++current != end())
      if ((*current)->property() == property) break;

    return current;
  }

#ifndef DBUG_OFF
  void dump() const {
    // List the entities in the order they were inserted.
    for (Const_iterator it = begin(); it != end(); ++it) (*it)->dump();
  }
#endif
};

/**
  Class used to represent the dictionary tables.

  This class is a singleton used to represent meta data of the dictionary
  tables, i.e., the tables that store meta data about dictionary entities.
  The meta data collected here are the Object_table instances, which are
  used to e.g. get hold of the definition of the table.

  The singleton contains an instance of the Entity_registry class, and
  has methods that mostly delegate to this instance.
 */

class System_tables {
 public:
  /*
    Classification of tables based on WL#6391.

    - An INERT table can never change.
    - The dd::Table objects representing the CORE tables must be present
      to handle a cache miss for an arbitrary table.
    - The dd::Table objects representing the SECOND order tables can be
      fetched from the dd tables as long as the core table objects are
      present.
    - The DDSE tables are not needed by the data dictionary, but the
      server manages these based on requests from the DD storage engine.
    - The PFS tables are not needed by the data dictionary, but the
      server manages these based on requests from the performance schema.
  */
  enum class Types {
    INERT,
    CORE,
    SECOND,
    DDSE_PRIVATE,
    DDSE_PROTECTED,
    PFS,
    SYSTEM
  };

  // Map from system table type to string description, e.g. for debugging.
  static const char *type_name(Types type) {
    switch (type) {
      case Types::INERT:
        return "INERT";
      case Types::CORE:
        return "CORE";
      case Types::SECOND:
        return "SECOND";
      case Types::DDSE_PRIVATE:
        return "DDSE_PRIVATE";
      case Types::DDSE_PROTECTED:
        return "DDSE_PROTECTED";
      case Types::PFS:
        return "PFS";
      case Types::SYSTEM:
        return "SYSTEM";
      default:
        return "";
    }
  }

  // Map from system table type to error code for localized error messages.
  static int type_name_error_code(Types type) {
    if (type == Types::INERT || type == Types::CORE || type == Types::SECOND)
      return ER_NO_SYSTEM_TABLE_ACCESS_FOR_DICTIONARY_TABLE;

    if (type == Types::DDSE_PRIVATE || type == Types::DDSE_PROTECTED ||
        type == Types::PFS || type == Types::SYSTEM)
      return ER_NO_SYSTEM_TABLE_ACCESS_FOR_SYSTEM_TABLE;

    DBUG_ASSERT(false);
    return ER_NO_SYSTEM_TABLE_ACCESS_FOR_TABLE;
  }

 private:
  // The actual registry is referred and delegated to rather than
  // being inherited from.
  typedef Entity_registry<std::pair<const String_type, const String_type>,
                          const Object_table, Types, type_name, true>
      System_table_registry_type;
  System_table_registry_type m_registry;

 public:
  // The ordered iterator type must be public.
  typedef System_table_registry_type::Const_iterator Const_iterator;

  static System_tables *instance();

  // Add predefined system tables.
  void add_inert_dd_tables();
  void add_remaining_dd_tables();

  // Add a new system table by delegation to the wrapped registry.
  void add(const String_type &schema_name, const String_type &table_name,
           Types type, const Object_table *table) {
    m_registry.add(schema_name, table_name, type, table);
  }

  // Find a system table by delegation to the wrapped registry.
  const Object_table *find_table(const String_type &schema_name,
                                 const String_type &table_name) const {
    return m_registry.find_entity(schema_name, table_name);
  }

  // Find a system table type by delegation to the wrapped registry.
  const Types *find_type(const String_type &schema_name,
                         const String_type &table_name) const {
    return m_registry.find_property(schema_name, table_name);
  }

  Const_iterator begin() const { return m_registry.begin(); }

  Const_iterator begin(Types type) const { return m_registry.begin(type); }

  Const_iterator end() const { return m_registry.end(); }

  Const_iterator next(Const_iterator current, Types type) const {
    return m_registry.next(current, type);
  }

#ifndef DBUG_OFF
  void dump() const { m_registry.dump(); }
#endif
};

/**
  Class used to represent the system views.

  This class is a singleton used to represent meta data of the system
  views, i.e., the views that are available through the information schema.

  @note The registry currently only stores the view names and their
        (dummy) classification.

  The singleton contains an instance of the Entity_registry class, and
  has methods that mostly delegate to this instance.
 */

class System_views {
 public:
  /*
    Classification of system views.

    Both of these types represent server INFORMATION_SCHEMA tables. The
    difference is that NON_DD_BASED_INFORMATION_SCHEMA indicates that the
    system view is based on ACL tables like role_edges, default_roles etc.
    NON_DD_BASED_INFORMATION_SCHEMA are created after creation of ACL
    tables defined in mysql_system_tables.sql and not with regular
    INFORMATION_SCHEMA tables that are created during bootstrap.

  */
  enum class Types { INFORMATION_SCHEMA, NON_DD_BASED_INFORMATION_SCHEMA };

  // Map from system view type to string description, e.g. for debugging.
  static const char *type_name(Types type) {
    switch (type) {
      case Types::INFORMATION_SCHEMA:
        return "INFORMATION_SCHEMA";
      case Types::NON_DD_BASED_INFORMATION_SCHEMA:
        return "NON_DD_BASED_INFORMATION_SCHEMA";
      default:
        return "";
    }
  }

 private:
  // The actual registry is referred and delegated to rather than
  // being inherited from.
  typedef Entity_registry<std::pair<const String_type, const String_type>,
                          const system_views::System_view, Types, type_name,
                          true>
      System_view_registry_type;
  System_view_registry_type m_registry;

 public:
  // The ordered iterator type must be public.
  typedef System_view_registry_type::Const_iterator Const_iterator;

  static System_views *instance();

  // Add predefined system views.
  void init();

  // Add a new system view by delegation to the wrapped registry.
  void add(const String_type &schema_name, const String_type &view_name,
           Types type, const system_views::System_view *view) {
    m_registry.add(schema_name, view_name, type, view);
  }

  // Find a system view by delegation to the wrapped registry.
  const system_views::System_view *find(const String_type &schema_name,
                                        const String_type &view_name) const {
    return m_registry.find_entity(schema_name, view_name);
  }

  Const_iterator begin() const { return m_registry.begin(); }

  Const_iterator begin(Types type) const { return m_registry.begin(type); }

  Const_iterator end() const { return m_registry.end(); }

  Const_iterator next(Const_iterator current, Types type) const {
    return m_registry.next(current, type);
  }

#ifndef DBUG_OFF
  void dump() const { m_registry.dump(); }
#endif
};

/**
  Class used to represent the system tablespaces.

  This class is a singleton used to represent meta data of the system
  tablespaces, i.e., the tablespaces that are predefined in the DDSE, or
  needed by the SQL layer.

  The singleton contains an instance of the Entity_registry class, and
  has methods that mostly delegate to this instance.
*/

class System_tablespaces {
 public:
  // Classification of system tablespaces.
  enum class Types {
    DD,              // For storing the DD tables.
    PREDEFINED_DDSE  // Needed by the DDSE.
  };

  // Map from system tablespace type to string description, e.g. for debugging.
  static const char *type_name(Types type) {
    switch (type) {
      case Types::DD:
        return "DD";
      case Types::PREDEFINED_DDSE:
        return "PREDEFINED_DDSE";
      default:
        return "";
    }
  }

 private:
  // The actual registry is referred and delegated to rather than
  // being inherited from.
  typedef Entity_registry<std::pair<const String_type, const String_type>,
                          const Plugin_tablespace, Types, type_name, false>
      System_tablespace_registry_type;
  System_tablespace_registry_type m_registry;

 public:
  // The ordered iterator type must be public.
  typedef System_tablespace_registry_type::Const_iterator Const_iterator;

  static System_tablespaces *instance();

  // Add a new system tablespace by delegation to the wrapped registry.
  void add(const String_type &tablespace_name, Types type,
           const Plugin_tablespace *space) {
    m_registry.add("", tablespace_name, type, space);
  }

  // Find a system tablespace by delegation to the wrapped registry.
  const Plugin_tablespace *find(const String_type &tablespace_name) const {
    return m_registry.find_entity("", tablespace_name);
  }

  Const_iterator begin() const { return m_registry.begin(); }

  Const_iterator begin(Types type) const { return m_registry.begin(type); }

  Const_iterator end() const { return m_registry.end(); }

  Const_iterator next(Const_iterator current, Types type) const {
    return m_registry.next(current, type);
  }

#ifndef DBUG_OFF
  void dump() const { m_registry.dump(); }
#endif
};
}  // namespace dd

#endif  // DD__SYSTEM_REGISTRY_INCLUDED
