/* Copyright (c) 2017, 2020, Oracle and/or its affiliates. All rights reserved.

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

/**
  @brief

  This defines the API used to call functions in logging components.
  When implementing such a service, refer to log_service_imp.h instead!

  A log service may take the shape of a writer for a specific log format
  (JSON, XML, traditional MySQL, etc.), it may implement a filter that
  removes or modifies log_items, etc.
*/

#ifndef LOG_SERVICE_H
#define LOG_SERVICE_H

#include <mysql/components/component_implementation.h>
#include <mysql/components/my_service.h>
#include <mysql/components/service_implementation.h>
#include <mysql/components/services/log_shared.h>

/* service helpers */
typedef enum enum_log_service_chistics {
  /** We do not have information about this service yet. */
  LOG_SERVICE_UNSPECIFIED = 0,

  /** Service is read-only -- it guarantees it will not modify the log-event.
      This information may later be used to e.g. run log-writers in parallel. */
  LOG_SERVICE_READ_ONLY = 1,

  /** Service is a singleton -- it may occur in the log service pipeline
      only once. */
  LOG_SERVICE_SINGLETON = 2,

  /** Service is built-in (and can not be INSTALLed/UNINSTALLed */
  LOG_SERVICE_BUILTIN = 4,

  // values from 8..64 are reserved for extensions

  /** Service is a source. It adds key/value pairs beyond those in the
      statement that first created the log-event. Log-sources are not
      normally READ_ONLY. */
  LOG_SERVICE_SOURCE = 128,

  /** Service is a filter. A filter should not be the last service in
      the log service pipeline. */
  LOG_SERVICE_FILTER = 256,

  /** Service is a sink (usually a log-writer). Sinks will normally
      not modify the log-event, but be READ_ONLY. */
  LOG_SERVICE_SINK = 512,

  /** Service is a special sink used during start-up that buffers log-events
      until the log service pipeline is fully set up, at which point we'll
      flush (that is, filter and prints) the buffered events.
      Services flagged this must also be flagged LOG_SERVICE_SINK! */
  LOG_SERVICE_BUFFER = 1024

} log_service_chistics;

BEGIN_SERVICE_DEFINITION(log_service)
/**
  Have the service process one log line.

  @param   instance  State-pointer that was returned on open.
  @param   ll        The log_line collection of log_items.

  @retval  <0        an error occurred
  @retval  =0        no work was done
  @retval  >0        number of processed entities
*/
DECLARE_METHOD(int, run, (void *instance, log_line *ll));

/**
  Flush any buffers.  This function will be called by the server
  on FLUSH ERROR LOGS.  The service may write its buffers, close
  and re-open any log files to work with log-rotation, etc.
  The flush function MUST NOT itself log anything!
  A service implementation may provide a nullptr if it does not
  wish to provide a flush function.

  @param   instance  State-pointer that was returned on open.
                     Value may be changed in flush.

  @retval  <0        an error occurred
  @retval  =0        no work was done
  @retval  >0        flush completed without incident
*/
DECLARE_METHOD(int, flush, (void **instance));

/**
  Open a new instance.

  @param   ll        optional arguments
  @param   instance  If state is needed, the service may allocate and
                     initialize it and return a pointer to it here.
                     (This of course is particularly pertinent to
                     components that may be opened multiple times,
                     such as the JSON log writer.)
                     This state is for use of the log-service component
                     in question only and can take any layout suitable
                     to that component's need. The state is opaque to
                     the server/logging framework. It must be released
                     on close.

  @retval  <0        a new instance could not be created
  @retval  =0        success, returned hande is valid
*/
DECLARE_METHOD(int, open, (log_line * ll, void **instance));

/**
  Close and release an instance. Flushes any buffers.

  @param   instance  State-pointer that was returned on open.
                     If memory was allocated for this state,
                     it should be released, and the pointer
                     set to nullptr.

  @retval  <0        an error occurred
  @retval  =0        success
*/
DECLARE_METHOD(int, close, (void **instance));

/**
  Get characteristics of a log-service.

  @retval  <0        an error occurred
  @retval  >=0       characteristics (a set of log_service_chistics flags)
*/
DECLARE_METHOD(int, characteristics, (void));
END_SERVICE_DEFINITION(log_service)

#endif
