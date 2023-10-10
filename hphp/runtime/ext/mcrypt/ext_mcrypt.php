<?hh

const string MCRYPT_3DES = "tripledes";
const string MCRYPT_ARCFOUR = "arcfour";
const string MCRYPT_ARCFOUR_IV = "arcfour-iv";
const string MCRYPT_BLOWFISH = "blowfish";
const string MCRYPT_BLOWFISH_COMPAT = "blowfish-compat";
const string MCRYPT_CAST_128 = "cast-128";
const string MCRYPT_CAST_256 = "cast-256";
const string MCRYPT_CRYPT = "crypt";
const int MCRYPT_DECRYPT = 1;
const string MCRYPT_DES = "des";
const int MCRYPT_ENCRYPT = 0;
const string MCRYPT_ENIGNA = "crypt";
const string MCRYPT_GOST = "gost";
const string MCRYPT_IDEA = "idea";
const string MCRYPT_LOKI97 = "loki97";
const string MCRYPT_MARS = "mars";
const string MCRYPT_MODE_CBC = "cbc";
const string MCRYPT_MODE_CFB = "cfb";
const string MCRYPT_MODE_ECB = "ecb";
const string MCRYPT_MODE_NOFB = "nofb";
const string MCRYPT_MODE_OFB = "ofb";
const string MCRYPT_MODE_STREAM = "stream";
const string MCRYPT_PANAMA = "panama";
const string MCRYPT_RC2 = "rc2";
const string MCRYPT_RC6 = "rc6";
const string MCRYPT_RIJNDAEL_128 = "rijndael-128";
const string MCRYPT_RIJNDAEL_192 = "rijndael-192";
const string MCRYPT_RIJNDAEL_256 = "rijndael-256";
const string MCRYPT_SAFER128 = "safer-sk128";
const string MCRYPT_SAFER64 = "safer-sk64";
const string MCRYPT_SAFERPLUS = "saferplus";
const string MCRYPT_SERPENT = "serpent";
const string MCRYPT_SKIPJACK = "skipjack";
const string MCRYPT_THREEWAY = "threeway";
const string MCRYPT_TRIPLEDES = "tripledes";
const string MCRYPT_TWOFISH = "twofish";
const string MCRYPT_WAKE = "wake";
const string MCRYPT_XTEA = "xtea";

/**
 * Encrypts/decrypts data in CBC mode
 *
 * @param string $cipher -
 * @param string $key -
 * @param string $data -
 * @param int $mode -
 * @param string $iv -
 *
 * @return string -
 */
<<__Native>>
function mcrypt_cbc(string $cipher,
                    string $key,
                    string $data,
                    mixed $mode,
                    ?string $iv = null): mixed;

/**
 * Encrypts/decrypts data in CFB mode
 *
 * @param string $cipher -
 * @param string $key -
 * @param string $data -
 * @param int $mode -
 * @param string $iv -
 *
 * @return string -
 */
<<__Native>>
function mcrypt_cfb(string $cipher,
                    string $key,
                    string $data,
                    mixed $mode,
                    ?string $iv = null): mixed;

/**
 * Creates an initialization vector (IV) from a random source
 *
 * @param int $size - The size of the IV.
 * @param int $source - The source of the IV. The source can be
 *   MCRYPT_RAND (system random number generator), MCRYPT_DEV_RANDOM (read
 *   data from /dev/random) and MCRYPT_DEV_URANDOM (read data from
 *   /dev/urandom). Prior to 5.3.0, MCRYPT_RAND was the only one supported
 *   on Windows.
 *
 * @return string - Returns the initialization vector, or FALSE on error.
 */
<<__Native>>
function mcrypt_create_iv(int $size,
                          int $source = MCRYPT_DEV_RANDOM): mixed;

/**
 * Decrypts crypttext with given parameters
 *
 * @param string $cipher -
 * @param string $key - The key with which the data was encrypted. If
 *   it's smaller than the required keysize, it is padded with '\0'.
 * @param string $data - The data that will be decrypted with the given
 *   cipher and mode. If the size of the data is not n * blocksize, the
 *   data will be padded with '\0'.
 * @param string $mode -
 * @param string $iv -
 *
 * @return string - Returns the decrypted data as a string.
 */
<<__Native>>
function mcrypt_decrypt(string $cipher,
                        string $key,
                        string $data,
                        string $mode,
                        ?string $iv = null): mixed;

/**
 * Deprecated: Encrypts/decrypts data in ECB mode
 *
 * @param string $cipher -
 * @param string $key -
 * @param string $data -
 * @param int $mode -
 * @param string $iv -
 *
 * @return string -
 */
<<__Native>>
function mcrypt_ecb(string $cipher,
                    string $key,
                    string $data,
                    mixed $mode,
                    ?string $iv = null): mixed;

/**
 * Returns the name of the opened algorithm
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return string - Returns the name of the opened algorithm as a string.
 */
<<__Native>>
function mcrypt_enc_get_algorithms_name(resource $td): mixed;

/**
 * Returns the blocksize of the opened algorithm
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return int - Returns the block size of the specified algorithm in
 *   bytes.
 */
<<__Native>>
function mcrypt_enc_get_block_size(resource $td): mixed;

/**
 * Returns the size of the IV of the opened algorithm
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return int - Returns the size of the IV, or 0 if the IV is ignored by
 *   the algorithm.
 */
<<__Native>>
function mcrypt_enc_get_iv_size(resource $td): mixed;

/**
 * Returns the maximum supported keysize of the opened mode
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return int - Returns the maximum supported key size of the algorithm
 *   in bytes.
 */
<<__Native>>
function mcrypt_enc_get_key_size(resource $td): mixed;

/**
 * Returns the name of the opened mode
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return string - Returns the name as a string.
 */
<<__Native>>
function mcrypt_enc_get_modes_name(resource $td): mixed;

/**
 * Returns an array with the supported keysizes of the opened algorithm
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return array - Returns an array with the key sizes supported by the
 *   algorithm specified by the encryption descriptor. If it returns an
 *   empty array then all key sizes between 1 and mcrypt_enc_get_key_size()
 *   are supported by the algorithm.
 */
<<__Native>>
function mcrypt_enc_get_supported_key_sizes(resource $td): mixed;

/**
 * Checks whether the encryption of the opened mode works on blocks
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return bool - Returns TRUE if the mode is for use with block
 *   algorithms, otherwise it returns FALSE.
 */
<<__Native>>
function mcrypt_enc_is_block_algorithm_mode(resource $td): bool;

/**
 * Checks whether the algorithm of the opened mode is a block algorithm
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return bool - Returns TRUE if the algorithm is a block algorithm or
 *   FALSE if it is a stream one.
 */
<<__Native>>
function mcrypt_enc_is_block_algorithm(resource $td): bool;

/**
 * Checks whether the opened mode outputs blocks
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return bool - Returns TRUE if the mode outputs blocks of bytes, or
 *   FALSE if it outputs just bytes.
 */
<<__Native>>
function mcrypt_enc_is_block_mode(resource $td): bool;

/**
 * Runs a self test on the opened module
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return int - If the self test succeeds it returns FALSE. In case of
 *   an error, it returns TRUE.
 */
<<__Native>>
function mcrypt_enc_self_test(resource $td): mixed;

/**
 * Encrypts plaintext with given parameters
 *
 * @param string $cipher -
 * @param string $key - The key with which the data will be encrypted. If
 *   it's smaller than the required keysize, it is padded with '\0'. It is
 *   better not to use ASCII strings for keys.   It is recommended to use
 *   the mhash functions to create a key from a string.
 * @param string $data - The data that will be encrypted with the given
 *   cipher and mode. If the size of the data is not n * blocksize, the
 *   data will be padded with '\0'.   The returned crypttext can be larger
 *   than the size of the data that was given by data.
 * @param string $mode -
 * @param string $iv -
 *
 * @return string - Returns the encrypted data, as a string.
 */
<<__Native>>
function mcrypt_encrypt(string $cipher,
                        string $key,
                        string $data,
                        string $mode,
                        ?string $iv = null): mixed;

/**
 * This function deinitializes an encryption module
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return bool -
 */
<<__Native>>
function mcrypt_generic_deinit(resource $td): bool;

/**
 * This function terminates encryption
 *
 * @param resource $td -
 *
 * @return bool -
 */
<<__Native>>
function mcrypt_generic_end(resource $td): bool;

/**
 * This function initializes all buffers needed for encryption
 *
 * @param resource $td - The encryption descriptor.
 * @param string $key - The maximum length of the key should be the one
 *   obtained by calling mcrypt_enc_get_key_size() and every value smaller
 *   than this is legal.
 * @param string $iv - The IV should normally have the size of the
 *   algorithms block size, but you must obtain the size by calling
 *   mcrypt_enc_get_iv_size(). IV is ignored in ECB. IV MUST exist in CFB,
 *   CBC, STREAM, nOFB and OFB modes. It needs to be random and unique (but
 *   not secret). The same IV must be used for encryption/decryption. If
 *   you do not want to use it you should set it to zeros, but this is not
 *   recommended.
 *
 * @return int - The function returns a negative value on error: -3 when
 *   the key length was incorrect, -4 when there was a memory allocation
 *   problem and any other return value is an unknown error. If an error
 *   occurs a warning will be displayed accordingly. FALSE is returned if
 *   incorrect parameters were passed.
 */
<<__Native>>
function mcrypt_generic_init(resource $td,
                             string $key,
                             string $iv): mixed;

/**
 * This function encrypts data
 *
 * @param resource $td - The encryption descriptor.   The encryption
 *   handle should always be initialized with mcrypt_generic_init() with a
 *   key and an IV before calling this function. Where the encryption is
 *   done, you should free the encryption buffers by calling
 *   mcrypt_generic_deinit(). See mcrypt_module_open() for an example.
 * @param string $data - The data to encrypt.
 *
 * @return string - Returns the encrypted data.
 */
<<__Native>>
function mcrypt_generic(resource $td,
                        string $data): mixed;

/**
 * Gets the block size of the specified cipher
 *
 * @param string $cipher -
 * @param string $mode -
 *
 * @return int - Gets the block size, as an integer.
 */
<<__Native>>
function mcrypt_get_block_size(string $cipher,
                               string $mode): mixed;

/**
 * Gets the name of the specified cipher
 *
 * @param string $cipher -
 *
 * @return string - This function returns the name of the cipher or FALSE
 *   if the cipher does not exist.
 */
<<__Native>>
function mcrypt_get_cipher_name(string $cipher): mixed;

/**
 * Returns the size of the IV belonging to a specific cipher/mode combination
 *
 * @param string $cipher -
 * @param string $mode - The IV is ignored in ECB mode as this mode does
 *   not require it. You will need to have the same IV (think: starting
 *   point) both at encryption and decryption stages, otherwise your
 *   encryption will fail.
 *
 * @return int - Returns the size of the Initialization Vector (IV) in
 *   bytes. On error the function returns FALSE. If the IV is ignored in
 *   the specified cipher/mode combination zero is returned.
 */
<<__Native>>
function mcrypt_get_iv_size(string $cipher,
                            string $mode): mixed;

/**
 * Gets the key size of the specified cipher
 *
 * @param string $cipher -
 * @param string $mode -
 *
 * @return int - Returns the maximum supported key size of the algorithm
 *   in bytes .
 */
<<__Native>>
function mcrypt_get_key_size(string $cipher,
                             string $mode): mixed;

/**
 * Gets an array of all supported ciphers
 *
 * @param string $lib_dir - Specifies the directory where all algorithms
 *   are located. If not specified, the value of the mcrypt.algorithms_dir
 *   directive is used.
 *
 * @return array - Returns an array with all the supported algorithms.
 */
<<__Native>>
function mcrypt_list_algorithms(string $lib_dir = ''): varray;

/**
 * Gets an array of all supported modes
 *
 * @param string $lib_dir - Specifies the directory where all modes are
 *   located. If not specified, the value of the mcrypt.modes_dir directive
 *   is used.
 *
 * @return array - Returns an array with all the supported modes.
 */
<<__Native>>
function mcrypt_list_modes(string $lib_dir = ''): varray;

/**
 * Closes the mcrypt module
 *
 * @param resource $td - The encryption descriptor.
 *
 * @return bool -
 */
<<__Native>>
function mcrypt_module_close(resource $td): bool;

/**
 * Returns the blocksize of the specified algorithm
 *
 * @param string $algorithm - The algorithm name.
 * @param string $lib_dir - This optional parameter can contain the
 *   location where the mode module is on the system.
 *
 * @return int - Returns the block size of the algorithm specified in
 *   bytes.
 */
<<__Native>>
function mcrypt_module_get_algo_block_size(string $algorithm,
                                           string $lib_dir = ''): int;

/**
 * Returns the maximum supported keysize of the opened mode
 *
 * @param string $algorithm - The algorithm name.
 * @param string $lib_dir - This optional parameter can contain the
 *   location where the mode module is on the system.
 *
 * @return int - This function returns the maximum supported key size of
 *   the algorithm specified in bytes.
 */
<<__Native>>
function mcrypt_module_get_algo_key_size(string $algorithm,
                                         string $lib_dir = ''): int;

/**
 * Returns an array with the supported keysizes of the opened algorithm
 *
 * @param string $algorithm - The algorithm to be used.
 * @param string $lib_dir - The optional lib_dir parameter can contain
 *   the location where the algorithm module is on the system.
 *
 * @return array - Returns an array with the key sizes supported by the
 *   specified algorithm. If it returns an empty array then all key sizes
 *   between 1 and mcrypt_module_get_algo_key_size() are supported by the
 *   algorithm.
 */
<<__Native>>
function mcrypt_module_get_supported_key_sizes(string $algorithm,
                                               string $lib_dir = ''): varray;

/**
 * Returns if the specified module is a block algorithm or not
 *
 * @param string $mode - The mode to check.
 * @param string $lib_dir - The optional lib_dir parameter can contain
 *   the location where the algorithm module is on the system.
 *
 * @return bool - This function returns TRUE if the mode is for use with
 *   block algorithms, otherwise it returns FALSE. (e.g. FALSE for stream,
 *   and TRUE for cbc, cfb, ofb).
 */
<<__Native>>
function mcrypt_module_is_block_algorithm_mode(string $mode,
                                               string $lib_dir= ''): bool;

/**
 * This function checks whether the specified algorithm is a block algorithm
 *
 * @param string $algorithm - The algorithm to check.
 * @param string $lib_dir - The optional lib_dir parameter can contain
 *   the location where the algorithm module is on the system.
 *
 * @return bool - This function returns TRUE if the specified algorithm
 *   is a block algorithm, or FALSE if it is a stream one.
 */
<<__Native>>
function mcrypt_module_is_block_algorithm(string $algorithm,
                                          string $lib_dir= ''): bool;

/**
 * Returns if the specified mode outputs blocks or not
 *
 * @param string $mode -
 * @param string $lib_dir - The optional lib_dir parameter can contain
 *   the location where the algorithm module is on the system.
 *
 * @return bool - This function returns TRUE if the mode outputs blocks
 *   of bytes or FALSE if it outputs just bytes. (e.g. TRUE for cbc and
 *   ecb, and FALSE for cfb and stream).
 */
<<__Native>>
function mcrypt_module_is_block_mode(string $mode,
                                     string $lib_dir= ''): bool;

/**
 * Opens the module of the algorithm and the mode to be used
 *
 * @param string $algorithm -
 * @param string $algorithm_directory - The algorithm_directory parameter
 *   is used to locate the encryption module. When you supply a directory
 *   name, it is used. When you set it to an empty string (""), the value
 *   set by the mcrypt.algorithms_dir directive is used. When it is not
 *   set, the default directory that is used is the one that was compiled
 *   into libmcrypt (usually /usr/local/lib/libmcrypt).
 * @param string $mode -
 * @param string $mode_directory - The mode_directory parameter is used
 *   to locate the encryption module. When you supply a directory name, it
 *   is used. When you set it to an empty string (""), the value set by the
 *   mcrypt.modes_dir directive is used. When it is not set, the default
 *   directory that is used is the one that was compiled-in into libmcrypt
 *   (usually /usr/local/lib/libmcrypt).
 *
 * @return resource - Normally it returns an encryption descriptor, or
 *   FALSE on error.
 */
<<__Native>>
function mcrypt_module_open(string $algorithm,
                            string $algorithm_directory,
                            string $mode,
                            string $mode_directory): mixed;

/**
 * This function runs a self test on the specified module
 *
 * @param string $algorithm -
 * @param string $lib_dir - The optional lib_dir parameter can contain
 *   the location where the algorithm module is on the system.
 *
 * @return bool - The function returns TRUE if the self test succeeds, or
 *   FALSE when it fails.
 */
<<__Native>>
function mcrypt_module_self_test(string $algorithm,
                                 string $lib_dir = ''): bool;

/**
 * Encrypts/decrypts data in OFB mode
 *
 * @param string $cipher -
 * @param string $key -
 * @param string $data -
 * @param int $mode -
 * @param string $iv -
 *
 * @return string -
 */
<<__Native>>
function mcrypt_ofb(string $cipher,
                    string $key,
                    string $data,
                    mixed $mode,
                    ?string $iv = null): mixed;

/**
 * Decrypts data
 *
 * @param resource $td - An encryption descriptor returned by
 *   mcrypt_module_open()
 * @param string $data - Encrypted data.
 *
 * @return string -
 */
<<__Native>>
function mdecrypt_generic(resource $td,
                          string $data): mixed;
