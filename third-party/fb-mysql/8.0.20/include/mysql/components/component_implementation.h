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

#ifndef COMPONENT_IMPLEMENTATION_H
#define COMPONENT_IMPLEMENTATION_H

#include <mysql/components/services/dynamic_loader.h>
#include <mysql/components/services/registry.h>
#include <cstddef>  // NULL
#include "service_implementation.h"

/**
  @page PAGE_COMPONENTS_COMPONENT A Component

  A component is a code container that contains one or more Service
  Implementations. Components can be internal (part of the MySQL Server binary)
  and external (hosted in a OS binary file different from the one of the MySQL
  Server).

  Each component will have:
  - Name
  - List of service implementations it provides.
  - List of services or service implementations it needs.
  - Initialization function that's called when a container is loaded. Takes a
    reference to the currently active service registry implementation.
  - De-initialization function that's called when a container unload is
    requested.

  Opposite to old plugin infrastructure, Components don't need linkage against
  mysqld executable, neither on Linux nor Windows. In fact, in Components we
  strongly discourage from linking against mysqld executable at all, as it
  presents significant threat to them being independent and using only
  Components infrastructure to communicate with other parts of software.

  @subpage PAGE_COMPONENTS_IMPLEMENTATION

  @page PAGE_COMPONENTS_IMPLEMENTATION MySQL Component - creating implementation
  The Component for the Dynamic Loader needs to be a dynamic library (for
  example .so or .dll) with specified entry-point method. The only exception is
  the MySQL server, for which, for now, it will be statically linked. All these
  implementation details are hidden away by macros defined in header files.
  All macros to define Services are specified in service.h, for defining Service
  Implementations in service_implementation.h and for specifying Components in
  component_implementation.h.

  @section EXAMPLE The Component example
  Example Components are located in components/example directory. These
  Components are used also for unit and MTR test, which assures these Components
  are fully functional and they quality is maintained. The example Services are
  defined in example_services.h, while implemented in three example Components
  defined in files example_component1.cc, example_component2.cc,
  and example_component3.cc. These files are specifically prepared to be used as
  examples as they provide simplicity and an extensive documentation. The
  following tutorial bases on these Components and will try to show a process to
  create similar.

  @subsection EXAMPLE_SERVICES Services defined in the example
  Example contains definition of 3 Services which will be used to show a general
  idea of services, basic implementations and simple example of one of concepts
  possible with the Component Infrastructure. These example Services are defined
  in example_services.h and are s_mysql_greetings,
  s_mysql_greetings_localization and s_mysql_example_math.

  @section TROUBLESHOOTING Common problems

  @section TUTORIAL Step by step tutorial for creating new Component
  The creation of component is a mean to get some functionality exported for the
  Components infrastructure. It can be divided into several steps:
  -# %List all high level functionalities Component is planned to have
    implemented. This will assure we know exactly what we need to benefit from
    next steps. In the example, we would like to have a "Hello, World!" string
    provider and simple math functions.
  -# Look for existing Services, that are designed specifically to provide some
    part of the new functionalities to reuse them, or other that are in any
    degree similar in functionality, design or use cases to use as an example.
    The Component infrastructure is highly oriented on reuse of Services and
    will benefit with every reuse case, as it will decrease total size of
    Services. In the example the existing base of Services is really small,
    with the core Components infrastructure Services available only leading to
    no reuse possible.
  -# Design list of functions needed to provide all functionalities. Try to make
    they follow existing patterns and ideas, possibly having some identical to
    existing ones.
  -# Try to separate groups of functions that specify some complete part of
    functionality into separate small Services to improve re-usability. Also,
    try to separate groups that seem to have more potential to be extended or
    modified in future, because changing existing Services is forbidden, in
    such a case this will lead to a lot of functions in Services that will be
    duplicates and will introduce more boilerplate code to implement them.
    Remove all functions that can be found in fully reused existing Services.
  -# Create definitions of Services, ideally one Service per file, or a group of
    really closely connected Services. In most cases you want to make these
    definitions public, in case of MySQL that means placing them in
    include/mysql/components/services/ directory to include them in mysql-dev
    package. See example_services.h, which in contrary is not made public and
    resides in example component source directory.
  -# Create declarations of all handles, the Opaque pointers for all opaque
    objects that are meant to be returned to the Services users. See usages of
    DEFINE_SERVICE_HANDLE in registry.h.
  -# Create basic structure of new Component. Use BEGIN_COMPONENT_PROVIDES,
    BEGIN_COMPONENT_REQUIRES, BEGIN_COMPONENT_METADATA, DECLARE_COMPONENT and
    DECLARE_LIBRARY_COMPONENTS. Fill in all information necessary to describe
    the new Component extensively. The example_component1.cc and
    example_component2.cc shows how structure should look like, and in
    example_component3.cc there is an example with additional Service
    references, for which placeholder definitions are kept in header file of the
    new Component, see example_component3.h. Note the placeholder for the
    Registry Service, which is available by default in every component.
  -# Create implementations of the desired Service interfaces. Implement handles
    used, as for example my_h_service_iterator_imp and
    my_h_service_metadata_iterator_imp in registry.cc. Create separate
    source and header files for each Service or closely connected group of
    Services. Remember to include the header file for Service Implementation in
    the Service Implementation source file to have no linkage problems.
    The Service Implementations in english_greeting_service_imp.cc and
    simple_example_math_imp.cc are implementations used in example_component1,
    polish_greeting_service_imp.cc and example_math_wrapping_imp.cc are
    implementations for example_component2 and example_component3 respectively.
  -# Make sure component is loaded/initialized before using its services.
    Atomic variable is_intialized represents the state of the component.
    Please check the details about the variable from validate_password_imp.cc
    file.
  -# Using registry acquire/release services inside component's init/deint
    functions is not supported. All the required services for the component
    has to be specified under BEGIN_COMPONENT_REQUIRES section only.
  .

  @file include/mysql/components/component_implementation.h
  Specifies macros to define Components.
*/

/**
  Declares a component. For specified name following macros must be executed
  earlier: BEGIN_COMPONENT_PROVIDES, BEGIN_COMPONENT_REQUIRES and
  BEGIN_COMPONENT_METADATA.
  It fills mysql_component_t structure with all of the component data. The
  info object will be named mysql_component_{source_name}.
  After this macro it is required to specify comma-separated pointers to
  initialize and deinitialize methods for components to be used during loading
  and unloading of component.

  @param source_name The source name used in other macros.
  @param name Name string with human readable name.
*/
#define DECLARE_COMPONENT(source_name, name)                        \
  mysql_component_t mysql_component_##source_name = {               \
      name, __##source_name##_provides, __##source_name##_requires, \
      __##source_name##_metadata,

/**
  A macro to end the last declaration of a Component.
*/
#define END_DECLARE_COMPONENT() }

/**
  Creates a service implementation list that are provided by specified
  component. Only a series of PROVIDES_SERVICE and PROVIDES_CUSTOM_SERVICE
  macros are expected to be used after this macro and before the
  END_COMPONENT_PROVIDES counterpart.

  @param name Component name.
*/
#define BEGIN_COMPONENT_PROVIDES(name) \
  static struct mysql_service_ref_t __##name##_provides[] = {
/**
  Declare a Service Implementation provided by a Component. It assumes standard
  Service Implementation name to be referenced.
  @sa SERVICE_IMPLEMENTATION

  @param component Component name.
  @param service A Service name for which the Service Implementation will be
    added.
*/
#define PROVIDES_SERVICE(component, service)                            \
  {                                                                     \
#service "." #component,                                            \
        const_cast < void *>                                            \
            ((const void *)&SERVICE_IMPLEMENTATION(component, service)) \
  }

/**
  A macro to end the last declaration started with the BEGIN_COMPONENT_PROVIDES.
*/
#define END_COMPONENT_PROVIDES() \
  { NULL, NULL }                 \
  }

/**
  A macro to specify requirements of the component. Creates a structure with
  a list for requirements and pointers to their placeholders.

  @param name Name of component.
*/
#define BEGIN_COMPONENT_REQUIRES_WITHOUT_REGISTRY(name) \
  static struct mysql_service_placeholder_ref_t __##name##_requires[] = {
/**
  A macro to specify requirements of the component. Creates a placeholder for
  the Registry service and structure with a list for requirements and
  pointers to their placeholders, adding the Registry service as first element.

  @param name Name of component.
*/
#define BEGIN_COMPONENT_REQUIRES(name)                                    \
  REQUIRES_SERVICE_PLACEHOLDER(registry);                                 \
  static struct mysql_service_placeholder_ref_t __##name##_requires[] = { \
      REQUIRES_SERVICE(registry),

/**
  Create a service placeholder, based on the service name.

  A service placeholder is a pointer to the service.
  It is named mysql_service_{service name}.

  This pointer is initialized by the framework upon loading a component,
  based on the component dependencies declared by @ref REQUIRES_SERVICE.

  When defining a service 'foo', in the header file for the service,
  a service placeholder is declared as follows:

  @verbatim
  extern REQUIRES_SERVICE_PLACEHOLDER(foo);
  @endverbatim

  When implementing a component 'bar', which requires the service 'foo',
  the definition of the component 'bar' should contain the following:

  @verbatim
  REQUIRES_SERVICE_PLACEHOLDER(foo);

  BEGIN_COMPONENT_REQUIRES(bar)
    REQUIRES_SERVICE(foo),
    ...
  END_COMPONENT_REQUIRES();
  @endverbatim

  The code in the implementation of service 'bar' can use
  the service placeholder pointer to invoke apis in service foo:

  @verbatim
  mysql_service_foo->some_api();
  @endverbatim

  @param service A referenced Service name.
*/
#define REQUIRES_SERVICE_PLACEHOLDER(service) \
  SERVICE_TYPE(service) * mysql_service_##service

/**
  Adds a Service requirement with a pointer to placeholder to the list of
  components.

  @param service A referenced Service name.
*/
#define REQUIRES_SERVICE(service)                                              \
  {                                                                            \
#service,                                                                  \
        static_cast < void **>                                                 \
            (static_cast <void *>(const_cast <mysql_service_##service##_t **>( \
                &mysql_service_##service)))                                    \
  }

/**
  A macro to end the last declaration started with the BEGIN_COMPONENT_REQUIRES.
*/
#define END_COMPONENT_REQUIRES() \
  { NULL, NULL }                 \
  }

/**
  A macro to specify metadata of the component. Creates a list of metadata.
  Only a series of METADATA macros are expected to be used after this macro and
  before the END_COMPONENT_METADATA counterpart.

  @param name Name of component.
*/
#define BEGIN_COMPONENT_METADATA(name) \
  static struct mysql_metadata_ref_t __##name##_metadata[] = {
/**
  Adds a Service requirement with a pointer to placeholder to the list of
  components.

  @param key A string name of the metadata to add.
  @param value A string value of the metadata to add.
*/
#define METADATA(key, value) \
  { key, value }

/**
  A macro to end the last declaration started with the BEGIN_COMPONENT_METADATA.
*/
#define END_COMPONENT_METADATA() \
  { NULL, NULL }                 \
  }

/* On Windows, exports from DLL need to be declared.
  Also, plug-in needs to be declared as extern "C" because MSVC
  unlike other compilers, uses C++ mangling for variables not only
  for functions. */
#if defined(_MSC_VER) /* Microsoft */
#ifdef __cplusplus
#define DLL_EXPORT extern "C" __declspec(dllexport)
#define DLL_IMPORT extern "C" __declspec(dllimport)
#else
#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)
#endif
#else /* non _MSC_VER */
#ifdef __cplusplus
#define DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define DLL_IMPORT
#else
#define DLL_EXPORT __attribute__((visibility("default")))
#define DLL_IMPORT
#endif
#endif

/**
  Creates a list of component implementations included in this dynamic library.
  It can be used only once in whole library. It defines an entry point method
  for library to be used with the Dynamic Loader. A list of pointers to
  Component structures is required after this macro up to the usage of
  the END_DECLARE_LIBRARY_COMPONENTS macro. Current implementation of the
  Dynamic Loader supports only one Component being specified in the library.
*/
#define DECLARE_LIBRARY_COMPONENTS \
  mysql_component_t *library_components_list = {
/**
  A macro to end the last declaration started with the
  DECLARE_LIBRARY_COMPONENTS.
*/
#define END_DECLARE_LIBRARY_COMPONENTS              \
  }                                                 \
  ;                                                 \
  DLL_EXPORT mysql_component_t *list_components() { \
    return library_components_list;                 \
  }

/**
  Defines a reference to the specified Component data info structure.
*/
#define COMPONENT_REF(name) mysql_component_##name

/**
  This is the component module entry function, used to get the component's
  structure to register the required services.
*/
#define COMPONENT_ENTRY_FUNC "list_components"

#endif /* COMPONENT_IMPLEMENTATION_H */
