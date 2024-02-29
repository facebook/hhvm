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

import uk.co.real_logic.sbe.PrimitiveType;
import uk.co.real_logic.sbe.PrimitiveValue;
import org.agrona.Verify;

import java.nio.ByteOrder;

/**
 * Optional encoding settings that can be associated with {@link Token}s.
 */
public class Encoding
{
    /**
     * Indicates the presence status of a primitive encoded field in a message.
     */
    public enum Presence
    {
        /**
         * The field presence is required.
         */
        REQUIRED,

        /**
         * The field presence is optional.
         */
        OPTIONAL,

        /**
         * The field presence is a constant.
         */
        CONSTANT
    }

    private Presence presence;
    private final PrimitiveType primitiveType;
    private final ByteOrder byteOrder;
    private final PrimitiveValue minValue;
    private final PrimitiveValue maxValue;
    private final PrimitiveValue nullValue;
    private final PrimitiveValue constValue;
    private final String characterEncoding;
    private final String epoch;
    private final String timeUnit;
    private final String semanticType;

    Encoding()
    {
        presence = Presence.REQUIRED;
        primitiveType = null;
        byteOrder = ByteOrder.LITTLE_ENDIAN;
        minValue = null;
        maxValue = null;
        nullValue = null;
        constValue = null;
        characterEncoding = null;
        epoch = null;
        timeUnit = null;
        semanticType = null;
    }

    Encoding(
        final Presence presence,
        final PrimitiveType primitiveType,
        final ByteOrder byteOrder,
        final PrimitiveValue minValue,
        final PrimitiveValue maxValue,
        final PrimitiveValue nullValue,
        final PrimitiveValue constValue,
        final String characterEncoding,
        final String epoch,
        final String timeUnit,
        final String semanticType)
    {
        Verify.notNull(presence, "presence");
        Verify.notNull(byteOrder, "byteOrder");

        this.primitiveType = primitiveType;
        this.presence = presence;
        this.byteOrder = byteOrder;
        this.minValue = minValue;
        this.maxValue = maxValue;
        this.nullValue = nullValue;
        this.constValue = constValue;
        this.characterEncoding = characterEncoding;
        this.epoch = epoch;
        this.timeUnit = timeUnit;
        this.semanticType = semanticType;
    }

    /**
     * The {@link PrimitiveType} of this encoding.
     *
     * @return the {@link PrimitiveType} of this encoding.
     */
    public PrimitiveType primitiveType()
    {
        return primitiveType;
    }

    /**
     * The {@link ByteOrder} for this encoding.
     *
     * @return the {@link ByteOrder} for this encoding.
     */
    public ByteOrder byteOrder()
    {
        return byteOrder;
    }

    /**
     * The min value for the token or null if not set.
     *
     * @return the minValue for the token or null if not set.
     */
    public PrimitiveValue minValue()
    {
        return minValue;
    }

    /**
     * The max value for the token or null if not set.
     *
     * @return the maxValue for the token or null if not set.
     */
    public PrimitiveValue maxValue()
    {
        return maxValue;
    }

    /**
     * The null value for the token or null if not set.
     *
     * @return the nullValue for the token or null if not set.
     */
    public PrimitiveValue nullValue()
    {
        return nullValue;
    }

    /**
     * The constant value for the token or null if not set.
     *
     * @return the constant value for the token or null if not set.
     */
    public PrimitiveValue constValue()
    {
        return constValue;
    }

    /**
     * Indicates the presence status of a field in a message.
     *
     * @return indicates the presence status of a field in a message.
     */
    public Presence presence()
    {
        return presence;
    }

    /**
     * Set the {@link Presence} for this encoding.
     *
     * @param presence the {@link Presence} for this encoding.
     */
    public void presence(final Presence presence)
    {
        this.presence = presence;
    }

    /**
     * The most applicable null value for the encoded type.
     *
     * @return most applicable null value for the encoded type.
     */
    public PrimitiveValue applicableNullValue()
    {
        if (null != nullValue)
        {
            return nullValue;
        }

        return primitiveType.nullValue();
    }

    /**
     * The most applicable min value for the encoded type.
     *
     * @return most applicable min value for the encoded type.
     */
    public PrimitiveValue applicableMinValue()
    {
        if (null != minValue)
        {
            return minValue;
        }

        return primitiveType.minValue();
    }

    /**
     * The most applicable max value for the encoded type.
     *
     * @return most applicable max value for the encoded type.
     */
    public PrimitiveValue applicableMaxValue()
    {
        if (null != maxValue)
        {
            return maxValue;
        }

        return primitiveType.maxValue();
    }

    /**
     * The character encoding for the token or null if not set.
     *
     * @return the character encoding for the token or null if not set.
     */
    public String characterEncoding()
    {
        return characterEncoding;
    }

    /**
     * The epoch from which a timestamp is offset. The default is "unix".
     *
     * @return the epoch from which a timestamp is offset.
     */
    public String epoch()
    {
        return epoch;
    }

    /**
     * The time unit of the timestamp.
     *
     * @return the time unit of the timestamp.
     */
    public String timeUnit()
    {
        return timeUnit;
    }

    /**
     * The semantic type of encoding value which can have relevance to the application layer.
     *
     * @return semantic type of encoding value which can have relevance to the application layer.
     */
    public String semanticType()
    {
        return semanticType;
    }

    /**
     * {@inheritDoc}
     */
    public String toString()
    {
        return
            "Encoding{" +
                "presence=" + presence +
                ", primitiveType=" + primitiveType +
                ", byteOrder=" + byteOrder +
                ", minValue=" + minValue +
                ", maxValue=" + maxValue +
                ", nullValue=" + nullValue +
                ", constValue=" + constValue +
                ", characterEncoding='" + characterEncoding + '\'' +
                ", epoch='" + epoch + '\'' +
                ", timeUnit=" + timeUnit +
                ", semanticType='" + semanticType + '\'' +
                '}';
    }

    /**
     * Builder to make {@link Encoding} easier to create.
     */
    public static class Builder
    {
        private PrimitiveType primitiveType = null;
        private Presence presence = Presence.REQUIRED;
        private ByteOrder byteOrder = ByteOrder.LITTLE_ENDIAN;
        private PrimitiveValue minValue = null;
        private PrimitiveValue maxValue = null;
        private PrimitiveValue nullValue = null;
        private PrimitiveValue constValue = null;
        private String characterEncoding = null;
        private String epoch = null;
        private String timeUnit = null;
        private String semanticType = null;

        /**
         * PrimitiveType for the Encoding.
         *
         * @param primitiveType for the Encoding.
         * @return this for a fluent API.
         */
        public Builder primitiveType(final PrimitiveType primitiveType)
        {
            this.primitiveType = primitiveType;
            return this;
        }

        /**
         * Presence for the Encoding.
         *
         * @param presence for the Encoding.
         * @return this for a fluent API.
         */
        public Builder presence(final Presence presence)
        {
            this.presence = presence;
            return this;
        }

        /**
         * ByteOrder for the Encoding.
         *
         * @param byteOrder for the Encoding.
         * @return this for a fluent API.
         */
        public Builder byteOrder(final ByteOrder byteOrder)
        {
            this.byteOrder = byteOrder;
            return this;
        }

        /**
         * Minimum value for the Encoding.
         *
         * @param minValue for the Encoding.
         * @return this for a fluent API.
         */
        public Builder minValue(final PrimitiveValue minValue)
        {
            this.minValue = minValue;
            return this;
        }

        /**
         * Maximum value for the Encoding.
         *
         * @param maxValue for the Encoding.
         * @return this for a fluent API.
         */
        public Builder maxValue(final PrimitiveValue maxValue)
        {
            this.maxValue = maxValue;
            return this;
        }

        /**
         * Null value for the Encoding.
         *
         * @param nullValue for the Encoding.
         * @return this for a fluent API.
         */
        public Builder nullValue(final PrimitiveValue nullValue)
        {
            this.nullValue = nullValue;
            return this;
        }

        /**
         * Constant value to be used for the Encoding.
         *
         * @param constValue for the Encoding.
         * @return this for a fluent API.
         */
        public Builder constValue(final PrimitiveValue constValue)
        {
            this.constValue = constValue;
            return this;
        }

        /**
         * Character encoding for the Encoding.
         *
         * @param characterEncoding for the Encoding.
         * @return this for a fluent API.
         */
        public Builder characterEncoding(final String characterEncoding)
        {
            this.characterEncoding = characterEncoding;
            return this;
        }

        /**
         * Epoch value for the Encoding.
         *
         * @param epoch for the Encoding.
         * @return this for a fluent API.
         */
        public Builder epoch(final String epoch)
        {
            this.epoch = epoch;
            return this;
        }

        /**
         * Time unit for the Encoding.
         *
         * @param timeUnit for the Encoding.
         * @return this for a fluent API.
         */
        public Builder timeUnit(final String timeUnit)
        {
            this.timeUnit = timeUnit;
            return this;
        }

        /**
         * Semantic type for the Encoding.
         *
         * @param semanticType for the Encoding.
         * @return this for a fluent API.
         */
        public Builder semanticType(final String semanticType)
        {
            this.semanticType = semanticType;
            return this;
        }

        /**
         * Build a new Encoding based on the values.
         *
         * @return a new Encoding based on the values.
         */
        public Encoding build()
        {
            return new Encoding(
                presence, primitiveType,
                byteOrder,
                minValue,
                maxValue,
                nullValue,
                constValue,
                characterEncoding,
                epoch,
                timeUnit,
                semanticType);
        }
    }
}
