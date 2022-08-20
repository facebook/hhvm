/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/hpke/Types.h>

#include <fizz/crypto/aead/Aead.h>
#include <fizz/crypto/exchange/KeyExchange.h>
#include <fizz/crypto/hpke/Hkdf.h>
#include <fizz/protocol/Types.h>

namespace fizz {
namespace hpke {

/**
 * Generates the `suite_id` string (used in HPKE LabeledExtract/LabelExpand).
 *
 * For more information on the construction of `suite_id`, refer to Section 4
 * on the HPKE specification.
 *
 * This is intended to be used in constructing an `HpkeContext` structure.
 *
 * @param group   A `fizz::NamedGroup` representing the group used in an HPKE
 *                instantiation.
 * @param hash    A `fizz::HashFunction` representing the hash function used
 *                in the KDF for the HPKE instantiation.
 * @param suite   A `fizz::CipherSuite` representing the AEAD used in the HPKE
 *                instantiation.
 *
 * @return An HPKE `suite_id` string.
 */
HpkeSuiteId
generateHpkeSuiteId(NamedGroup group, HashFunction hash, CipherSuite suite);

/**
 * Generates the `suite_id` string (used in HPKE LabeledExtract/LabelExpand).
 *
 * For more information on the construction of `suite_id`, refer to Section 4
 * on the HPKE specification.
 *
 * This is intended to be used in constructing an `HpkeContext` structure.
 *
 * @param kem     The HPKE KEM code point for the intended HPKE context.
 * @param kdf     The HPKE KDF code point for the intended HPKE context.
 * @param aead    The HPKE AEAD code point for the intended HPKE context.
 *
 * @return An HPKE `suite_id` string.
 */
HpkeSuiteId generateHpkeSuiteId(KEMId kem, KDFId kdf, AeadId aead);

/*****************************
 *                           *
 * Key Encapsulation (KEM)   *
 *                           *
 *****************************/

/**
 * fizz::hpke::getKEMId converts a `fizz:NamedGroup` code point to the
 * corresponding HPKE KEM code point.
 *
 * @param group A `fizz::NamedGroup` code point
 * @return The corresponding HPKE KEM code point
 * @throws std::runtime_error On invalid code points.
 */
KEMId getKEMId(NamedGroup group);

/**
 * fizz::hpke::getKexGroup converts an HPKE KDF code point to a
 * `fizz::NamedGroup` code point
 *
 * @param kdfId  An HPKE KDF code point.
 * @return The corresponding `fizz::NamedGroup` code point.
 * @throws std::runtime_error On invalid code points.
 */
NamedGroup getKexGroup(KEMId kemId);

/**
 * fizz::hpke::makeKeyExchange returns a `fizz::KeyExchange` instance given an
 * HPKE code point.
 *
 * The constructed `KeyExchange` object does *not* have a keypair associated
 * with it yet.
 *
 * @param  kemId    An HPKE KEM code point.
 *
 * @return An instance of a `fizz::KeyExchange` object without an associated
 *         keypair
 * @throws std::runtime_error   On invalid code points.
 */
std::unique_ptr<KeyExchange> makeKeyExchange(KEMId kemId);

/**
 * fizz::hpke::nenc returns the size of the serialized public component (`enc`)
 * for a given keypair.
 *
 * @param  kemId    An HPKE KEM code point.
 *
 * @return A non zero value indicating the size of the public component for the
 *         given KEM.
 * @throws std::runtime_error   On invalid code points.
 */
size_t nenc(KEMId kemId);

/*****************************
 *                           *
 * Key Derivation (KDF)      *
 *                           *
 *****************************/

/**
 * fizz::hpke::getHashFunction converts an HPKE KDF code point to a
 * `fizz::HashFunction` code point
 *
 * @param kdfId  An HPKE KDF code point.
 * @return The corresponding `fizz::HashFunction` code point.
 * @throws std::runtime_error On invalid code points.
 */
HashFunction getHashFunction(KDFId kdfId);

/**
 * fizz::hpke::getKDFId converts a `fizz::HashFunction` code point to the
 * corresponding HPKE KDF code point.
 *
 * @param hash  A `fizz::HashFunction` code point.
 * @return The corresponding HPKE code point.
 * @throws std::runtime_error On invalid code points
 */
KDFId getKDFId(HashFunction hash);

/**
 * fizz::hpke::makeHpkeHkdf constructs an `fizz::Hkdf` instance from an HPKE
 * code point.
 *
 * @param prefix  A prefix string that contextualizes the HKDF instance.
 *
 *                This prefix string is passed through *as-is*. This function
 *                does *not* add any HPKE specific prefixes.
 *
 * @param kdfId   An HPKE HKDF code point.
 *
 * @return  An instantiated `fizz::Hkdf` instance.
 * @throws std::runtime_error  On invalid code points.
 */
std::unique_ptr<Hkdf> makeHpkeHkdf(
    std::unique_ptr<folly::IOBuf> prefix,
    KDFId kdfId);

/*****************************
 *                           *
 * Symmetric Encryption      *
 *                           *
 *****************************/

/**
 * fizz::hpke::getAeadId converts a TLS CipherSuite code point to the
 * corresponding HPKE AEAD code point.
 *
 * @param suite  The TLS cipher suite
 * @return The corresponding HPKE AEAD code point. The digest portion of the
 *         TLS ciphersuite is dropped.
 * @throws std::runtime_error On invalid code points.
 */
AeadId getAeadId(CipherSuite suite);

/**
 * fizz::hpke::getCipherSuite converts an HPKE AEAD code point to a TLS
 * CipherSuite code point.
 *
 * @param  aeadId  An HPKE AEAD code point
 *
 * @return The corresponding TLS CipherSuite code point.
 * @throws std::runtime_error On invalid code points.
 */
CipherSuite getCipherSuite(AeadId aeadId);

/**
 * fizz::hpke::makeCipher constructs a `fizz::Aead` instance given an HPKE
 * aead code point.
 *
 * The cipher instance does *not* have the keys set.
 *
 * @param aeadId   An HPKE aead code point.
 *
 * @return An instance of a `fizz::Aead` without keying parameters.
 * @throws std::runtime_error     On invalid aead code point.
 */
std::unique_ptr<Aead> makeCipher(AeadId aeadId);
} // namespace hpke
} // namespace fizz
