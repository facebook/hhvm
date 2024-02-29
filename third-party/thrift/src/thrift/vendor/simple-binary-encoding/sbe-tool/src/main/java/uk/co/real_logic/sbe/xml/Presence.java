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
package uk.co.real_logic.sbe.xml;

/**
 * Presence attribute values for a {@link Type} applied to a {@link Field}.
 */
public enum Presence
{
    /**
     * The field must be present in the message encoding.
     */
    REQUIRED("required"),

    /**
     * The field value is constant and held in the schema and not passed on the wire.
     */
    CONSTANT("constant"),

    /**
     * The field is optional and an optional value must be provided in the schema.
     */
    OPTIONAL("optional");

    private static final Presence[] VALUES = values();

    private final String value;

    Presence(final String value)
    {
        this.value = value;
    }

    /**
     * The value as a String of the presence.
     *
     * @return the value as a String
     */
    public String value()
    {
        return value;
    }

    /**
     * Lookup Presence name and return enum.
     *
     * @param name of presence to get
     * @return the {@link Presence} matching the name
     * @throws IllegalArgumentException if the name is not found
     */
    public static Presence get(final String name)
    {
        for (final Presence p : VALUES)
        {
            if (p.value.equals(name))
            {
                return p;
            }
        }

        throw new IllegalArgumentException("No Presence for name: " + name);
    }
}
