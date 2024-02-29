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

import org.w3c.dom.Node;

import java.util.List;
import java.util.Map;

import static uk.co.real_logic.sbe.xml.XmlSchemaParser.*;

/**
 * Representation for a field (or group or data) member from the SBE schema.
 */
public class Field
{
    /**
     * Value to indicate a {@link Field} is invalid or not yet set.
     */
    public static final int INVALID_ID = Integer.MAX_VALUE;  // schemaId must be a short, so this is way out of range.

    private final String name;                 // required for field/data & group
    private final String description;          // optional for field/data & group
    private final int id;                      // required for field/data (not present for group)
    private final Type type;                   // required for field/data (not present for group)
    private final int offset;                  // optional for field/data (not present for group)
    private final String semanticType;         // optional for field/data (not present for group?)
    private final Presence presence;           // optional, defaults to required
    private final String valueRef;             // optional, defaults to null
    private final int blockLength;             // optional for group (not present for field/data)
    private final CompositeType dimensionType; // required for group (not present for field/data)
    private final boolean variableLength;      // true for data (false for field/group)
    private final int sinceVersion;            // optional
    private final int deprecated;              // optional
    private List<Field> groupFieldList;        // used by group fields as the list of child fields in the group
    private int computedOffset;                // holds the calculated offset of this field from <message> or <group>
    private int computedBlockLength;           // used to hold the calculated block length of this group
    private final String epoch;                // optional, epoch from which a timestamps start, defaults to "unix"
    private final String timeUnit;             // optional, defaults to "nanosecond".

    Field(
        final String name,
        final String description,
        final int id,
        final Type type,
        final int offset,
        final String semanticType,
        final Presence presence,
        final String valueRef,
        final int blockLength,
        final CompositeType dimensionType,
        final boolean variableLength,
        final int sinceVersion,
        final int deprecated,
        final String epoch,
        final String timeUnit)
    {
        this.name = name;
        this.description = description;
        this.id = id;
        this.type = type;
        this.offset = offset;
        this.semanticType = semanticType;
        this.presence = presence;
        this.valueRef = valueRef;
        this.blockLength = blockLength;
        this.dimensionType = dimensionType;
        this.variableLength = variableLength;
        this.sinceVersion = sinceVersion;
        this.deprecated = deprecated;
        this.groupFieldList = null;
        this.computedOffset = 0;
        this.computedBlockLength = 0;
        this.epoch = epoch;
        this.timeUnit = timeUnit;
    }

    Field(final Field field, final List<Field> groupFieldList)
    {
        this(
            field.name,
            field.description,
            field.id,
            field.type,
            field.offset,
            field.semanticType,
            field.presence,
            field.valueRef,
            field.blockLength,
            field.dimensionType,
            field.variableLength,
            field.sinceVersion,
            field.deprecated,
            field.epoch,
            field.timeUnit);
        this.groupFieldList = groupFieldList;
    }

    /**
     * Validate the node is correct for the type.
     *
     * @param node          in the XML.
     * @param typeByNameMap for validating refs.
     */
    public void validate(final Node node, final Map<String, Type> typeByNameMap)
    {
        if (type != null &&
            semanticType != null &&
            type.semanticType() != null &&
            !semanticType.equals(type.semanticType()))
        {
            handleError(node, "Mismatched semanticType on type and field: " + name);
        }

        checkForValidName(node, name);

        if (null != valueRef)
        {
            validateValueRef(node, typeByNameMap);
        }

        if (type instanceof EnumType && presence == Presence.CONSTANT)
        {
            if (null == valueRef)
            {
                handleError(node, "valueRef not set for constant enum");
            }
        }

        if (null != valueRef && presence == Presence.CONSTANT)
        {
            final String valueRefType = valueRef.substring(0, valueRef.indexOf('.'));

            if (!(type instanceof EnumType))
            {
                if (type instanceof EncodedDataType)
                {
                    final EnumType enumType = (EnumType)typeByNameMap.get(valueRefType);

                    if (((EncodedDataType)type).primitiveType() != enumType.encodingType())
                    {
                        handleError(node, "valueRef does not match field type: " + valueRef);
                    }
                }
                else
                {
                    handleError(node, "valueRef does not match field type: " + valueRef);
                }
            }
            else if (!valueRefType.equals(type.name()))
            {
                handleError(node, "valueRef for enum name not found: " + valueRefType);
            }
        }
    }

    /**
     * Set the group fields when a group.
     *
     * @param fields for the group.
     */
    public void groupFields(final List<Field> fields)
    {
        groupFieldList = fields;
    }

    /**
     * Get the list of group fields.
     *
     * @return the list of group fields.
     */
    public List<Field> groupFields()
    {
        return groupFieldList;
    }

    /**
     * Set the computed offset at which the field begins.
     *
     * @param offset at which the field begins.
     */
    public void computedOffset(final int offset)
    {
        computedOffset = offset;
    }

    /**
     * The computed offset at which the field begins.
     *
     * @return the computed offset at which the field begins.
     */
    public int computedOffset()
    {
        return computedOffset;
    }

    /**
     * Name value for the field.
     *
     * @return name value for the field.
     */
    public String name()
    {
        return name;
    }

    /**
     * Description attribute for the field.
     *
     * @return the description attribute for the field.
     */
    public String description()
    {
        return description;
    }

    /**
     * ID attribute for the field.
     *
     * @return the ID attribute for the field.
     */
    public int id()
    {
        return id;
    }

    /**
     * Type attribute for the field.
     *
     * @return the Type attribute for the field.
     */
    public Type type()
    {
        return type;
    }

    /**
     * The offset at which the field begins.
     *
     * @return the offset at which the field begins.
     */
    public int offset()
    {
        return offset;
    }

    /**
     * The block length of the field.
     *
     * @return the block length the field.
     */
    public int blockLength()
    {
        return blockLength;
    }

    /**
     * The computed block length of the field.
     *
     * @param length computed for the block length,
     */
    public void computedBlockLength(final int length)
    {
        computedBlockLength = length;
    }

    /**
     * The computed block length of the field.
     *
     * @return the computed block length the field.
     */
    public int computedBlockLength()
    {
        return computedBlockLength;
    }

    /**
     * Presence attribute for the field.
     *
     * @return the Presence attribute for the field.
     */
    public Presence presence()
    {
        return presence;
    }

    /**
     * Ref attribute for the field.
     *
     * @return the Ref attribute for the field.
     */
    public String valueRef()
    {
        return valueRef;
    }

    /**
     * Semantic type attribute for the field.
     *
     * @return the Semantic type attribute for the field.
     */
    public String semanticType()
    {
        return semanticType;
    }

    /**
     * Dimension type for the field when group or var data.
     *
     * @return the Dimension type for the field when group or var data.
     */
    public CompositeType dimensionType()
    {
        return dimensionType;
    }

    /**
     * Is the field variable length when encoded?
     *
     * @return true if the field is variable length when encoded.
     */
    public boolean isVariableLength()
    {
        return variableLength;
    }

    /**
     * Since version attribute for the field.
     *
     * @return the Since version attribute for the field.
     */
    public int sinceVersion()
    {
        return sinceVersion;
    }

    /**
     * Deprecated version attribute for the field.
     *
     * @return the Deprecated version attribute for the field.
     */
    public int deprecated()
    {
        return deprecated;
    }

    /**
     * Epoch attribute for the field.
     *
     * @return the Epoch attribute for the field.
     */
    public String epoch()
    {
        return epoch;
    }

    /**
     * Time unit attribute for the field.
     *
     * @return the time unit attribute for the field.
     */
    public String timeUnit()
    {
        return timeUnit;
    }

    /**
     * {@inheritDoc}
     */
    public String toString()
    {
        return "Field{name='" + name + '\'' +
            ", description='" + description + '\'' +
            ", id=" + id +
            ", type=" + type +
            ", offset=" + offset +
            ", semanticType='" + semanticType + '\'' +
            ", presence=" + presence +
            ", valueRef='" + valueRef + '\'' +
            ", blockLength=" + blockLength +
            ", dimensionType=" + dimensionType +
            ", variableLength=" + variableLength +
            ", sinceVersion=" + sinceVersion +
            ", deprecated=" + deprecated +
            ", groupFieldList=" + groupFieldList +
            ", computedOffset=" + computedOffset +
            ", computedBlockLength=" + computedBlockLength +
            ", epoch='" + epoch + '\'' +
            ", timeUnit=" + timeUnit +
            '}';
    }

    private void validateValueRef(final Node node, final Map<String, Type> typeByNameMap)
    {
        final int periodIndex = valueRef.indexOf('.');
        if (periodIndex < 1 || periodIndex == (valueRef.length() - 1))
        {
            handleError(node, "valueRef format not valid (enum-name.valid-value-name): " + valueRef);
        }

        final String valueRefType = valueRef.substring(0, periodIndex);
        final Type valueType = typeByNameMap.get(valueRefType);
        if (null == valueType)
        {
            handleError(node, "valueRef for enum name not found: " + valueRefType);
        }

        if (valueType instanceof EnumType)
        {
            final EnumType enumType = (EnumType)valueType;
            final String validValueName = valueRef.substring(periodIndex + 1);

            if (null == enumType.getValidValue(validValueName))
            {
                handleError(node, "valueRef for validValue name not found: " + validValueName);
            }
        }
        else
        {
            handleError(node, "valueRef for is not of type enum: " + valueRefType);
        }
    }

    /**
     * Builder to make creation of {@link Field} easier.
     */
    static class Builder
    {
        private String name;
        private String description;
        private int id = INVALID_ID;
        private Type type;
        private int offset;
        private String semanticType;
        private Presence presence;
        private String refValue;
        private int blockLength;
        private CompositeType dimensionType;
        private boolean variableLength;
        private int sinceVersion = 0;
        private int deprecated = 0;
        private String epoch;
        private String timeUnit;

        Builder name(final String name)
        {
            this.name = name;
            return this;
        }

        Builder description(final String description)
        {
            this.description = description;
            return this;
        }

        Builder id(final int id)
        {
            this.id = id;
            return this;
        }

        Builder type(final Type type)
        {
            this.type = type;
            return this;
        }

        Builder offset(final int offset)
        {
            this.offset = offset;
            return this;
        }

        Builder semanticType(final String semanticType)
        {
            this.semanticType = semanticType;
            return this;
        }

        Builder presence(final Presence presence)
        {
            this.presence = presence;
            return this;
        }

        Builder valueRef(final String refValue)
        {
            this.refValue = refValue;
            return this;
        }

        Builder blockLength(final int blockLength)
        {
            this.blockLength = blockLength;
            return this;
        }

        Builder dimensionType(final CompositeType dimensionType)
        {
            this.dimensionType = dimensionType;
            return this;
        }

        Builder variableLength(final boolean variableLength)
        {
            this.variableLength = variableLength;
            return this;
        }

        Builder sinceVersion(final int sinceVersion)
        {
            this.sinceVersion = sinceVersion;
            return this;
        }

        Builder deprecated(final int deprecated)
        {
            this.deprecated = deprecated;
            return this;
        }

        Builder epoch(final String epoch)
        {
            this.epoch = epoch;
            return this;
        }

        Builder timeUnit(final String timeUnit)
        {
            this.timeUnit = timeUnit;
            return this;
        }

        Field build()
        {
            return new Field(
                name,
                description,
                id,
                type,
                offset,
                semanticType,
                presence,
                refValue,
                blockLength,
                dimensionType,
                variableLength,
                sinceVersion,
                deprecated,
                epoch,
                timeUnit);
        }
    }
}
