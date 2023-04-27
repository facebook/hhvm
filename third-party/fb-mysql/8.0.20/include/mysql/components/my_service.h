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

#ifndef MY_SERVICE_H
#define MY_SERVICE_H

#include <mysql/components/service.h>
#include <mysql/components/services/registry.h>

/**
  Wraps my_h_service struct conforming ABI into RAII C++ object with ability to
  cast to desired service type.
*/
template <typename TService>
class my_service {
 public:
  /**
    Acquires service by name.

    @param name Name of service, with or without component name, to acquire.
    @param registry Handle to registry service to use. The registry service
      must be valid (i.e. not released) up to the moment when this instance
      dies.
  */
  my_service(const char *name, SERVICE_TYPE(registry) * registry)
      : m_registry(registry) {
    if (registry->acquire(name, &m_service)) {
      /* NULLed service handle means no valid service managed. */
      m_service = {};
    }
  }
  /**
    Acquires service by name.

    @param name Name of service, with or without component name, to acquire.
    @param related_service Handle to service to acquire related to.
    @param registry Handle to registry service to use.
  */
  my_service(const char *name, my_h_service related_service,
             SERVICE_TYPE(registry) * registry)
      : m_registry(registry) {
    if (registry->acquire_related(name, related_service, &m_service)) {
      /* NULLed service handle means no valid service managed. */
      m_service = nullptr;
    }
  }
  /**
    Wraps service implementation already acquired.

    @param service Service handle to manage.
    @param registry Handle to registry service to use.
  */
  my_service(my_h_service service, SERVICE_TYPE(registry) * registry)
      : m_service(service), m_registry(registry) {}

  my_service(const my_service<TService> &other) = delete;

  my_service(my_service<TService> &&other)
      : m_service(other.m_service), m_registry(other.m_registry) {
    other.m_service = nullptr;
  }

  ~my_service() {
    if (this->is_valid()) {
      m_registry->release(m_service);
    }
  }

  operator TService *() const {
    return reinterpret_cast<TService *>(m_service);
  }

  operator my_h_service() const { return m_service; }
  /**
    Returns managed service typed as desired service type to execute
    operations specified after -> on it.
  */
  TService *operator->() const { return static_cast<TService *>(*this); }
  /**
    @retval false Object manages valid service.
    @retval true Object does not manage any service.
  */
  operator bool() const { return !is_valid(); }
  bool is_valid() const {
    /* NULLed service handle means no valid service managed. */
    return static_cast<const my_h_service_imp *>(this->m_service) != nullptr;
  }

 private:
  my_h_service m_service;
  SERVICE_TYPE(registry) * m_registry;
};

#endif /* MY_SERVICE_H */
