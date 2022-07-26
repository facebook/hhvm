// Copyright (c) Meta Platforms, Inc. and affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

use bstr::ByteSlice;
use once_cell::sync::Lazy;
use regex::bytes::Regex;

/// This crate is a port of hphp/hack/src/utils/signed_source.ml, which was
/// based on a historical version of fbsource/tools/signedsource.py.

/// The signing token, which you must embed in the file you wish to sign.
/// Generally, you should put this in a header comment.
pub static SIGNING_TOKEN: &str = concat!(
    "@",
    "generated",
    " ",
    "<<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>"
);

/// Sign a source file into which you have previously embedded a signing token.
/// Signing modifies only the signing token, so the semantics of the file will
/// not change if the token is put in a comment.
///
/// Returns `TokenNotFoundError` if no signing token is present.
pub fn sign_file(data: &[u8]) -> Result<Vec<u8>, TokenNotFoundError> {
    let data = SIGN_OR_OLD_TOKEN.replace_all(data, TOKEN.as_bytes());
    if !data.contains_str(TOKEN) {
        return Err(TokenNotFoundError);
    }
    let signature = format!("SignedSource<<{}>>", hash(&data));
    Ok(TOKEN_REGEX
        .replace_all(&data, signature.as_bytes())
        .into_owned())
}

/// Sign a UTF-8 source file into which you have previously embedded a signing
/// token. Signing modifies only the signing token, so the semantics of the file
/// will not change if the token is put in a comment.
///
/// Returns `TokenNotFoundError` if no signing token is present.
pub fn sign_utf8_file(data: &str) -> Result<String, TokenNotFoundError> {
    let data = sign_file(data.as_bytes())?;
    // SAFETY: `data` was a valid `&str` before signing, and signing only
    // replaces ASCII characters with other ASCII characters.
    unsafe { Ok(String::from_utf8_unchecked(data)) }
}

/// Determine whether a file is signed. This does NOT verify the signature.
pub fn is_signed(data: &[u8]) -> bool {
    SIGNING_REGEX.is_match(data)
}

#[derive(Copy, Clone, Debug, PartialEq, Eq)]
pub enum SignCheckResponse {
    Ok,
    Unsigned,
    Invalid,
}

/// Verify a file's signature.
pub fn verify_signature(data: &[u8]) -> SignCheckResponse {
    let expected_md5 = match SIGNING_REGEX.captures(data) {
        None => return SignCheckResponse::Unsigned,
        Some(caps) => match caps.get(1) {
            None => return SignCheckResponse::Unsigned,
            Some(cap) => cap.as_bytes(),
        },
    };
    for tok in [TOKEN, OLD_TOKEN] {
        let replacement = make_signing_token(tok);
        let unsigned_data = SIGNING_REGEX.replace_all(data, replacement.as_bytes());
        let actual_md5 = hash(&unsigned_data);
        if expected_md5 == actual_md5.as_bytes() {
            return SignCheckResponse::Ok;
        }
    }
    SignCheckResponse::Invalid
}

static TOKEN: &str = "<<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>";

/// This old token was historically used as the signing token. It was replaced
/// because it is 2 characters shorter than the final signature, and as a result,
/// signing data with the old token forced the entire string to be rewritten
/// (everything after the token needs to be shifted forwards 2 bytes).
/// In this implementation, we rewrite the entire string anyway.
static OLD_TOKEN: &str = "<<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@I>>";

fn make_signing_token(token: &str) -> String {
    format!("@{} {}", "generated", token)
}

static SIGNATURE_RE: &str = r"SignedSource<<([a-f0-9]+)>>";

static SIGN_OR_OLD_TOKEN: Lazy<Regex> =
    Lazy::new(|| Regex::new(&format!("{}|{}", SIGNATURE_RE, regex::escape(OLD_TOKEN))).unwrap());

static SIGNING_REGEX: Lazy<Regex> =
    Lazy::new(|| Regex::new(&make_signing_token(SIGNATURE_RE)).unwrap());

static TOKEN_REGEX: Lazy<Regex> = Lazy::new(|| Regex::new(&regex::escape(TOKEN)).unwrap());

fn hash(data: &[u8]) -> String {
    use md5::Digest;
    let mut digest = md5::Md5::new();
    digest.update(&data);
    hex::encode(digest.finalize())
}

#[derive(Debug, thiserror::Error, PartialEq, Eq)]
#[error("Failed to sign file: input does not contain signing token")]
pub struct TokenNotFoundError;

#[cfg(test)]
mod test {
    use super::is_signed;
    use super::make_signing_token;
    use super::sign_utf8_file;
    use super::verify_signature;
    use super::SignCheckResponse;
    use super::TokenNotFoundError;
    use super::SIGNING_TOKEN;
    use super::TOKEN;

    static NO_TOKEN: &str = concat!("// @", "generated\nfn foo() {}");
    static INVALID: &str = concat!(
        "// @",
        "generated SignedSource<<48ab1081d9394843f184debf0b251a18>>\nfn foo() {}"
    );
    static UNSIGNED: &str = concat!(
        "// @",
        "generated <<SignedSource::*O*zOeWoEQle#+L!plEphiEmie@IsG>>\nfn foo() {}"
    );
    // Below signature was manually verified to be equal to the OCaml
    // Signed_source output for `UNSIGNED`.
    static SIGNED: &str = concat!(
        "// @",
        "generated SignedSource<<38ab1081d9394843f184debf0b251a18>>\nfn foo() {}"
    );

    #[test]
    fn test_signing_token() {
        // We use `concat!` so that `SIGNING_TOKEN` can be a `&str` rather than
        // a `Lazy`, since `make_signing_token` can't be a `const fn` yet.
        // Verify that we're producing the same result.
        assert_eq!(SIGNING_TOKEN, make_signing_token(TOKEN))
    }

    #[test]
    fn test_sign_utf8_file() {
        assert_eq!(sign_utf8_file(UNSIGNED), Ok(SIGNED.to_owned()));
        assert_eq!(sign_utf8_file(SIGNED), Ok(SIGNED.to_owned()));
        assert_eq!(sign_utf8_file(NO_TOKEN), Err(TokenNotFoundError));
    }

    #[test]
    fn test_is_signed() {
        assert!(is_signed(SIGNED.as_bytes()));
        assert!(is_signed(INVALID.as_bytes())); // `is_signed` doesn't validate
        assert!(!is_signed(NO_TOKEN.as_bytes()));
        assert!(!is_signed(UNSIGNED.as_bytes()));
    }

    #[test]
    fn test_verify_signature() {
        assert_eq!(verify_signature(SIGNED.as_bytes()), SignCheckResponse::Ok);
        assert_eq!(
            verify_signature(INVALID.as_bytes()),
            SignCheckResponse::Invalid
        );
        assert_eq!(
            verify_signature(NO_TOKEN.as_bytes()),
            SignCheckResponse::Unsigned
        );
        assert_eq!(
            verify_signature(UNSIGNED.as_bytes()),
            SignCheckResponse::Unsigned
        );
    }
}
