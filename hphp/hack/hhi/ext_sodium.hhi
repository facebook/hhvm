<?hh // decl

final class SodiumException extends Exception { }

///// utilities

function sodium_bin2hex(string $binary): string;

function sodium_hex2bin(string $hex, ?string $ignore = null): string;

function sodium_memzero(mixed &$buffer): void;

function sodium_increment(mixed &$buffer): void;

function sodium_add(mixed &$value, string $add): void;

function sodium_memcmp(string $a, string $b): int;

function sodium_compare(string $a, string $b): int;

function sodium_crypto_scalarmult(string $n, string $p): string;

///// Hashing (not for passwords or keys)

function sodium_crypto_generichash_keygen(): string;

function sodium_crypto_generichash(
  string $msg,
  ?string $key = null,
  ?int $length = null,
): string;

function sodium_crypto_generichash_init(
  ?string $key = null,
  ?int $length = null
): string;

function sodium_crypto_generichash_update(
  mixed &$state,
  string $msg,
): bool;

function sodium_crypto_generichash_final(
  mixed &$state,
  ?int $length = null,
): string;

function sodium_crypto_shorthash_keygen(): string;

function sodium_crypto_shorthash(string $message, string $key): string;

///// Key derivation (kdf)

function sodium_crypto_pwhash(
  int $output_length,
  string $password,
  string $salt,
  int $opslimit,
  int $memlimit,
): string;

function sodium_crypto_pwhash_scryptsalsa208sha256(
  int $output_length,
  string $password,
  string $salt,
  int $opslimit,
  int $memlimit,
): string;

function sodium_crypto_kdf_derive_from_key(
  int $subkey_length,
  int $subkey_id,
  string $context,
  string $key,
): string;

///// Password hashing for storage

function sodium_crypto_pwhash_str(
  string $password,
  int $opslimit,
  int $memlimit,
): string;

function sodium_crypto_pwhash_str_verify(
  string $hash,
  string $password,
): bool;

function sodium_crypto_pwhash_scryptsalsa208sha256_str(
  string $password,
  int $opslimit,
  int $memlimit,
): string;

function sodium_crypto_pwhash_scryptsalsa208sha256_str_verify(
  string $hash,
  string $password,
): bool;

///// MACs

function sodium_crypto_auth_keygen(): string;

function sodium_crypto_auth(string $message, string $key): string;

function sodium_crypto_auth_verify(
  string $signature,
  string $message,
  string $key,
): bool;

///// Symmetric (PSK) encryption

function sodium_crypto_secretbox_keygen(): string;

function sodium_crypto_secretbox(
  string $plaintext,
  string $nonce,
  string $key,
): string;

function sodium_crypto_secretbox_open(
  string $ciphertext,
  string $nonce,
  string $key,
): mixed /* string | false */;

///// Asymetric (public-key) encryption key management

function sodium_crypto_box_keypair(): string;

function sodium_crypto_box_seed_keypair(string $seed): string;

function sodium_crypto_box_keypair_from_secretkey_and_publickey(
  string $secretkey,
  string $publickey,
): string;

function sodium_crypto_box_publickey(string $keypair): string;

function sodium_crypto_box_secretkey(string $keypair): string;

function sodium_crypto_box_publickey_from_secretkey(string $key): string;

function sodium_crypto_scalarmult_base(string $key): string;

function sodium_crypto_kx_keypair(): string;
function sodium_crypto_kx_seed_keypair(string $seed): string;
function sodium_crypto_kx_secretkey(string $keypair): string;
function sodium_crypto_kx_publickey(string $keypair): string;
function sodium_crypto_kx_client_session_keys(
  string $client_keypair,
  string $server_pubkey,
): (string, string);
function sodium_crypto_kx_server_session_keys(
  string $server_keypair,
  string $client_pubkey,
): (string, string);

///// Unauthenticated asymetric (PSK) encryption (you probably don't want this)

function sodium_crypto_stream(
  int $length,
  string $nonce,
  string $key,
): string;

function sodium_crypto_stream_xor(
  string $message,
  string $nonce,
  string $key,
): string;

///// Asymetric (public-key) encryption

function sodium_crypto_box(
  string $plaintext,
  string $nonce,
  string $keypair,
): string;

function sodium_crypto_box_open(
  string $ciphertext,
  string $nonce,
  string $key,
): mixed;

function sodium_crypto_box_seal(
  string $plaintext,
  string $publickey,
): string;

function sodium_crypto_box_seal_open(
  string $ciphertext,
  string $keypair,
): mixed;

///// Asymetric (public key) signature key management

function sodium_crypto_sign_keypair(): string;

function sodium_crypto_sign_seed_keypair(string $seed): string;

function sodium_crypto_sign_publickey_from_secretkey(string $secretkey): string;

function sodium_crypto_sign_keypair_from_secretkey_and_publickey(
  string $secretkey,
  string $publickey,
): string;

function sodium_crypto_sign_publickey(string $keypair): string;

function sodium_crypto_sign_secretkey(string $keypair): string;

function sodium_crypto_sign_ed25519_pk_to_curve25519(string $eddsakey): string;
function sodium_crypto_sign_ed25519_sk_to_curve25519(string $eddsakey): string;

///// Asymetric (public key) signatures

function sodium_crypto_sign(
  string $message,
  string $secretkey,
): string;

function sodium_crypto_sign_open(
  string $signed,
  string $publickey,
): mixed;

function sodium_crypto_sign_detached(
  string $message,
  string $secretkey,
): string;

function sodium_crypto_sign_verify_detached(
  string $signature,
  string $message,
  string $publickey,
): bool;

///// AEAD functions

function sodium_crypto_aead_aes256gcm_is_available(): bool;

function sodium_crypto_aead_aes256gcm_decrypt(
  string $ciphertext,
  string $ad,
  string $pnonce,
  string $key,
): mixed;

function sodium_crypto_aead_aes256gcm_encrypt(
  string $plaintext,
  string $ad,
  string $pnonce,
  string $key,
): string;

function sodium_crypto_aead_chacha20poly1305_decrypt(
  string $ciphertext,
  string $ad,
  string $pnonce,
  string $key,
): mixed;

function sodium_crypto_aead_chacha20poly1305_encrypt(
  string $plaintext,
  string $ad,
  string $pnonce,
  string $key,
): string;

function sodium_crypto_aead_chacha20poly1305_ietf_decrypt(
  string $ciphertext,
  string $ad,
  string $pnonce,
  string $key,
): mixed;

function sodium_crypto_aead_chacha20poly1305_ietf_encrypt(
  string $plaintext,
  string $ad,
  string $pnonce,
  string $key,
): string;

function sodium_crypto_aead_xchacha20poly1305_ietf_decrypt(
  string $ciphertext,
  string $ad,
  string $pnonce,
  string $key,
): mixed;

function sodium_crypto_aead_xchacha20poly1305_ietf_encrypt(
  string $plaintext,
  string $ad,
  string $pnonce,
  string $key,
): string;

///// Always-defined constants

const string SODIUM_LIBRARY_VERSION = '';
const int SODIUM_LIBRARY_MAJOR_VERSION = 0;
const int SODIUM_LIBRARY_MINOR_VERSION = 0;

const int SODIUM_CRYPTO_SCALARMULT_BYTES = 0;
const int SODIUM_CRYPTO_SCALARMULT_SCALARBYTES = 0;

const int SODIUM_CRYPTO_GENERICHASH_KEYBYTES = 0;
const int SODIUM_CRYPTO_GENERICHASH_KEYBYTES_MIN = 0;
const int SODIUM_CRYPTO_GENERICHASH_KEYBYTES_MAX = 0;

const int SODIUM_CRYPTO_SHORTHASH_BYTES = 0;
const int SODIUM_CRYPTO_SHORTHASH_KEYBYTES = 0;

const string SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_STRPREFIX = '';
const int SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_SALTBYTES = 0;
const int SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_OPSLIMIT_INTERACTIVE
  = 0;
const int SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_MEMLIMIT_INTERACTIVE
  = 0;
const int SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_OPSLIMIT_SENSITIVE
  = 0;
const int SODIUM_CRYPTO_PWHASH_SCRYPTSALSA208SHA256_MEMLIMIT_SENSITIVE
  = 0;

const int SODIUM_CRYPTO_AUTH_BYTES = 0;
const int SODIUM_CRYPTO_AUTH_KEYBYTES = 0;

const int SODIUM_CRYPTO_SECRETBOX_KEYBYTES = 0;
const int SODIUM_CRYPTO_SECRETBOX_MACBYTES = 0;
const int SODIUM_CRYPTO_SECRETBOX_NONCEBYTES = 0;

const int SODIUM_CRYPTO_BOX_SECRETKEYBYTES = 0;
const int SODIUM_CRYPTO_BOX_PUBLICKEYBYTES = 0;
const int SODIUM_CRYPTO_BOX_KEYPAIRBYTES = 0;
const int SODIUM_CRYPTO_BOX_MACBYTES = 0;
const int SODIUM_CRYPTO_BOX_NONCEBYTES = 0;
const int SODIUM_CRYPTO_BOX_SEEDBYTES = 0;

const int SODIUM_CRYPTO_SIGN_BYTES = 0;
const int SODIUM_CRYPTO_SIGN_SEEDBYTES = 0;
const int SODIUM_CRYPTO_SIGN_PUBLICKEYBYTES = 0;
const int SODIUM_CRYPTO_SIGN_SECRETKEYBYTES = 0;
const int SODIUM_CRYPTO_SIGN_KEYPAIRBYTES = 0;

const int SODIUM_CRYPTO_STREAM_NONCEBYTES = 0;
const int SODIUM_CRYPTO_STREAM_KEYBYTES = 0;

const int SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_KEYBYTES = 0;
const int SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_NSECBYTES = 0;
const int SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_NPUBBYTES = 0;
const int SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_ABYTES = 0;

///// Conditionally-defined constants (depends on libsodium build options)

const int SODIUM_CRYPTO_PWHASH_SALTBYTES = 0;
const string SODIUM_CRYPTO_PWHASH_STRPREFIX = '';
const int SODIUM_CRYPTO_PWHASH_OPSLIMIT_INTERACTIVE = 0;
const int SODIUM_CRYPTO_PWHASH_MEMLIMIT_INTERACTIVE = 0;
const int SODIUM_CRYPTO_PWHASH_OPSLIMIT_MODERATE = 0;
const int SODIUM_CRYPTO_PWHASH_MEMLIMIT_MODERATE = 0;
const int SODIUM_CRYPTO_PWHASH_OPSLIMIT_SENSITIVE = 0;
const int SODIUM_CRYPTO_PWHASH_MEMLIMIT_SENSITIVE = 0;

const int SODIUM_CRYPTO_BOX_SEALBYTES = 0;

const int SODIUM_CRYPTO_AEAD_AES245GCM_KEYBYTES = 0;
const int SODIUM_CRYPTO_AEAD_AES245GCM_NSECBYTES = 0;
const int SODIUM_CRYPTO_AEAD_AES245GCM_NPUBBYTES = 0;
const int SODIUM_CRYPTO_AEAD_AES245GCM_ABYTES = 0;

const int SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_IETF_KEYBYTES = 0;
const int SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_IETF_NSECBYTES = 0;
const int SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_IETF_NPUBBYTES = 0;
const int SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_IETF_ABYTES = 0;

const int SODIUM_CRYPTO_AEAD_XCHACHA20POLY1305_IETF_KEYBYTES = 0;
const int SODIUM_CRYPTO_AEAD_XCHACHA20POLY1305_IETF_NSECBYTES = 0;
const int SODIUM_CRYPTO_AEAD_XCHACHA20POLY1305_IETF_NPUBBYTES = 0;
const int SODIUM_CRYPTO_AEAD_XCHACHA20POLY1305_IETF_ABYTES = 0;

const int SODIUM_CRYPTO_KDF_BYTES_MIN = 0;
const int SODIUM_CRYPTO_KDF_BYTES_MAX = 0;
const int SODIUM_CRYPTO_KDF_BYTES_CONTEXTBYTES = 0;
const int SODIUM_CRYPTO_KDF_BYTES_KEYBYTES  = 0;

const int SODIUM_CRYPTO_KX_PUBLICKEYBYTES = 0;
const int SODIUM_CRYPTO_KX_SESSIONKEYBYTES = 0;
const int SODIUM_CRYPTO_KX_SECRETKEYBYTES = 0;
const int SODIUM_CRYPTO_KX_KEYPAIRBYTES = 0;
