/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <fizz/extensions/delegatedcred/SelfDelegatedCredential.h>
#include <fizz/extensions/delegatedcred/Types.h>

#pragma once

namespace fizz {
namespace extensions {
/*
 * Returns PEM entry for a delegated credential. This includes the Delegated
 * Credential itself and it's corresponding keys. Note there are distinct labels
 * for client and server modes. This does not include the leaf cert that signed
 * the credential, the caller is expected to append that themselves.
 */
std::string generateDelegatedCredentialPEM(
    DelegatedCredentialMode mode,
    DelegatedCredential credential,
    std::string credKeyData);
/*
 * Takes in a string which we expect to be the Combined PEM including
 * the leaf cert, the credential private key and the credential itself.
 * The caller must specify whether they want to load a client or server
 * delegated credential, if such a credential doesn't exist we will throw.
 * Will return a cert that has the credential in its extensions.
 * This currently only supports P256 Delegated credentials.
 * If we use a different signature algorithm or the pem does not contain
 * a valid delegated credentail this will throw, otherwise this will return
 * a valid non null ptr;
 */
std::unique_ptr<SelfDelegatedCredential> loadDCFromPEM(
    std::string combinedPemData,
    DelegatedCredentialMode mode);
} // namespace extensions
} // namespace fizz
