/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef RPL_LOG_ENCRYPTION_INCLUDED
#define RPL_LOG_ENCRYPTION_INCLUDED

#include <openssl/evp.h>
#include <sql/stream_cipher.h>
#include <stdint.h>
#include <map>
#include <string>
#include "my_inttypes.h"

class Basic_istream;
class Basic_ostream;
class THD;

/**
  @file rpl_log_encryption.h

  @brief This file includes the major components for encrypting/decrypting
         binary log files.

  * Replication logs

    Here, replication logs includes both the binary and relay log files.

  * File Level Encryption

    - All standard binary log file data (including BINLOG_MAGIC) in replication
      logs are encrypted.

    - A replication log file is either encrypted or not (standard binary log
      file). It is not possible that part of a log file is encrypted and part
      of it is non-encrypted.

    - There is an encryption header in the begin of each encrypted replication
      log file.

      <pre>
            +--------------------+
            |  Encryption Header |
            +--------------------+
            |  Encrypted Data    |
            +--------------------+
      </pre>

      The encrypted replication file header includes necessary information to
      decrypt the encrypted data of the file (the standard binary log file
      data). For detail, check Rpl_encryption_header class.

  * Two Tier Keys

    Replication logs are encrypted with two tier keys. A 'File Password' for
    encrypting the standard binary log file data and a 'Replication Encryption
    Key' for encrypting the 'File Password'.

    - File password

      Each replication log file has a password. A file key used to encrypt the
      file is generated from the file password. The encrypted 'File Password'
      is stored into encryption header of the file. For details, check
      Rpl_encryption_header class.

    - Replication encryption key

      A replication encryption key is used to encrypt/decrypt the file
      password stored in an encrypted replication file header. It is generated
      by keyring and stored in/retrieved from keyring.
*/
#ifdef MYSQL_SERVER

/**
  The Rpl_encryption class is the container for the binlog encryption feature
  generic and server instance functions.
*/
class Rpl_encryption {
 public:
  struct Rpl_encryption_key {
    std::string m_id;
    Key_string m_value;
  };

  Rpl_encryption() = default;
  Rpl_encryption(const Rpl_encryption &) = delete;
  Rpl_encryption(Rpl_encryption &&) = delete;
  Rpl_encryption &operator=(const Rpl_encryption &) = delete;
  Rpl_encryption &operator=(Rpl_encryption &&) = delete;

  enum class Keyring_status {
    SUCCESS = 0,
    KEYRING_ERROR_FETCHING = 1,
    KEY_NOT_FOUND = 2,
    UNEXPECTED_KEY_SIZE = 3,
    UNEXPECTED_KEY_TYPE = 4,
    KEY_EXISTS_UNEXPECTED = 5,
    KEYRING_ERROR_GENERATING = 6,
    KEYRING_ERROR_STORING = 7,
    KEYRING_ERROR_REMOVING = 8,
  };
  /**
    A wrapper function to throw a binlog encryption keyring error.
    The wrapper will decide if the error will be reported to the client session
    or to the server error log according to current_thd.

    @param error The Keyring_status to be reported.
  */
  static void report_keyring_error(Keyring_status error);
  /**
    A wrapper function to throw a replication logs encryption keyring error,
    reporting also the key ID.
    The wrapper will decide if the error will be reported to the client session
    or to the server error log according to current_thd.

    @param error The Keyring_status to be reported.
    @param key_id The key ID to appear in the error message.
  */
  static void report_keyring_error(Keyring_status error, const char *key_id);

  /**
    Replication encryption master key rotation process is recoverable. The
    steps defined in the enum class below are the steps from which the rotation
    process may continue after an unexpected interruption.
  */
  enum class Key_rotation_step {
    START,
    DETERMINE_NEXT_SEQNO,
    GENERATE_NEW_MASTER_KEY,
    REMOVE_MASTER_KEY_INDEX,
    STORE_MASTER_KEY_INDEX,
    ROTATE_LOGS,
    PURGE_UNUSED_ENCRYPTION_KEYS,
    REMOVE_KEY_ROTATION_TAG
  };

  /**
    Initialize the rpl_encryption instance. This initialization shall be called
    after generating/loading the server UUID and before opening new binary and
    relay log files for writing.

    When the replication_logs_encrypt option is on at server startup, the
    initialization process will try to recover master key and may generate
    a new replication master key if needed.

    @retval false Success.
    @retval true Error.
  */
  bool initialize();
  /**
    Remove remaining old/new master key index in order to cleanup any previous
    master key rotation.

    @retval false Success.
    @retval true Error.
  */
  bool remove_remaining_seqnos_from_keyring();
  /**
    Recover the replication encryption master key from keyring.

    The recovery of the master key process starts by trying to read the
    replication master key information from keyring (the master key sequence
    number, and the master key itself).

    Then, if detected that a key rotation did not completed properly, tries to
    continue the master key rotation.

    When recovery is successful, the m_master_key_recovered flag is set true.

    @retval false Success.
    @retval true Error.
  */
  bool recover_master_key();
  /**
    Return the current replication encryption master key.

    @return The current replication encryption master key.
  */
  const Rpl_encryption_key get_master_key();

  /**
    Get the key with given key ID. The key to be returned will be retrieved
    from the keyring or from a cached copy in memory.

    @param[in] key_id ID of the key to be returned.
    @param[in] key_type Expected type of the key to be returned.

    @return A pair containing the status of the operation (Keyring_status) and
            a Key_string. Errors shall be checked by consulting the status.
  */
  static std::pair<Keyring_status, Key_string> get_key(
      const std::string &key_id, const std::string &key_type);

  /**
    Get the key with given key ID. The key to be returned will be retrieved
    from the keyring or from a cached copy in memory.

    @param[in] key_id ID of the key to be returned.
    @param[in] key_type Expected type of the key to be returned.
    @param[in] key_size Expected size of the key to be returned.

    @return A pair containing the status of the operation (Keyring_status) and
            a Key_string. Errors shall be checked by consulting the status.
  */
  static std::pair<Keyring_status, Key_string> get_key(
      const std::string &key_id, const std::string &key_type, size_t key_size);

  /**
    Enable binlog encryption option. It will generate a new global key if
    there is no master key yet. Then rotate replication logs to make encryption
    effective immediately.

    Replication logs rotation errors don't fail, but they will throw a warning.

    @param[in] thd the thd object of the session.

    @retval false Success.
    @retval true Error. If error happens when generating new key, it will fail.
  */
  bool enable(THD *thd);
  /**
    Disable binlog encryption option. It rotates replication logs to make
    encryption ineffective immediately.

    Replication logs rotation errors don't fail, but they will throw a warning.

    @param[in] thd the thd object of the session.
  */
  void disable(THD *thd);
  /**
    Return is the replication logs encryption feature is enabled.

    @retval false The feature is disabled.
    @retval true The feature is enabled.
  */
  bool is_enabled();
  const bool &get_enabled_var();
  const bool &get_master_key_rotation_at_startup_var();
  /**
    Purge unused master keys from Keyring.

    @retval false Success.
    @retval true Error.
  */
  bool purge_unused_keys();
  /**
    Rotate the master key.

    @param step Step to start the process (it might be recovering).
    @param new_master_key_seqno When recovering, this is the new master key
                                sequence number detected by recovery process.
    @retval false Success.
    @retval true Error.
  */
  bool rotate_master_key(Key_rotation_step step = Key_rotation_step::START,
                         uint32_t new_master_key_seqno = 0);

 private:
  /* Define the keyring key type for keys storing sequence numbers */
  static const char *SEQNO_KEY_TYPE;
  /* Define the keyring key length for keys storing sequence numbers */
  static const int SEQNO_KEY_LENGTH = 16;
  /*
    Sys_binlog_encryption uses m_enabled as the storage of global var
    binlog_encryption.
  */
  bool m_enabled = false;
  /*
    Sys_binlog_rotate_encryption_master_key_at_startup uses
    m_rotate_at_startup as the storage of global var
    binlog_rotate_encryption_master_key_at_startup.
  */
  bool m_rotate_at_startup = false;
#ifndef DBUG_OFF
  /*
    This variable is only used to assert that enable(), disable() and
    get_master_key() functions are called only after initialize() was called.
  */
  bool m_initialized = false;
#endif
  /*
    The replication logs encryption only needs to recover the current
    replication master key if the binlog_encryption option is enabled.

    This flag will be set true after a successful replication master key
    recovery.
  */
  bool m_master_key_recovered = false;
  /* The sequence number of the replication master key. */
  uint32_t m_master_key_seqno = 0;
  /* The current replication master key */
  Rpl_encryption_key m_master_key;
  /*
    Flag to avoid double logs rotation when enabling the option and
    recovering from master key rotation.
  */
  bool m_skip_logs_rotation = false;

  /**
    Fetch a key from keyring. When error happens, it either reports an error to
    user or write an error to log accordingly.

    @param[in] key_id ID of the key to be returned.
    @param[in] key_type Expected type of the key to be returned.

    @return A tuple containing the status of the operation (Keyring_status), a
            pointer to the fetched key (nullptr if the key was not fetched) and
            the returned key size. Errors shall be checked by consulting the
            status.
  */
  static std::tuple<Keyring_status, void *, size_t> fetch_key_from_keyring(
      const std::string &key_id, const std::string &key_type);

  /**
    Rotate replication logs excluding relay logs of group replication channels.
    If error happens, it will either report a warning to session user.

    @param[in] thd The thd object of current session.
  */
  void rotate_logs(THD *thd);

  /**
    Get a sequence number from the keyring. The sequence number to be returned
    will be extracted from the key retrieved from the keyring. No caching shall
    be used for this function.

    @param[in] key_id ID of the key to extract the sequence number from.

    @return A pair containing the status of the operation (Keyring_status) and
            a sequence number. Errors shall be checked by consulting the status.
  */
  std::pair<Rpl_encryption::Keyring_status, uint32_t> get_seqno_from_keyring(
      std::string key_id);
  /**
    Set a sequence number into a key and store it into keyring.

    @param[in] key_id ID of the key to set the sequence number.
    @param[in] seqno The sequence number to be set.

    @retval false Success.
    @retval true Error.
  */
  bool set_seqno_on_keyring(std::string key_id, uint32_t seqno);
  /**
    Remove a key from the keyring.

    @param[in] key_id ID of the key to be removed from keyring.

    @retval false Success.
    @retval true Error.
  */
  bool remove_key_from_keyring(std::string key_id);
  /**
    Returns the key ID of the keyring key that stores the master key sequence
    number.

    @return The key ID.
  */
  std::string get_master_key_seqno_key_id();
  /**
    Get the master key sequence number from keyring.

    @return A pair containing the status of the operation (Keyring_status) and
            a sequence number. Errors shall be checked by consulting the status.
  */
  std::pair<Rpl_encryption::Keyring_status, uint32_t>
  get_master_key_seqno_from_keyring();
  /**
    Set the master key sequence number into a key and store it into keyring.

    @retval false Success.
    @retval true Error.
  */
  bool set_master_key_seqno_on_keyring(uint32 seqno);
  /**
    Remove the master key sequence number key from the keyring.

    @retval false Success.
    @retval true Error.
  */
  bool remove_master_key_seqno_from_keyring();
  /**
    Returns the key ID of the keyring key that stores the "new" master key
    sequence number.

    @return The key ID.
  */
  std::string get_new_master_key_seqno_key_id();
  /**
    Returns the key ID of the keyring key that stores the "last_purged"
    master key sequence number.

    @return The key ID.
  */
  std::string get_last_purged_master_key_seqno_key_id();
  /**
    Returns the key ID of the keyring key that stores the "old" master key
    sequence number.

    @return The key ID.
  */
  std::string get_old_master_key_seqno_key_id();
  /**
    Get the "new" master key sequence number from keyring.

    @return A pair containing the status of the operation (Keyring_status) and
            a sequence number. Errors shall be checked by consulting the status.
  */
  std::pair<Rpl_encryption::Keyring_status, uint32_t>
  get_new_master_key_seqno_from_keyring();
  /**
    Get the "old" master key sequence number from keyring.

    @return A pair containing the status of the operation (Keyring_status) and
            a sequence number. Errors shall be checked by consulting the status.
  */
  std::pair<Rpl_encryption::Keyring_status, uint32_t>
  get_old_master_key_seqno_from_keyring();
  /**
    Get the "last_purged" master key sequence number from keyring.

    @return A pair containing the status of the operation (Keyring_status) and
            a sequence number. Errors shall be checked by consulting the status.
  */
  std::pair<Rpl_encryption::Keyring_status, uint32_t>
  get_last_purged_master_key_seqno_from_keyring();
  /**
    Set the "new" master key sequence number into a key and store it into
    keyring.

    @retval false Success.
    @retval true Error.
  */
  bool set_new_master_key_seqno_on_keyring(uint32 seqno);
  /**
    Set the "last_purged" master key sequence number into a key and store it
    into keyring.

    @retval false Success.
    @retval true Error.
  */
  bool set_last_purged_master_key_seqno_on_keyring(uint32 seqno);
  /**
    Set the "old" master key sequence number into a key and store it into
    keyring.

    @retval false Success.
    @retval true Error.
  */
  bool set_old_master_key_seqno_on_keyring(uint32 seqno);
  /**
    Remove the "new" master key sequence number key from the keyring.

    @retval false Success.
    @retval true Error.
  */
  bool remove_new_master_key_seqno_from_keyring();
  /**
    Remove the "last_purged" master key sequence number key from the keyring.

    @retval false Success.
    @retval true Error.
  */
  bool remove_last_purged_master_key_seqno_from_keyring();
  /**
    Remove the "old" master key sequence number key from the keyring.

    @retval false Success.
    @retval true Error.
  */
  bool remove_old_master_key_seqno_from_keyring();
  /**
    Generate a new replication master key on keyring and retrieve it.

    @param[in] seqno The sequence number of the master key.

    @retval false Success.
    @retval true Error.
  */
  bool generate_master_key_on_keyring(uint32 seqno);
};

extern Rpl_encryption rpl_encryption;
#endif  // MYSQL_SERVER

/**
  @class Rpl_encryption_header

  This is the base class to serialize and deserialize a replication log file
  encryption header.

  The new encrypted binary log file format is composed of two parts:

  <pre>
      +---------------------+
      |  Encryption Header  |
      +---------------------+
      |   Encrypted Data    |
      +---------------------+
  </pre>

  The encryption header exists only in the begin of encrypted replication log
  files.

  <pre>
    +------------------------+----------------------------------------------+
    | MAGIC HEADER (4 bytes) | Replication logs encryption version (1 byte) |
    +------------------------+----------------------------------------------+
    |                Version specific encryption header data                |
    +-----------------------------------------------------------------------+
                             Encryption Header Format
  </pre>

  <table>
  <caption>Encryption Header Format</caption>
  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>
  <tr>
    <td>Magic Header</td>
    <td>4 Bytes</td>
    <td>
      The content is always 0xFD62696E. It is similar to Binlog Magic Header.
      Binlog magic header is: 0xFE62696e.
    </td>
  <tr>
    <td>Replication logs encryption version</td>
    <td>1 Byte</td>
    <td>
      The replication logs encryption version defines how the header shall be
      deserialized and how the Encrypted Data shall be decrypted.
    </td>
  </tr>
  <tr>
    <td>Version specific encryption data header</td>
    <td>Depends on the version field</td>
    <td>
      Data required to fetch a replication key from keyring and deserialize
      the Encrypted Data.
    </td>
  </tr>
  </table>
*/
class Rpl_encryption_header {
 public:
  /* Same as BINLOG_MAGIC_SIZE */
  static const int ENCRYPTION_MAGIC_SIZE = 4;
  /* The magic for an encrypted replication log file */
  static const char *ENCRYPTION_MAGIC;

  virtual ~Rpl_encryption_header();

  /**
    Deserialize the replication encrypted log file header from the given stream.
    This function shall be called right after reading the magic from the stream.
    It will read the version of the encrypted log file header, instantiate a
    proper Rpl_encryption_header based on version and delegate the rest of the
    header deserialization to the new instance.

    @param istream The stream containing the header to deserialize.

    @return A Rpl_encryption_header on success or nullptr on failure.
  */
  static std::unique_ptr<Rpl_encryption_header> get_header(
      Basic_istream *istream);
  /**
    Generate a new replication encryption header based on the default
    replication encrypted log file header version.

    @return A Rpl_encryption_header of default version.
  */
  static std::unique_ptr<Rpl_encryption_header> get_new_default_header();
  /**
    Serialize the header into an output stream.

    @param ostream The output stream to serialize the header.

    @retval false Success.
    @retval true Error.
  */
  virtual bool serialize(Basic_ostream *ostream) = 0;
  /**
    Deserialize encryption header from a stream.

    @param[in] istream The input stream for deserializing the encryption
                       header.

    @retval false Success.
    @retval true Error.
  */
  virtual bool deserialize(Basic_istream *istream) = 0;
  /**
    Get the header version.

    @return The header version.
  */
  virtual char get_version() const = 0;
  /**
    Return the header size to be taken into account when serializing an
    deserializing encrypted file headers from replication log files.

    @return The size of the header for the header version.
  */
  virtual int get_header_size() = 0;
  /**
    Decrypt the file password.
  */
  virtual Key_string decrypt_file_password() = 0;
  /**
    Factory to generate ciphers to encrypt streams based on current header.

    @return A Stream_cipher for this header version or nullptr on failure.
  */
  virtual std::unique_ptr<Stream_cipher> get_encryptor() = 0;
  /**
    Factory to generate ciphers to decrypt streams based on current header.

    @return A Stream_cipher for this header version or nullptr on failure.
  */
  virtual std::unique_ptr<Stream_cipher> get_decryptor() = 0;
  /**
    Setup the header with current master key and generates a new random file
    password. This function shall be called when creating new replication
    log files.

    @return The new file password, or an empty password if error happens.
  */
  virtual Key_string generate_new_file_password() = 0;
#ifdef MYSQL_SERVER
  /**
    Encrypt a file password using current replication encryption master key.

    @param[in] password_str The plain file password.

    @retval false Success.
    @retval true Error.
  */
  virtual bool encrypt_file_password(Key_string password_str) = 0;
#endif
  /**
    Build a key id prefix using default header version.

    @return A key ID prefix.
  */
  static std::string key_id_prefix();
  /**
    Build a key id using the given sequence number using default header version.

    @param[in] seqno The sequence number used to build key id.

    @return A key ID with a sequence number.
  */
  static std::string seqno_to_key_id(uint32_t seqno);
  /**
    Build a key id using the given suffix using default header version.

    @param[in] suffix The suffix used to build key id.

    @return A key ID with a suffix.
  */
  static std::string key_id_with_suffix(const char *suffix);
  /**
    Return the default header version encryption key type.

    @return The encrypted key type.
  */
  static const char *get_key_type();

 protected:
  /* Offset of the version field in the header */
  static const int VERSION_OFFSET = ENCRYPTION_MAGIC_SIZE;
  /* Size of the version field in the header */
  static const int VERSION_SIZE = 1;
  /* Offset of the optional header fields in the header */
  static const int OPTIONAL_FIELD_OFFSET = VERSION_OFFSET + VERSION_SIZE;

 private:
  /* The default header version for new headers */
  static const char m_default_version = 1;
};

/**
  @class Rpl_encryption_header_v1

  <pre>
    +------------------------+----------------------------------------------+
    | MAGIC HEADER (4 bytes) | Replication logs encryption version (1 byte) |
    +------------------------+----------------------------------------------+
    |             Replication Encryption Key ID (60 to 69 bytes)            |
    +-----------------------------------------------------------------------+
    |                   Encrypted File Password (33 bytes)                  |
    +-----------------------------------------------------------------------+
    |               IV For Encrypting File Password (17 bytes)              |
    +-----------------------------------------------------------------------+
    |                       Padding (388 to 397 bytes)                      |
    +-----------------------------------------------------------------------+
                Encrypted binary log file header format version 1
  </pre>

  <table>
  <caption>Encrypted binary log file header format version 1</caption>
  <tr>
    <th>Name</th>
    <th>Format</th>
    <th>Description</th>
  </tr>
  <tr>
    <td>Replication Encryption Key ID</td>
    <td>
     Variable length field that uses Type, Length, Value (TLV) format. Type
     takes 1 byte. Length takes 1 byte. Values takes Length bytes.
    </td>
    <td>
      ID of the key that shall be retrieved from keyring to be used to decrypt
      the file password field.
    </td>
  </tr>
  <tr>
    <td>Encrypted File Password</td>
    <td>
      Fixed length field that uses Type, Value format. Type takes 1 byte.
      Value takes 32 bytes.</td>
    <td>It is the encrypted file password.</td>
  </tr>
  <tr>
    <td>IV for Encrypting File Password</td>
    <td>
      Fixed length field that uses Type, Value format. Type takes 1 byte.
      Value takes 16 bytes.</td>
    <td>
      The iv, together with the key, is used to encrypt/decrypt the
      file password.
    </td>
  </tr>
  <tr>
    <td>Padding</td>
    <td>Variable length, all bytes are 0.</td>
    <td>
      Encryption header has 512 bytes. Above fields don't take all bytes. All
      unused bytes are filled with 0 as padding.
    </td>
  </tr>
  </table>
*/
class Rpl_encryption_header_v1 : public Rpl_encryption_header {
 public:
  static const char *KEY_TYPE;
  static const int KEY_LENGTH = 32;
  static const int HEADER_SIZE = 512;
  static const int IV_FIELD_SIZE = 16;
  static const int PASSWORD_FIELD_SIZE = 32;

  Rpl_encryption_header_v1() = default;

  ~Rpl_encryption_header_v1() override;

  bool serialize(Basic_ostream *ostream) override;
  bool deserialize(Basic_istream *istream) override;
  char get_version() const override;
  int get_header_size() override;
  Key_string decrypt_file_password() override;
  std::unique_ptr<Stream_cipher> get_encryptor() override;
  std::unique_ptr<Stream_cipher> get_decryptor() override;
  Key_string generate_new_file_password() override;
#ifdef MYSQL_SERVER
  bool encrypt_file_password(Key_string password_str) override;
#endif

  /**
    Build a key id prefix.
  */
  static std::string key_id_prefix();
  /**
    Build a key id using the given sequence number.

    @param[in] seqno The sequence number used to build key id.
  */
  static std::string seqno_to_key_id(uint32_t seqno);
  /**
    Build a key id using the given suffix.

    @param[in] suffix The suffix used to build key id.
  */
  static std::string key_id_with_suffix(const char *suffix);

 private:
  /* The prefix for key IDs */
  static const char *KEY_ID_PREFIX;
  /* Expected field types */
  enum Field_type {
    KEY_ID = 1,
    ENCRYPTED_FILE_PASSWORD = 2,
    IV_FOR_FILE_PASSWORD = 3
  };
  /* This header implementation version */
  char m_version = 1;
  /* The key ID of the keyring key that encrypted the password */
  std::string m_key_id;
  /* The encrypted file password */
  Key_string m_encrypted_password;
  /* The IV used to encrypt/decrypt the file password */
  Key_string m_iv;
};
#endif  // RPL_LOG_ENCRYPTION_INCLUDED
