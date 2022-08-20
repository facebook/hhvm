#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# This script exists to generate the SSL certs used in unit tests.
# It shouldn't really ever be run again until the year 2041
# (when these certs expire)., unless you need to change
# or update attributes of the certificate (Common Name, Organization,
# whatever).

set -e

DAYS=10000
CA_KEY=ca_key.pem
CA_CERT=ca_cert.pem
CA_CERT_SRL=ca_cert.srl

# Generate Certificate Authority Key
openssl genrsa -out "${CA_KEY}" 2048

# Generate Self-Signed Certificate Authority Cert
openssl req -x509 -new -nodes \
    -key "${CA_KEY}" \
    -days "${DAYS}" \
    -out "${CA_CERT}" \
    -subj '/C=US/O=Asox/CN=Asox Certification Authority'

# CA serial number
echo 00000009 > ${CA_CERT_SRL}

function generateCert() {
    local KEYPREFIX="$1"
    local CN=$2
    local TEST_KEY="${KEYPREFIX}_key.pem"
    local TEST_CERT="${KEYPREFIX}_cert.pem"
    local TEST_CERT_CSR="${KEYPREFIX}_cert.csr"

    cat > conf.tmp <<EOF
prompt = no
[req]
distinguished_name = req_distinguished_name
req_extensions = v3_req

[req_distinguished_name]
countryName = US
stateOrProvinceName = Ohio
localityName = Hilliard
commonName = $CN

[v3_req]
# Extensions to add to a certificate request
# basicConstraints = CA:FALSE
# keyUsage = nonRepudiation, digitalSignature, keyEncipherment
subjectAltName = IP:$3, IP:$4
EOF

    # Generate the test key
    openssl genrsa -out "${TEST_KEY}" 2048

    # Generate the test key certificate request
    openssl req -new -nodes \
	-config conf.tmp \
	-key "${TEST_KEY}" \
	-days "${DAYS}" \
	-out "${TEST_CERT_CSR}" \
	-subj '/C=US/O=Asox/CN=Asox Town Limited'

    # Sign the test key
    openssl x509 -req \
	-extensions v3_req \
	-extfile conf.tmp \
	-days "${DAYS}" \
	-in "${TEST_CERT_CSR}" \
	-CA "${CA_CERT}" \
	-CAkey "${CA_KEY}" \
	-out "${TEST_CERT}"

    # Clean up the signing request
    rm "${TEST_CERT_CSR}" conf.tmp
}
