#include <fizz/extensions/delegatedcred/SelfDelegatedCredential.h>
#include <fizz/extensions/delegatedcred/Types.h>

#pragma once

namespace fizz {
namespace extensions {
/*
 * We expect certData and credKeyData to already be in PEM format with their
 * associated labels. We simply append the cert and key data so we expect these
 * to be non combined pems.  This returns one combined pem
 */
std::string generateDelegatedCredentialPEM(
    DelegatedCredential credential,
    std::string certData,
    std::string credKeyData);
/*
 * Takes in a string which we expect to be the Combined PEM including
 * the leaf cert, the credential private key and the credential itself.
 * Will return a cert that has the credential in its extensions.
 * This currently only supports P256 Delegated credentials.
 * If we use a different signature algorithm or the pem does not contain
 * a valid delegated credentail this will throw, otherwise this will return
 * a valid non null ptr;
 */
std::unique_ptr<SelfDelegatedCredential> loadDCFromPEM(
    std::string combinedPemData);
} // namespace extensions
} // namespace fizz
