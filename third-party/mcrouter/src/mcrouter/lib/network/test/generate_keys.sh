#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# we assume the generate_key-inl.sh is in the same dir as this script
ROOT=$(dirname "$0")
. "$ROOT/generate_keys-inl.sh"

generateCert test Asox 127.0.0.1 ::1
generateCert broken Asox 0.0.0.0 ::0

# Clean up serial number
rm "${CA_CERT_SRL}"
