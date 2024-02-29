/*
 * Copyright 2013-2024 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _OTF_TOKEN_H
#define _OTF_TOKEN_H

#include <cstdint>
#include <string>

#include "Encoding.h"

namespace sbe { namespace otf {

/// Constants used for holding Token signals
enum class Signal : int
{
    /// Begins a message. Is followed by a number of tokens in the message and terminated by an end message.
        BEGIN_MESSAGE = 1,
    /// Ends a message.
        END_MESSAGE = 2,
    /// Begins a composite. Is followed by a number of tokens in the composite and terminated by an end composite.
        BEGIN_COMPOSITE = 3,
    /// Ends a composite.
        END_COMPOSITE = 4,
    /// Begins a field. Is followed by a number of tokens in the field and terminated by an end field.
        BEGIN_FIELD = 5,
    /// Ends a field.
        END_FIELD = 6,
    /// Begins a repeating group. Is followed by a number of tokens in the group and terminated by an end group.
        BEGIN_GROUP = 7,
    /// Ends a repeating group.
        END_GROUP = 8,
    /// Begins an enumeration. Is followed by a number of tokens in the enumeration and terminated by an end enum.
        BEGIN_ENUM = 9,
    /// Indicates a valid value for an enumeration. Must appear between a begin/end enum pair.
        VALID_VALUE = 10,
    /// Ends an enumeration.
        END_ENUM = 11,
    /// Begins a bit set. Is followed by a number of tokens in the set and terminated by an end set
        BEGIN_SET = 12,
    /// Indicates a bit value in the bit set. Must appear between a begin/end set pair.
        CHOICE = 13,
    /// Ends a bit set.
        END_SET = 14,
    /// Begins a variable length data element. Is followed by a number of tokens in the element and terminated by an end var data.
        BEGIN_VAR_DATA = 15,
    /// Ends a variable length data element.
        END_VAR_DATA = 16,
    /// Indicates an encoding of a primitive element.
        ENCODING = 17
};

/*
 * Hold the state for a single token in the IR
 */
class Token
{
public:
    Token(
        std::int32_t offset,
        std::int32_t fieldId,
        std::int32_t version,
        std::int32_t encodedLength,
        std::int32_t componentTokenCount,
        Signal signal,
        std::string name,
        std::string description,
        Encoding encoding) :
        m_offset(offset),
        m_fieldId(fieldId),
        m_version(version),
        m_encodedLength(encodedLength),
        m_componentTokenCount(componentTokenCount),
        m_signal(signal),
        m_name(std::move(name)),
        m_description(std::move(description)),
        m_encoding(std::move(encoding))
    {
    }

    inline Signal signal() const
    {
        return m_signal;
    }

    inline const std::string &name() const
    {
        return m_name;
    }

    inline const std::string &description() const
    {
        return m_description;
    }

    inline std::int32_t fieldId() const
    {
        return m_fieldId;
    }

    inline std::int32_t tokenVersion() const
    {
        return m_version;
    }

    inline const Encoding &encoding() const
    {
        return m_encoding;
    }

    inline std::int32_t encodedLength() const
    {
        return m_encodedLength;
    }

    inline std::int32_t offset() const
    {
        return m_offset;
    }

    inline std::int32_t componentTokenCount() const
    {
        return m_componentTokenCount;
    }

    inline bool isConstantEncoding() const
    {
        return m_encoding.presence() == Presence::SBE_CONSTANT;
    }

private:
    const std::int32_t m_offset;
    const std::int32_t m_fieldId;
    const std::int32_t m_version;
    const std::int32_t m_encodedLength;
    const std::int32_t m_componentTokenCount;
    const Signal m_signal;
    const std::string m_name;
    const std::string m_description;
    const Encoding m_encoding;
};

}}

#endif
