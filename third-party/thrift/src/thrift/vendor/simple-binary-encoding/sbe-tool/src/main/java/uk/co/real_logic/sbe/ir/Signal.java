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
package uk.co.real_logic.sbe.ir;

/**
 * Signal the {@link uk.co.real_logic.sbe.ir.Token} role within a stream of tokens. These signals begin/end various
 * entities such as fields, composites, messages, repeating groups, enumerations, bitsets, etc.
 */
public enum Signal
{
    /**
     * Denotes the beginning of a message.
     */
    BEGIN_MESSAGE,

    /**
     * Denotes the end of a message.
     */
    END_MESSAGE,

    /**
     * Denotes the beginning of a composite.
     */
    BEGIN_COMPOSITE,

    /**
     * Denotes the end of a composite.
     */
    END_COMPOSITE,

    /**
     * Denotes the beginning of a field.
     */
    BEGIN_FIELD,

    /**
     * Denotes the end of a field.
     */
    END_FIELD,

    /**
     * Denotes the beginning of a repeating group.
     */
    BEGIN_GROUP,

    /**
     * Denotes the end of a repeating group.
     */
    END_GROUP,

    /**
     * Denotes the beginning of an enumeration.
     */
    BEGIN_ENUM,

    /**
     * Denotes a value of an enumeration.
     */
    VALID_VALUE,

    /**
     * Denotes the end of an enumeration.
     */
    END_ENUM,

    /**
     * Denotes the beginning of a bitset.
     */
    BEGIN_SET,

    /**
     * Denotes a bit value (choice) of a bitset.
     */
    CHOICE,

    /**
     * Denotes the end of a bitset.
     */
    END_SET,

    /**
     * Denotes the beginning of a variable data block.
     */
    BEGIN_VAR_DATA,

    /**
     * Denotes the end of a variable data block.
     */
    END_VAR_DATA,

    /**
     * Denotes the {@link uk.co.real_logic.sbe.ir.Token} is an encoding.
     */
    ENCODING
}
