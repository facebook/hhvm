/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef GROUP_REPLICATION_MESSAGE_SERVICE_H
#define GROUP_REPLICATION_MESSAGE_SERVICE_H

#include <mysql/components/service.h>
#include <stddef.h>

/**
  @ingroup group_components_services_inventory

  A service that sends content agnostic messages from a member to the group.

  This only works if the component is on a server with group replication
  running and the member state is ONLINE. If server isn't ONLINE message won't
  be deliver.

  After message sent to all members of the group, all components that have
  registered group_replication_message_service_recv service will be notified.

  @code
  SERVICE_TYPE(registry) *plugin_registry = mysql_plugin_registry_acquire();
  my_service<SERVICE_TYPE(group_replication_message_service_send)> svc(
      "group_replication_message_service_send", plugin_registry);

  if (svc.is_valid()) {
    bool error = svc->send("tag", "payload", sizeof("payload"));
  }
  @endcode
*/
BEGIN_SERVICE_DEFINITION(group_replication_message_service_send)

/**
  This function SHALL be called whenever the caller wants to
  send an agnostic messages to the group replication stream.

  @param[in] tag              tag identifies message
  @param[in] payload          data to be deliver
  @param[in] payload_length   size of data

  @return false success, true on failure. Failure can happen if Group
  Replication status isn't ONLINE or RECOVERING  or a communication failure
  occurs.
*/
DECLARE_BOOL_METHOD(send, (const char *tag, const unsigned char *payload,
                           const size_t payload_length));

END_SERVICE_DEFINITION(group_replication_message_service_send)

/**
  A service that gets called whenever an agnostic message
  from the group replication stream is to be delivered by
  the group replication receiver thread.

  The implementation MUST NOT block the caller.

  This only works if the component is on a server with group replication
  running and the member state is ONLINE. If server isn't ONLINE message won't
  be notified about messages received.

  @code
DEFINE_BOOL_METHOD(recv, (const char *tag, const unsigned char *data,
                          size_t data_length)) {

  // If tag is of interest do something with data
  return false;
}

BEGIN_SERVICE_IMPLEMENTATION(listener_example,
                             group_replication_message_service_recv)
recv, END_SERVICE_IMPLEMENTATION();

bool register_listener() {
  SERVICE_TYPE(registry) *plugin_registry = mysql_plugin_registry_acquire();
  my_service<SERVICE_TYPE(registry_registration)> reg("registry_registration",
                                                      plugin_registry);
  using group_replication_message_service_recv_t =
      SERVICE_TYPE_NO_CONST(group_replication_message_service_recv);
  bool result = reg->register_service(
      "group_replication_message_service_recv.listener_example",
      reinterpret_cast<my_h_service>(
          const_cast<group_replication_message_service_recv_t *>(
              &SERVICE_IMPLEMENTATION(
                  listener_example,
                  group_replication_message_service_recv))));
  @endcode
*/
BEGIN_SERVICE_DEFINITION(group_replication_message_service_recv)

/**
  This function of every service implementation SHALL be called by group
  replication whenever the group replication receive a service message sent to
  the group.

  @param[out] tag              tag identifies message
  @param[out] payload          data to be deliver
  @param[out] payload_length   size of data

  @return false success, true on failure.
*/
DECLARE_BOOL_METHOD(recv, (const char *tag, const unsigned char *payload,
                           const size_t payload_length));

END_SERVICE_DEFINITION(group_replication_message_service_recv)

#endif /* GROUP_REPLICATION_MESSAGE_SERVICE_H */
