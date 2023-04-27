/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

namespace carbon {

class CarbonProtocolReader;

class CarbonQueueAppenderStorage;

template <class TS>
class CarbonProtocolWriterImpl;

using CarbonProtocolWriter =
    CarbonProtocolWriterImpl<CarbonQueueAppenderStorage>;

/*
 * A user type may be used in a Carbon structure if the user provides an
 * appropriate specialization of carbon::SerializationTraits.  The following
 * methods should be provided in such a specialization:
 *
 * template <class T>
 * struct SerializationTraits;
 *   static T read(CarbonProtocolReader&);
 *   static void write(const T&, CarbonProtocolReader&);
 *   static bool isEmpty(const T&);
 * };
 */
template <class T, class Enable = void>
struct SerializationTraits;

} // namespace carbon
