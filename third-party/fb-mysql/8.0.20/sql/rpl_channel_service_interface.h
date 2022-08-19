/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_SERVICE_INTERFACE_INCLUDE
#define RPL_SERVICE_INTERFACE_INCLUDE

#include <string>

// Channel errors

#define RPL_CHANNEL_SERVICE_RECEIVER_CONNECTION_ERROR -1
#define RPL_CHANNEL_SERVICE_DEFAULT_CHANNEL_CREATION_ERROR -2
#define RPL_CHANNEL_SERVICE_SLAVE_SKIP_COUNTER_ACTIVE -3
#define RPL_CHANNEL_SERVICE_CHANNEL_DOES_NOT_EXISTS_ERROR -4
// Error for the wait event consumption, equal to the server wait for GTID
// method
#define REPLICATION_THREAD_WAIT_TIMEOUT_ERROR -1
#define REPLICATION_THREAD_WAIT_NO_INFO_ERROR -2

// Settings

// Used whenever a parameter should take the server default value
#define RPL_SERVICE_SERVER_DEFAULT -1

// Channel creation settings

/**
  Types of channels
*/
enum enum_channel_type {
  SLAVE_REPLICATION_CHANNEL,  // Master slave replication channels
  GROUP_REPLICATION_CHANNEL   // Group replication channels
};

/**
  Know parallelization options that can be applied to channel appliers
*/
enum enum_multi_threaded_workers_type {
  CHANNEL_MTS_PARALLEL_TYPE_DB_NAME,
  CHANNEL_MTS_PARALLEL_TYPE_LOGICAL_CLOCK
};

/**
 SSL information to be used when creating a channel.
 It maps the SSL options present in a CHANGE MASTER.
*/
struct Channel_ssl_info {
  int use_ssl;                 // use SSL
  char *ssl_ca_file_name;      // SSL list of trusted certificate authorities
  char *ssl_ca_directory;      // SSL certificate authorities directory
  char *ssl_cert_file_name;    // SSL connection certificate
  char *ssl_crl_file_name;     // SSL certificate revocation list
  char *ssl_crl_directory;     // SSL certificate revocation list file directory
  char *ssl_key;               // SSL key file for connections
  char *ssl_cipher;            // list of permissible ciphers to use for SSL
  int ssl_verify_server_cert;  // check the server's Common Name value
  char *tls_version;           // TLS version to use for SSL
  char *tls_ciphersuites;      // list of permissible ciphersuites for TLS 1.3
};

void initialize_channel_ssl_info(Channel_ssl_info *channel_ssl_info);

/**
 Creation information for a channel.
 It includes the data that is usually associated to a change master command
*/
struct Channel_creation_info {
  enum_channel_type type;
  char *hostname;
  int port;
  char *user;
  char *password;
  Channel_ssl_info *ssl_info;
  int auto_position;
  int channel_mts_parallel_type;
  int channel_mts_parallel_workers;
  int channel_mts_checkpoint_group;
  int replicate_same_server_id;
  int thd_tx_priority;  // The applier thread priority
  int sql_delay;
  int connect_retry;         // How many seconds to wait between retries.
  int retry_count;           // Limits the number of reconnection attempts
  bool preserve_relay_logs;  // If the logs should be preserved on creation
  char *public_key_path;     // RSA Public key information
  int get_public_key;        // Preference to get public key from donor if not
                             // available
  char *compression_algorithm;
  unsigned int zstd_compression_level;
};

void initialize_channel_creation_info(Channel_creation_info *channel_info);

// Start settings

/**
  The known types of channel threads.
  All new types should be power of 2
*/
enum enum_channel_thread_types {
  CHANNEL_NO_THD = 0,
  CHANNEL_RECEIVER_THREAD = 1,
  CHANNEL_APPLIER_THREAD = 2
};

/**
  The known until conditions that can be applied to channels
*/
enum enum_channel_until_condition {
  CHANNEL_NO_UNTIL_CONDITION,
  CHANNEL_UNTIL_APPLIER_BEFORE_GTIDS,
  CHANNEL_UNTIL_APPLIER_AFTER_GTIDS,
  CHANNEL_UNTIL_APPLIER_AFTER_GAPS,
  CHANNEL_UNTIL_VIEW_ID
};

/**
  Channel information to connect to a receiver
*/
struct Channel_connection_info {
  int until_condition;  // base on enum_channel_until_condition
  char *gtid;           // Gtids to wait on a until condition
  char *view_id;        // The view id to wait on a until condition
};

void initialize_channel_connection_info(Channel_connection_info *channel_info);

/**
  Initializes a channel connection in a similar way to a change master command.

  @note If the channel exists, it is reconfigured with the new options.
        About the logs, the preserve_relay_logs option allows the user to
        maintain them untouched.

  @param channel              The channel name
  @param channel_information  Channel creation information.

  @return the operation status
    @retval 0      OK
    @retval !=0    Error on channel creation
*/
int channel_create(const char *channel,
                   Channel_creation_info *channel_information);

/**
  Start the Applier/Receiver threads according to the given options.
  If the receiver thread is to be started, connection credential must be
  supported.

  @param channel              The channel name
  @param connection_info      Channel connection information
  @param threads_to_start     The types of threads to be started
  @param wait_for_connection  If when starting the receiver, the method should
                              wait for the connection to succeed

  @return the operation status
    @retval 0      OK
    @retval !=0    Error
 */
int channel_start(const char *channel, Channel_connection_info *connection_info,
                  int threads_to_start, int wait_for_connection);

/**
  Stops the channel threads according to the given options.

  @param channel              The channel name
  @param threads_to_stop      The types of threads to be stopped
  @param timeout              The expected time in which the thread should stop
  @return the operation status
    @retval 0      OK
    @retval !=0    Error
*/
int channel_stop(const char *channel, int threads_to_stop, long timeout);

/**
  Kills the Binlog Dump threads.

  @return the operation status
    @retval 0      OK
*/
int binlog_dump_thread_kill();

/**
  Stops all the running channel threads according to the given options.

  @param threads_to_stop      The types of threads to be stopped
  @param timeout              The expected time in which the thread should stop
  @param error_message        The returned error_message

  @return the operation status
    @retval 0      OK
    @retval !=0    Error
*/
int channel_stop_all(int threads_to_stop, long timeout,
                     std::string *error_message);
/**
  Purges the channel logs

  @param channel    The channel name
  @param reset_all  If true, the method will purge logs and remove the channel
                    If false, only the channel information will be reset.

  @return the operation status
    @retval 0      OK
    @retval !=0    Error
*/
int channel_purge_queue(const char *channel, bool reset_all);

/**
  Tells if the selected component of the channel is active or not.
  If no component is passed, this method returns if the channel exists or not

  @param channel  The channel name
  @param type     The thread that should be checked.
                  If 0, this method applies to the channel existence.

  @return is the channel (component) active
    @retval true    Yes
    @retval false   No
*/
bool channel_is_active(const char *channel, enum_channel_thread_types type);

/**
  Returns the id(s) of the channel threads: receiver or applier.
  If more than one applier exists, an array is returned, on which first
  index is coordinator thread id.

  @param[in]  channel      The channel name
  @param[in]  thread_type  The thread type (receiver or applier)
  @param[out] thread_id    The array of id(s)

  @return the number of returned ids
    @retval -1  the channel does no exists, or the thread is not present
    @retval >0 the number of thread ids returned.
*/
int channel_get_thread_id(const char *channel,
                          enum_channel_thread_types thread_type,
                          unsigned long **thread_id);

/**
  Returns last GNO from applier from a given UUID.

  @param channel the channel name
  @param sidno   the uuid associated to the desired gno

  @return the last applier gno
    @retval <0 the channel does no exists, or the applier is not present
    @retval >0 the gno
*/
long long channel_get_last_delivered_gno(const char *channel, int sidno);

/**
  Adds server executed GTID set to channel received GTID set.

  @param channel the channel name

  @return the operation status
    @retval 0      OK
    @retval != 0   Error
*/
int channel_add_executed_gtids_to_received_gtids(const char *channel);

/**
  Queues a event packet into the current active channel.

  @param channel     the channel name
  @param buf         the event buffer
  @param len         the event buffer length

  @return the operation status
    @retval 0      OK
    @retval != 0   Error on queue
*/
int channel_queue_packet(const char *channel, const char *buf,
                         unsigned long len);

/**
  Checks if all the queued transactions were executed.

  @note This method assumes that the channel is not receiving any more events.
        If it is still receiving, then the method should wait for execution of
        transactions that were present when this method was invoked.

  @param channel  the channel name
  @param timeout  the time (seconds) after which the method returns if the
                  above condition was not satisfied

  @return the operation status
    @retval 0   All transactions were executed
    @retval REPLICATION_THREAD_WAIT_TIMEOUT_ERROR     A timeout occurred
    @retval REPLICATION_THREAD_WAIT_NO_INFO_ERROR     An error occurred
*/
int channel_wait_until_apply_queue_applied(const char *channel, double timeout);

/**
  Checks if all the transactions in the given set were executed.

  @param channel  the channel name
  @param gtid_set the set in string format of transaction to wait for
  @param timeout  the time (seconds) after which the method returns if the
                  above condition was not satisfied
  @param update_THD_status     Shall the method update the THD stage

  @return the operation status
    @retval 0   All transactions were executed
    @retval REPLICATION_THREAD_WAIT_TIMEOUT_ERROR     A timeout occurred
    @retval REPLICATION_THREAD_WAIT_NO_INFO_ERROR     An error occurred
*/
int channel_wait_until_transactions_applied(const char *channel,
                                            const char *gtid_set,
                                            double timeout,
                                            bool update_THD_status = true);

/**
  Checks if the applier, and its workers when parallel applier is
  enabled, has already consumed all relay log, that is, applier is
  waiting for transactions to be queued.

  @param channel  The channel name

  @return the operation status
    @retval <0  Error
    @retval  0  Applier is not waiting
    @retval  1  Applier is waiting
*/
int channel_is_applier_waiting(const char *channel);

/**
  Checks if the applier thread, and its workers when parallel applier is
  enabled, has already consumed all relay log, that is, applier thread
  is waiting for transactions to be queued.

  @param thread_id  the applier thread id to check
  @param worker     flag to indicate if thread is a parallel worker

  @return the operation status
    @retval -1  Unable to find applier thread
    @retval  0  Applier thread is not waiting
    @retval  1  Applier thread is waiting
*/
int channel_is_applier_thread_waiting(unsigned long thread_id,
                                      bool worker = false);

/**
  Flush the channel.

  @return the operation status
    @retval 0      OK
    @retval != 0   Error on flush
*/
int channel_flush(const char *channel);

/**
  Initializes channel structures if needed.

  @return the operation status
    @retval 0      OK
    @retval != 0   Error on queue
*/
int initialize_channel_service_interface();

/**
  Returns the receiver thread retrieved GTID set in string format.

  @param      channel        The channel name.
  @param[out] retrieved_set  Pointer to pointer to string. The function will
                             set it to point to a newly allocated buffer, or
                             NULL on out of memory.

  @return the operation status
    @retval 0    OK
    @retval !=0  Error on retrieval
*/
int channel_get_retrieved_gtid_set(const char *channel, char **retrieved_set);

/**
  Tells if the selected component of the channel is stopping or not.

  @param channel  The channel name
  @param type     The thread that should be checked.

  @return is the channel (component) stopping
    @retval true    Yes
    @retval false   No, no type was specified or the channel does not exist.
*/
bool channel_is_stopping(const char *channel, enum_channel_thread_types type);

/**
  Checks if the given channel's relaylog contains a partial transaction.

  @param channel  The channel name

  @retval true    If relaylog contains partial transcation.
  @retval false   If relaylog does not contain partial transaction.
*/
bool is_partial_transaction_on_channel_relay_log(const char *channel);

/**
  Checks if any slave threads of any channel is running

  @param[in]        thread_mask       type of slave thread- IO/SQL or any

  @retval          true               atleast one channel threads are running.
  @retval          false              none of the the channels are running.
*/
bool is_any_slave_channel_running(int thread_mask);

/**
  Method to get the credentials configured for a channel

  @param[in]  channel       The channel name
  @param[out] user          The user to extract
  @param[out] password      The password to extract
  @param[out] pass_size     The password size

  @return the operation status
    @retval false   OK
    @retval true    Error, channel not found
*/
int channel_get_credentials(const char *channel, const char **user,
                            char **password, size_t *pass_size);

/**
  Return type for function
  has_any_slave_channel_open_temp_table_or_is_its_applier_running()
*/
enum enum_slave_channel_status {
  /*
    None of all slave channel appliers are running and none
    of all slave channels have open temporary table(s).
  */
  SLAVE_CHANNEL_NO_APPLIER_RUNNING_AND_NO_OPEN_TEMPORARY_TABLE = 0,
  /* At least one slave channel applier is running. */
  SLAVE_CHANNEL_APPLIER_IS_RUNNING,
  /* At least one slave channel has open temporary table(s). */
  SLAVE_CHANNEL_HAS_OPEN_TEMPORARY_TABLE
};

/**
  Checks if any slave channel applier is running or any slave channel has open
  temporary table(s). This holds handled appliers' run_locks until finding a
  running slave channel applier or a slave channel which has open temporary
  table(s), or handling all slave channels.

  @return SLAVE_CHANNEL_NO_APPLIER_RUNNING_AND_NO_OPEN_TEMPORARY_TABLE,
          SLAVE_CHANNEL_APPLIER_IS_RUNNING or
          SLAVE_CHANNEL_HAS_OPEN_TEMPORARY_TABLE.
*/
enum_slave_channel_status
has_any_slave_channel_open_temp_table_or_is_its_applier_running();

#endif  // RPL_SERVICE_INTERFACE_INCLUDE
