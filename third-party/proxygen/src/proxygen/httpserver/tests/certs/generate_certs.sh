#!/bin/bash -ue
# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

CERT_LIFETIME_DAYS=${CERT_LIFETIME_DAYS:-36500}

extensions() {
  cat << EOF
[ca]
basicConstraints        = critical, CA:TRUE
subjectKeyIdentifier    = hash
authorityKeyIdentifier  = keyid:always, issuer:always
keyUsage                = critical, cRLSign, digitalSignature, keyCertSign

[client]
basicConstraints        = critical, CA:FALSE
subjectKeyIdentifier    = hash
authorityKeyIdentifier  = keyid:always
keyUsage                = critical, nonRepudiation, digitalSignature, keyEncipherment
extendedKeyUsage        = critical, clientAuth

[server]
basicConstraints        = critical, CA:FALSE
subjectKeyIdentifier    = hash
authorityKeyIdentifier  = keyid:always
keyUsage                = critical, nonRepudiation, digitalSignature, keyEncipherment, keyAgreement
extendedKeyUsage        = critical, serverAuth
subjectAltName          = IP:127.0.0.1,IP:::1

[client_and_server]
basicConstraints        = critical, CA:FALSE
subjectKeyIdentifier    = hash
authorityKeyIdentifier  = keyid:always
keyUsage                = critical, nonRepudiation, digitalSignature, keyEncipherment, keyAgreement
extendedKeyUsage        = critical, serverAuth, clientAuth
subjectAltName          = IP:127.0.0.1,IP:::1
EOF
}

die() {
  echo "$@" >&2
  exit 1
}

generate_key() {
  keytype="$1"
  modulus_bits="$2"
  outfile="$3"

  [ "$keytype" != "rsa" ] && die "unsupported key type"

  openssl genrsa -out "$outfile" "$modulus_bits" 2>/dev/null
}

mkcsr() {
  local key="$1"
  local name="$2"
  local output="$3"

  openssl req -new \
    -sha256 \
    -key "$key" \
    -keyform PEM \
    -subj "/CN=$name/" \
    -outform PEM \
    -out "$output" \

}

selfsign() {
  local incsr="$1"
  local inkey="$2"
  local outfile="$3"

  openssl x509 \
    -req \
    -set_serial 1 \
    -signkey "$inkey" \
    -inform PEM \
    -in "$incsr" \
    -outform PEM \
    -out "$outfile" \
    -days "$CERT_LIFETIME_DAYS" \
    -extfile <(extensions) \
    -extensions "ca" \

}

sign() {
  csr="$1"
  cacert="$2"
  cakey="$3"
  serial="$4"
  extensions="$5"
  outfile="$6"


  openssl x509 \
    -req \
    -set_serial "$serial" \
    -CA "$cacert" \
    -CAkey "$cakey" \
    -inform PEM \
    -in "$csr" \
    -outform PEM \
    -out "$outfile" \
    -days "$CERT_LIFETIME_DAYS" \
    -extfile <(extensions) \
    -extensions "$extensions" \

}
# ca_cert.pem
generate_key rsa 2048 ca_key.pem
mkcsr ca_key.pem "Asox Certification Authority" ca_key.csr.pem
selfsign ca_key.csr.pem ca_key.pem ca_cert.pem

# Client CA
generate_key rsa 2048 client_ca_key.pem
mkcsr client_ca_key.pem "Asox Certification Authority" client_ca.csr.pem
selfsign client_ca.csr.pem client_ca_key.pem client_ca_cert.pem

# client_cert.pem: CN = testuser1, serial 10
generate_key rsa 2048 client_key.pem
mkcsr client_key.pem "testuser1" client_cert.csr.pem
sign client_cert.csr.pem client_ca_cert.pem client_ca_key.pem 10 client client_cert.pem

# test_cert1, CN=test_cert1, Serial number = 1, client/server
generate_key rsa 2048 test_key1.pem
mkcsr test_key1.pem "test_cert1" test_cert1.csr.pem
sign test_cert1.csr.pem client_ca_cert.pem client_ca_key.pem 1 client_and_server test_cert1.pem

# test_cert2, CN=test_cert2, Serial number = 1, client/server
generate_key rsa 2048 test_key2.pem
mkcsr test_key2.pem "test_cert2" test_cert2.csr.pem
sign test_cert2.csr.pem client_ca_cert.pem client_ca_key.pem 1 client_and_server test_cert2.pem

# Remove all of the CSRs but leave the keys in case you need to regenerate a
# different cert with the same key.
rm ./*.csr.pem
