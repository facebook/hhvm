<?hh

final class SodiumException extends Exception {
}

///// utilities

<<__Native>>
function sodium_bin2hex(string $binary): string;

<<__Native>>
function sodium_hex2bin(string $hex, ?string $ignore = null): string;

<<__Native>>
function sodium_memzero(inout mixed $buffer): void;

<<__Native>>
function sodium_increment(inout mixed $buffer): void;

<<__Native>>
function sodium_add(inout mixed $value, string $add): void;

<<__Native>>
function sodium_memcmp(string $a, string $b): int;

<<__Native>>
function sodium_compare(string $a, string $b): int;

<<__Native>>
function sodium_crypto_scalarmult(string $n, string $p): string;

///// Hashing (not for passwords or keys)

function sodium_crypto_generichash_keygen(): string {
  return random_bytes(
    SODIUM_CRYPTO_GENERICHASH_KEYBYTES
  );
}

<<__Native>>
function sodium_crypto_generichash(
  string $msg,
  ?string $key = null,
  ?int $length = null,
): string;

<<__Native>>
function sodium_crypto_generichash_init(
  ?string $key = null,
  ?int $length = null
): string;

<<__Native>>
function sodium_crypto_generichash_update(
  inout mixed $state,
  string $msg,
): bool;

<<__Native>>
function sodium_crypto_generichash_final(
  inout mixed $state,
  ?int $length = null,
): string;

function sodium_crypto_shorthash_keygen(): string {
  return random_bytes(SODIUM_CRYPTO_SHORTHASH_KEYBYTES);
}

<<__Native>>
function sodium_crypto_shorthash(string $message, string $key): string;

///// Key derivation

<<__Native>>
function sodium_crypto_pwhash(
  int $output_length,
  string $password,
  string $salt,
  int $opslimit,
  int $memlimit,
): string;

<<__Native>>
function sodium_crypto_pwhash_scryptsalsa208sha256(
  int $output_length,
  string $password,
  string $salt,
  int $opslimit,
  int $memlimit,
): string;

///// Password hashing for storage

<<__Native>>
function sodium_crypto_pwhash_str(
  string $password,
  int $opslimit,
  int $memlimit,
): string;

<<__Native>>
function sodium_crypto_pwhash_str_verify(
  string $hash,
  string $password,
): bool;

<<__Native>>
function sodium_crypto_pwhash_scryptsalsa208sha256_str(
  string $password,
  int $opslimit,
  int $memlimit,
): string;

<<__Native>>
function sodium_crypto_pwhash_scryptsalsa208sha256_str_verify(
  string $hash,
  string $password,
): bool;

///// MACs

function sodium_crypto_auth_keygen(): string {
  return random_bytes(SODIUM_CRYPTO_AUTH_KEYBYTES);
}

<<__Native>>
function sodium_crypto_auth(string $message, string $key): string;

<<__Native>>
function sodium_crypto_auth_verify(
  string $signature,
  string $message,
  string $key,
): bool;

///// Symmetric (PSK) encryption

function sodium_crypto_secretbox_keygen(): string {
  return random_bytes(SODIUM_CRYPTO_SECRETBOX_KEYBYTES);
}

<<__Native>>
function sodium_crypto_secretbox(
  string $plaintext,
  string $nonce,
  string $key,
): string;

<<__Native>>
function sodium_crypto_secretbox_open(
  string $ciphertext,
  string $nonce,
  string $key,
): mixed /* string | false */;

///// Asymetric (public-key) encryption key management

<<__Native>>
function sodium_crypto_box_keypair(): string;

<<__Native>>
function sodium_crypto_box_seed_keypair(string $seed): string;

function sodium_crypto_box_keypair_from_secretkey_and_publickey(
  string $secretkey,
  string $publickey,
): string {
  if (strlen($secretkey) !== SODIUM_CRYPTO_BOX_SECRETKEYBYTES) {
    throw new SodiumException(
      'secretkey should be'.
      'CRYPTO_BOX_SECRETKEYBYTES long'
    );
  }

  if (strlen($publickey) !== SODIUM_CRYPTO_BOX_PUBLICKEYBYTES) {
    throw new SodiumException(
      'publickey should be'.
      'CRYPTO_BOX_PUBLICKEYBYTES long'
    );
  }

  return $secretkey.$publickey;
}

function sodium_crypto_box_publickey(string $keypair): string {
  if (strlen($keypair) !== SODIUM_CRYPTO_BOX_KEYPAIRBYTES) {
    throw new SodiumException(
      'keypair should be CRYPTO_BOX_KEYPAIRBYTES long'
    );
  }
  return substr(
    $keypair,
    SODIUM_CRYPTO_BOX_SECRETKEYBYTES,
    SODIUM_CRYPTO_BOX_PUBLICKEYBYTES,
  );
}

function sodium_crypto_box_secretkey(string $keypair): string {
  if (strlen($keypair) !== SODIUM_CRYPTO_BOX_KEYPAIRBYTES) {
    throw new SodiumException(
      'keypair should be CRYPTO_BOX_KEYPAIRBYTES long'
    );
  }
  return substr(
    $keypair,
    0,
    SODIUM_CRYPTO_BOX_SECRETKEYBYTES,
  );
}

<<__Native>>
function sodium_crypto_box_publickey_from_secretkey(string $key): string;

<<__Native>>
function sodium_crypto_scalarmult_base(string $key): string;

<<__Native>>
function sodium_crypto_kx_keypair(): string;

<<__Native>>
function sodium_crypto_kx_seed_keypair(string $seed): string;

function sodium_crypto_kx_secretkey(string $keypair): string {
  if (strlen($keypair) !== SODIUM_CRYPTO_KX_KEYPAIRBYTES) {
    throw new SodiumException(
      "keypair should be CRYPTO_KX_KEYPAIRBYTES bytes"
    );
  }
  return substr($keypair, 0, SODIUM_CRYPTO_KX_SECRETKEYBYTES);
}

function sodium_crypto_kx_publickey(string $keypair): string {
  if (strlen($keypair) !== SODIUM_CRYPTO_KX_KEYPAIRBYTES) {
    throw new SodiumException(
      "keypair should be CRYPTO_KX_KEYPAIRBYTES bytes"
    );
  }
  return substr(
    $keypair,
    SODIUM_CRYPTO_KX_SECRETKEYBYTES,
    SODIUM_CRYPTO_KX_PUBLICKEYBYTES,
  );
}

<<__Native>>
function sodium_crypto_kx_client_session_keys(
  string $client_keypair,
  string $server_pubkey,
): varray<string>;

<<__Native>>
function sodium_crypto_kx_server_session_keys(
  string $server_keypair,
  string $client_pubkey,
): varray<string>;

///// Unauthenticated asymetric (PSK) encryption (you probably don't want this)

function sodium_crypto_stream_keygen(): string {
  return random_bytes(SODIUM_CRYPTO_STREAM_KEYBYTES);
}

<<__Native>>
function sodium_crypto_stream(
  int $length,
  string $nonce,
  string $key,
): string;

<<__Native>>
function sodium_crypto_stream_xor(
  string $message,
  string $nonce,
  string $key,
): string;

///// Asymetric (public-key) encryption

<<__Native>>
function sodium_crypto_box(
  string $plaintext,
  string $nonce,
  string $keypair,
): string;

<<__Native>>
function sodium_crypto_box_open(
  string $ciphertext,
  string $nonce,
  string $key,
): mixed;

<<__Native>>
function sodium_crypto_box_seal(
  string $plaintext,
  string $publickey,
): string;

<<__Native>>
function sodium_crypto_box_seal_open(
  string $ciphertext,
  string $keypair,
): mixed;

///// Asymetric (public key) signature key management

<<__Native>>
function sodium_crypto_sign_keypair(): string;

<<__Native>>
function sodium_crypto_sign_seed_keypair(string $seed): string;

<<__Native>>
function sodium_crypto_sign_publickey_from_secretkey(string $secretkey): string;

function sodium_crypto_sign_keypair_from_secretkey_and_publickey(
  string $secretkey,
  string $publickey,
): string {
  if (strlen($secretkey) !== SODIUM_CRYPTO_SIGN_SECRETKEYBYTES) {
    throw new SodiumException(
      'secretkey should '.
      'be CRYPTO_BOX_SECRETKEYBYTES long',
    );
  }

  if (strlen($publickey) !== SODIUM_CRYPTO_SIGN_PUBLICKEYBYTES) {
    throw new SodiumException(
      'publickey should '.
      'be CRYPTO_BOX_PUBLICKEYBYTES long',
    );
  }

  return $secretkey.$publickey;
}

function sodium_crypto_sign_publickey(string $keypair): string {
  if (strlen($keypair) !== SODIUM_CRYPTO_SIGN_KEYPAIRBYTES) {
    throw new SodiumException(
      'keypair should be CRYPTO_SIGN_KEYPAIRBYTES '.
      'long'
    );
  }
  return substr(
    $keypair,
    SODIUM_CRYPTO_SIGN_SECRETKEYBYTES,
    SODIUM_CRYPTO_SIGN_PUBLICKEYBYTES,
  );
}

function sodium_crypto_sign_secretkey(string $keypair): string {
  if (strlen($keypair) !== SODIUM_CRYPTO_SIGN_KEYPAIRBYTES) {
    throw new SodiumException(
      'keypair should be CRYPTO_SIGN_KEYPAIRBYTES long'
    );
  }
  return substr($keypair, 0, SODIUM_CRYPTO_SIGN_SECRETKEYBYTES);
}

<<__Native>>
function sodium_crypto_sign_ed25519_pk_to_curve25519(string $eddsakey): string;
<<__Native>>
function sodium_crypto_sign_ed25519_sk_to_curve25519(string $eddsakey): string;

///// Asymetric (public key) signatures

<<__Native>>
function sodium_crypto_sign(
  string $message,
  string $secretkey,
): string;

<<__Native>>
function sodium_crypto_sign_open(
  string $signed,
  string $publickey,
): mixed;

<<__Native>>
function sodium_crypto_sign_detached(
  string $message,
  string $secretkey,
): string;

<<__Native>>
function sodium_crypto_sign_verify_detached(
  string $signature,
  string $message,
  string $publickey,
): bool;

///// AEAD functions

function sodium_crypto_aead_aes256gcm_keygen(): string {
  return random_bytes(SODIUM_CRYPTO_AEAD_AES256GCM_KEYBYTES);
}

<<__Native>>
function sodium_crypto_aead_aes256gcm_is_available(): bool;

<<__Native>>
function sodium_crypto_aead_aes256gcm_decrypt(
  string $ciphertext,
  string $ad,
  string $pnonce,
  string $key,
): mixed;

<<__Native>>
function sodium_crypto_aead_aes256gcm_encrypt(
  string $plaintext,
  string $ad,
  string $pnonce,
  string $key,
): string;

function sodium_crypto_aead_chacha20poly1305_keygen(): string {
  return random_bytes(SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_KEYBYTES);
}

<<__Native>>
function sodium_crypto_aead_chacha20poly1305_decrypt(
  string $ciphertext,
  string $ad,
  string $pnonce,
  string $key,
): mixed;

<<__Native>>
function sodium_crypto_aead_chacha20poly1305_encrypt(
  string $plaintext,
  string $ad,
  string $pnonce,
  string $key,
): string;

function sodium_crypto_aead_chacha20poly1305_ietf_keygen(): string {
  return random_bytes(
    SODIUM_CRYPTO_AEAD_CHACHA20POLY1305_IETF_KEYBYTES
  );
}

<<__Native>>
function sodium_crypto_aead_chacha20poly1305_ietf_decrypt(
  string $ciphertext,
  string $ad,
  string $pnonce,
  string $key,
): mixed;

<<__Native>>
function sodium_crypto_aead_chacha20poly1305_ietf_encrypt(
  string $plaintext,
  string $ad,
  string $pnonce,
  string $key,
): string;

function sodium_crypto_aead_xchacha20poly1305_ietf_keygen(): string {
  return random_bytes(
    SODIUM_CRYPTO_AEAD_XCHACHA20POLY1305_IETF_KEYBYTES,
  );
}

<<__Native>>
function sodium_crypto_aead_xchacha20poly1305_ietf_decrypt(
  string $ciphertext,
  string $ad,
  string $pnonce,
  string $key,
): mixed;

<<__Native>>
function sodium_crypto_aead_xchacha20poly1305_ietf_encrypt(
  string $plaintext,
  string $ad,
  string $pnonce,
  string $key,
): string;

///// key deriviation functions (kdf)

function sodium_crypto_kdf_keygen(): string {
  return random_bytes(SODIUM_CRYPTO_KDF_KEYBYTES);
}

<<__Native>>
function sodium_crypto_kdf_derive_from_key(
  int $subkey_length,
  int $subkey_id,
  string $context,
  string $key,
): string;

<<__Native>>
function sodium_crypto_core_hchacha20(
  string $in,
  string $k,
  ?string $c = null,
): string;

///// Ristretto

<<__Native>>
function sodium_crypto_core_ristretto255_add(string $p, string $q): string;

<<__Native>>
function sodium_crypto_core_ristretto255_sub(string $p, string $q): string;

<<__Native>>
function sodium_crypto_core_ristretto255_is_valid_point(string $s): bool;

<<__Native>>
function sodium_crypto_core_ristretto255_random(): string;

<<__Native>>
function sodium_crypto_core_ristretto255_from_hash(string $r): string;

<<__Native>>
function sodium_crypto_scalarmult_ristretto255(string $n, string $p): string;

<<__Native>>
function sodium_crypto_core_ristretto255_scalar_reduce(string $s): string;

// The range of outputs of random_bytes(SODIUM_CRYPTO_CORE_RISTRETTO255_SCALARBYTES)
// is larger than that of crypto_core_ristretto255_scalar_random().
function sodium_crypto_core_ristretto255_scalar_random(): string {
  return sodium_crypto_core_ristretto255_scalar_reduce(random_bytes(
      SODIUM_CRYPTO_CORE_RISTRETTO255_NONREDUCEDSCALARBYTES));
}

<<__Native>>
function sodium_crypto_core_ristretto255_scalar_invert(string $s): string;

<<__Native>>
function sodium_crypto_core_ristretto255_scalar_negate(string $s): string;

<<__Native>>
function sodium_crypto_core_ristretto255_scalar_complement(string $s): string;

<<__Native>>
function sodium_crypto_core_ristretto255_scalar_add(string $x, string $y): string;

<<__Native>>
function sodium_crypto_core_ristretto255_scalar_sub(string $x, string $y): string;

<<__Native>>
function sodium_crypto_core_ristretto255_scalar_mul(string $x, string $y): string;

///// AEAD secretstream

// Generate random key bits for xchacha20 poly1305 based secretstream.
// This function is not required. Any key of size
// crypto_secretstream_xchacha20poly1305_KEYBYTES can be used in secretstream APIs.
<<__Native>>
function sodium_crypto_secretstream_xchacha20poly1305_keygen(): string;

// Initialize the state for encryption/push side secretstream.
// Return the state string and the header string.
<<__Native>>
function sodium_crypto_secretstream_xchacha20poly1305_init_push(string $key): varray<string>;

// Encryption. Return the ciphertext.
<<__Native>>
function sodium_crypto_secretstream_xchacha20poly1305_push(inout mixed $state, string $plaintext, string $ad, int $tag): string;

// Initialize the state for decryption/pull side secretstream.
// Return the state string.
<<__Native>>
function sodium_crypto_secretstream_xchacha20poly1305_init_pull(string $key, string $header): string;

// Decryption. Return the plaintext and the tag.
<<__Native>>
function sodium_crypto_secretstream_xchacha20poly1305_pull(inout mixed $state, string $ciphertext, string $ad): varray;

// Renew the symmetric key used in the secretstream.
<<__Native>>
function sodium_crypto_secretstream_xchacha20poly1305_rekey(inout mixed $state): void;

///// Ed25519 primitives.

<<__Native>>
function sodium_crypto_core_ed25519_is_valid_point(string $point): bool;

<<__Native>>
function sodium_crypto_core_ed25519_add(string $point_a, string $point_b): string;

<<__Native>>
function sodium_crypto_core_ed25519_sub(string $point_a, string $point_b): string;

<<__Native>>
function sodium_crypto_scalarmult_ed25519_noclamp(string $scalar, string $point): string;

<<__Native>>
function sodium_crypto_core_ed25519_scalar_reduce(string $scalar): string;

<<__Native>>
function sodium_crypto_core_ed25519_scalar_add(string $scalar_a, string $scalar_b): string;

<<__Native>>
function sodium_crypto_core_ed25519_scalar_mul(string $scalar_a, string $scalar_b): string;

<<__Native>>
function sodium_crypto_scalarmult_ed25519_base(string $scalar): string;

<<__Native>>
function sodium_crypto_scalarmult_ed25519_base_noclamp(string $scalar): string;
