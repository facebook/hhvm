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

#ifndef MYSQL_REGISTRY_H
#define MYSQL_REGISTRY_H

#include <mysql/components/service.h>
#include <stdint.h>

/**
  A handle type for acquired Service.
*/
DEFINE_SERVICE_HANDLE(my_h_service);

/**
  A handle type for a iterator to a Service Implementation.
*/
DEFINE_SERVICE_HANDLE(my_h_service_iterator);
/**
  A handle type for a iterator to metadata of some Service Implementation.
*/
DEFINE_SERVICE_HANDLE(my_h_service_metadata_iterator);

/**
  Service for acquiring and releasing references to all registered Service
  Implementations.
*/
BEGIN_SERVICE_DEFINITION(registry)
/**
  Finds and acquires a Service by name. A name of the Service or the Service
  Implementation can be specified. In case of the Service name, the default
  Service Implementation for Service specified will be returned.

  @param service_name Name of Service or Service Implementation to acquire.
  @param [out] out_service Pointer to Service handle to set acquired Service.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(acquire,
                    (const char *service_name, my_h_service *out_service));
/**
  Finds a Service by name. If there is a Service Implementation with the same
  Component part of name as the input Service then the found Service is
  returned. Otherwise the default Service Implementation for specified
  Service is returned.

  @param service_name Name of Service or Service Implementation to acquire.
  @param service Service handle already acquired Service Implementation.
  @param [out] out_service Pointer to Service Implementation handle to set
    acquired Service Implementation.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(acquire_related,
                    (const char *service_name, my_h_service service,
                     my_h_service *out_service));
/**
  Releases the Service Implementation previously acquired. After the call to
  this method the usage of the Service Implementation handle will lead to
  unpredicted results.

  @param service Service Implementation handle of already acquired Service.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(release, (my_h_service service));
END_SERVICE_DEFINITION(registry)

/**
  Service for managing list of registered Service Implementations.
*/
BEGIN_SERVICE_DEFINITION(registry_registration)
/**
  Registers a new Service Implementation. If it is the first Service
  Implementation for the specified Service then it is made a default one.

  @param service_implementation_name Name of the Service Implementation to
    register.
  @param ptr Pointer to the Service Implementation structure.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(register_service, (const char *service_implementation_name,
                                       my_h_service ptr));
/**
  Removes previously registered Service Implementation from registry. If it is
  the default one for specified Service then any one still registered is made
  default. If there is no other, the default entry is removed from the
  Registry too.

  @param service_implementation_name Name of the Service Implementation to
    unregister.
  @return Status of performed operation
  @retval false success
  @retval true Failure. May happen when Service is still being referenced.
*/
DECLARE_BOOL_METHOD(unregister, (const char *service_implementation_name));
/**
  Sets new default Service Implementation for corresponding Service name.

  @param service_implementation_name Name of the Service Implementation to
    set as default one.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(set_default, (const char *service_implementation_name));
END_SERVICE_DEFINITION(registry_registration)

/**
  Service for listing all Service Implementations by iterator.
*/
BEGIN_SERVICE_DEFINITION(registry_query)
/**
  Creates iterator that iterates through all registered Service
  Implementations. If successful it leaves read lock on the Registry until
  iterator is released. The starting point of iteration may be specified
  to be on one particular Service Implementation. The iterator will move
  through all Service Implementations and additionally through all default
  Service Implementation additionally, i.e. the default Service Implementation
  will be returned twice. If no name is specified for search, iterator will be
  positioned on the first Service Implementation.

  @param service_name_pattern Name of Service or Service Implementation to
    start iteration from. May be empty string or NULL pointer, in which case
    iteration starts from the first Service Implementation.
  @param [out] out_iterator Pointer to the Service Implementation iterator
    handle.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(create, (const char *service_name_pattern,
                             my_h_service_iterator *out_iterator));
/**
  Gets name of Service pointed to by iterator. The pointer returned will last
  at least up to the moment of call to the release() method on the iterator.

  @param iterator Service Implementation iterator handle.
  @param [out] out_name Pointer to string with name to set result pointer to.
  @return Status of performed operation
  @retval false success
  @retval true Failure, may be caused when called on iterator that went
    through all values already.
*/
DECLARE_BOOL_METHOD(get, (my_h_service_iterator iter, const char **out_name));
/**
  Advances specified iterator to next element. Will succeed but return true if
  it reaches one-past-last element.

  @param iterator Service Implementation iterator handle.
  @return Status of performed operation and validity of iterator after
    operation.
  @retval false success
  @retval true Failure or called on iterator that was on last element.
*/
DECLARE_BOOL_METHOD(next, (my_h_service_iterator iter));
/**
  Checks if specified iterator is valid, i.e. have not reached one-past-last
  element.

  @param iterator Service Implementation iterator handle.
  @return Validity of iterator
  @retval false Valid
  @retval true Invalid or reached one-past-last element.
*/
DECLARE_BOOL_METHOD(is_valid, (my_h_service_iterator iter));
/**
  Releases the Service Implementations iterator. Releases read lock on the
  Registry.

  @param iterator Service Implementation iterator handle.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_METHOD(void, release, (my_h_service_iterator iter));
END_SERVICE_DEFINITION(registry_query)

/**
  Service for listing all metadata for a Service Implementation specified by
  the given iterator.
*/
BEGIN_SERVICE_DEFINITION(registry_metadata_enumerate)
/**
  Creates a iterator that iterates through all metadata for the object pointed
  by the specified iterator. If successful it leaves read lock on the registry
  until the iterator is released.

  @param iterator A iterator that points to object to get the metadata
    iterator for.
  @param [out] out_iterator Pointer to metadata iterator handle.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_BOOL_METHOD(create, (my_h_service_iterator iterator,
                             my_h_service_metadata_iterator *out_iterator));
/**
  Gets the key and value of the metadata pointed to by the specified iterator.
  The pointers returned will last at least up to the moment of call to the
  release() method on the iterator.

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
DECLARE_BOOL_METHOD(get, (my_h_service_metadata_iterator iterator,
                          const char **name, const char **value));
/**
  Advances specified iterator to next element. Will fail if it reaches
  one-past-last element.

  @param iterator Metadata iterator handle.
  @return Status of performed operation
  @retval false success
  @retval true Failure, may be caused when called on iterator that was on last
    element.
*/
DECLARE_BOOL_METHOD(next, (my_h_service_metadata_iterator iterator));
/**
  Checks if specified iterator is valid, i.e. have not reached one-past-last
  element.

  @param iterator Metadata iterator handle.
  @return Validity of iterator
  @retval false Valid
  @retval true Invalid or reached one-past-last element.
*/
DECLARE_BOOL_METHOD(is_valid, (my_h_service_metadata_iterator iterator));
/**
  Releases the specified iterator. Releases read lock on the registry.

  @param iterator Metadata iterator handle.
  @return Status of performed operation
  @retval false success
  @retval true failure
*/
DECLARE_METHOD(void, release, (my_h_service_metadata_iterator iterator));
END_SERVICE_DEFINITION(registry_metadata_enumerate)

/**
  Service to query specified metadata key directly for the specified Service
  Implementation by iterator to it.
*/
BEGIN_SERVICE_DEFINITION(registry_metadata_query)
/**
  Gets the key and value of the metadata pointed to by the specified object
  iterator. The pointer returned will last at least up to the moment of call
  to the release() method on the iterator.

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
DECLARE_BOOL_METHOD(get_value, (my_h_service_iterator iterator,
                                const char *name, const char **value));
END_SERVICE_DEFINITION(registry_metadata_query)

#endif /* MYSQL_REGISTRY_H */
