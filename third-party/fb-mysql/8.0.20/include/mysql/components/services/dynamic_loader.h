/* Copyright (c) 2016, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MYSQL_DYNAMIC_LOADER_H
#define MYSQL_DYNAMIC_LOADER_H

#include <mysql/components/service.h>

/**
  A handle type for a iterator to a Component.
*/
DEFINE_SERVICE_HANDLE(my_h_component_iterator);
/**
  A handle type for a iterator to metadata of some Component.
*/
DEFINE_SERVICE_HANDLE(my_h_component_metadata_iterator);

/**
  Service for managing the list of loaded Components.
*/
BEGIN_SERVICE_DEFINITION(dynamic_loader)
/**
  Loads specified group of components by URN, initializes them and
  registers all Service Implementations present in these components.
  Assures all dependencies will be met after loading specified components.
  The dependencies may be circular, in such case it's necessary to specify
  all components on cycle to load in one batch. From URNs specified the
  scheme part of URN (part before "://") is extracted and used to acquire
  Service Implementation of scheme component loader Service for specified
  scheme.

  @param urns List of URNs of components to load.
  @param component_count Number of components on list to load.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(load, (const char *urns[], int component_count));
/**
  Unloads specified group of components by URN, deinitializes them and
  unregisters all Service Implementations present in these components.
  Assumes, thous does not check it, all dependencies of not unloaded
  components will still be met after unloading specified components.
  The dependencies may be circular, in such case it's necessary to specify
  all components on cycle to unload in one batch. From URNs specified the
  scheme part of URN (part before "://") is extracted and used to acquire
  Service Implementation of scheme component loader Service for specified
  scheme.

  @param urns List of URNs of components to unload.
  @param component_count Number of components on list to unload.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(unload, (const char *urns[], int component_count));
END_SERVICE_DEFINITION(dynamic_loader)

/**
  Service for listing all Components by iterator.
*/
BEGIN_SERVICE_DEFINITION(dynamic_loader_query)
/**
  Creates iterator that iterates through all loaded components.
  If successful it leaves read lock on dynamic loader until iterator is
  released.

  @param [out] out_iterator Pointer to component iterator handle.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(create, (my_h_component_iterator * out_iterator));
/**
  Gets name and URN of Service pointed to by iterator.

  @param iterator Component iterator handle.
  @param [out] out_name Pointer to string with component name to set result
    pointer to.
  @param [out] out_urn Pointer to string with URN from which the component was
    loaded from, to set result pointer to.
  @return Status of performed operation
  @retval false success
  @retval true Failure, may be caused when called on iterator that went
    through all values already.
*/
DECLARE_BOOL_METHOD(get, (my_h_component_iterator iter, const char **out_name,
                          const char **out_urn));
/**
  Advances specified iterator to next element. Will succeed but return true if
  it reaches one-past-last element.

  @param iterator Component iterator handle.
  @return Status of performed operation and validity of iterator after
    operation.
  @retval false success
  @retval true Failure or called on iterator that was on last element.
*/
DECLARE_BOOL_METHOD(next, (my_h_component_iterator iter));
/**
  Checks if specified iterator is valid, i.e. have not reached one-past-last
  element.

  @param iterator Component iterator handle.
  @return Validity of iterator
  @retval false Valid
  @retval true Invalid or reached one-past-last element.
*/
DECLARE_BOOL_METHOD(is_valid, (my_h_component_iterator iter));
/**
  Releases component iterator. Releases read lock on dynamic loader.

  @param iterator Component iterator handle.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_METHOD(void, release, (my_h_component_iterator iter));
END_SERVICE_DEFINITION(dynamic_loader_query)

/**
  Service for listing all metadata for a Component specified by the iterator.
*/
BEGIN_SERVICE_DEFINITION(dynamic_loader_metadata_enumerate)
/**
  Creates iterator that iterates through all metadata for object pointed by
  the specified iterator. If successful it leaves read lock on the registry
  until the iterator is released.

  @param iterator A iterator that points to object to get the metadata
    iterator for.
  @param [out] out_iterator Pointer to metadata iterator handle.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(create, (my_h_component_iterator iterator,
                             my_h_component_metadata_iterator *out_iterator));
/**
  Gets the key and value of the metadata pointed to by the specified iterator.

  @param iterator Metadata iterator handle.
  @param [out] out_name A pointer to the string with the key to set the result
    pointer to.
  @param [out] out_value A pointer to the string with the metadata value to
    set the result pointer to.
  @return Status of performed operation
  @retval false success
  @retval true Failure, may be caused when called on the iterator that went
    through all values already.
*/
DECLARE_BOOL_METHOD(get, (my_h_component_metadata_iterator iterator,
                          const char **name, const char **value));
/**
  Advances specified iterator to next element. Will fail if it reaches
  one-past-last element.

  @param iterator Metadata iterator handle.
  @return Status of performed operation
  @retval false success
  @retval true Failure, may be caused when called on iterator that was on the
    last element.
*/
DECLARE_BOOL_METHOD(next, (my_h_component_metadata_iterator iterator));
/**
  Checks if specified iterator is valid, i.e. have not reached one-past-last
  element.

  @param iterator Metadata iterator handle.
  @return Validity of iterator
  @retval false Valid
  @retval true Invalid or reached one-past-last element.
*/
DECLARE_BOOL_METHOD(is_valid, (my_h_component_metadata_iterator iterator));
/**
  Releases the specified iterator. Releases read lock on the registry.

  @param iterator Metadata iterator handle.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_METHOD(void, release, (my_h_component_metadata_iterator iterator));
END_SERVICE_DEFINITION(dynamic_loader_metadata_enumerate)

/**
  Service to query specified metadata key directly for the specified Component
  by iterator to it.
*/
BEGIN_SERVICE_DEFINITION(dynamic_loader_metadata_query)
/**
  Gets the key and value of the metadata pointed to by the specified object
  iterator.

  @param iterator A iterator that points to object to get the metadata
    iterator for.
  @param name A pointer to the string with the key to set the result
    pointer to.
  @param [out] out_value A pointer to the string with the metadata value to
    set the result pointer to.
  @return Status of performed operation
  @retval false success
  @retval true Failure, may be caused when called on the iterator that went
    through all values already.
*/
DECLARE_BOOL_METHOD(get_value, (my_h_component_iterator iterator,
                                const char *name, const char **value));
END_SERVICE_DEFINITION(dynamic_loader_metadata_query)

/**
  Carries information on specific Service Implementation.
*/
struct mysql_service_ref_t {
  const char *name;
  void *implementation;
};

/**
  Carries information on the specific Service requirement for some Component and
  a pointer to member where to store the acquired Service Implementation to
  satisfy this requirement.
*/
struct mysql_service_placeholder_ref_t {
  const char *name;
  void **implementation;
};

/**
  Specifies a key and value pair of the single Component metadata.
*/
struct mysql_metadata_ref_t {
  const char *key;
  const char *value;
};

/**
  Carries information on the specific Component, all Service Implementations it
  provides, all its requirements and metadata.
*/
#ifdef __clang__
struct __attribute__((visibility("default"))) mysql_component_t {
#else
struct mysql_component_t {
#endif
  const char *name;
  struct mysql_service_ref_t *provides;
  struct mysql_service_placeholder_ref_t *requires;
  struct mysql_metadata_ref_t *metadata;
  mysql_service_status_t (*init)();
  mysql_service_status_t (*deinit)();
};

/**
  Service for providing Components from a specified scheme of URN.

  All scheme loading Services are the same although they have separate names
  (aliased to the main type) to allow a single component implement several
  scheme loaders, to not break the recommendation to keep implementation names
  the same as the component name, and to be able to create wrappers and other
  solutions that require to have multiple implementations of a single type.
*/
BEGIN_SERVICE_DEFINITION(dynamic_loader_scheme)
/**
  Loads all Components that are located under the URN specified.

  @param urn URN to location of the component to load from.
  @param [out] out_data Pointer to pointer to MySQL component data structures
    to set result components data retrieved from specified file.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(load, (const char *urn, mysql_component_t **out_data));
/**
  Unloads all Components that were previously loaded.

  @param urn URN to location to unload all components from.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(unload, (const char *urn));
END_SERVICE_DEFINITION(dynamic_loader_scheme)

#endif /* MYSQL_DYNAMIC_LOADER_H */
