<?php
/**
 * Auto generated from test.proto at 2014-11-19 14:12:00
 */

/**
 * Foo message
 */
class Foo extends \ProtobufMessage
{
    /* Field index constants */
    const DOUBLE_FIELD = 1;
    const FLOAT_FIELD = 2;
    const INT32_FIELD = 3;
    const INT64_FIELD = 4;
    const UINT32_FIELD = 5;
    const UINT64_FIELD = 6;
    const SINT32_FIELD = 7;
    const SINT64_FIELD = 8;
    const FIXED32_FIELD = 9;
    const FIXED64_FIELD = 10;
    const SFIXED32_FIELD = 11;
    const SFIXED64_FIELD = 12;
    const BOOL_FIELD = 13;
    const STRING_FIELD = 14;
    const BYTES_FIELD = 15;
    const REPEATED_FIELD = 16;
    const EMBEDDED_FIELD = 17;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::DOUBLE_FIELD => array(
            'name' => 'double_field',
            'required' => false,
            'type' => 1,
        ),
        self::FLOAT_FIELD => array(
            'name' => 'float_field',
            'required' => false,
            'type' => 4,
        ),
        self::INT32_FIELD => array(
            'name' => 'int32_field',
            'required' => false,
            'type' => 5,
        ),
        self::INT64_FIELD => array(
            'name' => 'int64_field',
            'required' => false,
            'type' => 5,
        ),
        self::UINT32_FIELD => array(
            'name' => 'uint32_field',
            'required' => false,
            'type' => 5,
        ),
        self::UINT64_FIELD => array(
            'name' => 'uint64_field',
            'required' => false,
            'type' => 9,
        ),
        self::SINT32_FIELD => array(
            'name' => 'sint32_field',
            'required' => false,
            'type' => 6,
        ),
        self::SINT64_FIELD => array(
            'name' => 'sint64_field',
            'required' => false,
            'type' => 6,
        ),
        self::FIXED32_FIELD => array(
            'name' => 'fixed32_field',
            'required' => false,
            'type' => 2,
        ),
        self::FIXED64_FIELD => array(
            'name' => 'fixed64_field',
            'required' => false,
            'type' => 3,
        ),
        self::SFIXED32_FIELD => array(
            'name' => 'sfixed32_field',
            'required' => false,
            'type' => 2,
        ),
        self::SFIXED64_FIELD => array(
            'name' => 'sfixed64_field',
            'required' => false,
            'type' => 3,
        ),
        self::BOOL_FIELD => array(
            'name' => 'bool_field',
            'required' => false,
            'type' => 8,
        ),
        self::STRING_FIELD => array(
            'name' => 'string_field',
            'required' => false,
            'type' => 7,
        ),
        self::BYTES_FIELD => array(
            'name' => 'bytes_field',
            'required' => false,
            'type' => 7,
        ),
        self::REPEATED_FIELD => array(
            'name' => 'repeated_field',
            'repeated' => true,
            'type' => 5,
        ),
        self::EMBEDDED_FIELD => array(
            'name' => 'embedded_field',
            'required' => false,
            'type' => 'Foo'
        ),
    );

    /**
     * Constructs new message container and clears its internal state
     *
     * @return null
     */
    public function __construct()
    {
        $this->reset();
    }

    /**
     * Clears message values and sets default ones
     *
     * @return null
     */
    public function reset()
    {
        $this->values[self::DOUBLE_FIELD] = null;
        $this->values[self::FLOAT_FIELD] = null;
        $this->values[self::INT32_FIELD] = null;
        $this->values[self::INT64_FIELD] = null;
        $this->values[self::UINT32_FIELD] = null;
        $this->values[self::UINT64_FIELD] = null;
        $this->values[self::SINT32_FIELD] = null;
        $this->values[self::SINT64_FIELD] = null;
        $this->values[self::FIXED32_FIELD] = null;
        $this->values[self::FIXED64_FIELD] = null;
        $this->values[self::SFIXED32_FIELD] = null;
        $this->values[self::SFIXED64_FIELD] = null;
        $this->values[self::BOOL_FIELD] = null;
        $this->values[self::STRING_FIELD] = null;
        $this->values[self::BYTES_FIELD] = null;
        $this->values[self::REPEATED_FIELD] = array();
        $this->values[self::EMBEDDED_FIELD] = null;
    }

    /**
     * Returns field descriptors
     *
     * @return array
     */
    public function fields()
    {
        return self::$fields;
    }

    /**
     * Sets value of 'double_field' property
     *
     * @param float $value Property value
     *
     * @return null
     */
    public function setDoubleField($value)
    {
        return $this->set(self::DOUBLE_FIELD, $value);
    }

    /**
     * Returns value of 'double_field' property
     *
     * @return float
     */
    public function getDoubleField()
    {
        return $this->get(self::DOUBLE_FIELD);
    }

    /**
     * Sets value of 'float_field' property
     *
     * @param float $value Property value
     *
     * @return null
     */
    public function setFloatField($value)
    {
        return $this->set(self::FLOAT_FIELD, $value);
    }

    /**
     * Returns value of 'float_field' property
     *
     * @return float
     */
    public function getFloatField()
    {
        return $this->get(self::FLOAT_FIELD);
    }

    /**
     * Sets value of 'int32_field' property
     *
     * @param int $value Property value
     *
     * @return null
     */
    public function setInt32Field($value)
    {
        return $this->set(self::INT32_FIELD, $value);
    }

    /**
     * Returns value of 'int32_field' property
     *
     * @return int
     */
    public function getInt32Field()
    {
        return $this->get(self::INT32_FIELD);
    }

    /**
     * Sets value of 'int64_field' property
     *
     * @param int $value Property value
     *
     * @return null
     */
    public function setInt64Field($value)
    {
        return $this->set(self::INT64_FIELD, $value);
    }

    /**
     * Returns value of 'int64_field' property
     *
     * @return int
     */
    public function getInt64Field()
    {
        return $this->get(self::INT64_FIELD);
    }

    /**
     * Sets value of 'uint32_field' property
     *
     * @param int $value Property value
     *
     * @return null
     */
    public function setUint32Field($value)
    {
        return $this->set(self::UINT32_FIELD, $value);
    }

    /**
     * Returns value of 'uint32_field' property
     *
     * @return int
     */
    public function getUint32Field()
    {
        return $this->get(self::UINT32_FIELD);
    }

    /**
     * Sets value of 'uint64_field' property
     *
     * @param string $value Property value
     *
     * @return null
     */
    public function setUint64Field($value)
    {
        return $this->set(self::UINT64_FIELD, $value);
    }

    /**
     * Returns value of 'uint64_field' property
     *
     * @return string
     */
    public function getUint64Field()
    {
        return $this->get(self::UINT64_FIELD);
    }

    /**
     * Sets value of 'sint32_field' property
     *
     * @param int $value Property value
     *
     * @return null
     */
    public function setSint32Field($value)
    {
        return $this->set(self::SINT32_FIELD, $value);
    }

    /**
     * Returns value of 'sint32_field' property
     *
     * @return int
     */
    public function getSint32Field()
    {
        return $this->get(self::SINT32_FIELD);
    }

    /**
     * Sets value of 'sint64_field' property
     *
     * @param int $value Property value
     *
     * @return null
     */
    public function setSint64Field($value)
    {
        return $this->set(self::SINT64_FIELD, $value);
    }

    /**
     * Returns value of 'sint64_field' property
     *
     * @return int
     */
    public function getSint64Field()
    {
        return $this->get(self::SINT64_FIELD);
    }

    /**
     * Sets value of 'fixed32_field' property
     *
     * @param int $value Property value
     *
     * @return null
     */
    public function setFixed32Field($value)
    {
        return $this->set(self::FIXED32_FIELD, $value);
    }

    /**
     * Returns value of 'fixed32_field' property
     *
     * @return int
     */
    public function getFixed32Field()
    {
        return $this->get(self::FIXED32_FIELD);
    }

    /**
     * Sets value of 'fixed64_field' property
     *
     * @param int $value Property value
     *
     * @return null
     */
    public function setFixed64Field($value)
    {
        return $this->set(self::FIXED64_FIELD, $value);
    }

    /**
     * Returns value of 'fixed64_field' property
     *
     * @return int
     */
    public function getFixed64Field()
    {
        return $this->get(self::FIXED64_FIELD);
    }

    /**
     * Sets value of 'sfixed32_field' property
     *
     * @param int $value Property value
     *
     * @return null
     */
    public function setSfixed32Field($value)
    {
        return $this->set(self::SFIXED32_FIELD, $value);
    }

    /**
     * Returns value of 'sfixed32_field' property
     *
     * @return int
     */
    public function getSfixed32Field()
    {
        return $this->get(self::SFIXED32_FIELD);
    }

    /**
     * Sets value of 'sfixed64_field' property
     *
     * @param int $value Property value
     *
     * @return null
     */
    public function setSfixed64Field($value)
    {
        return $this->set(self::SFIXED64_FIELD, $value);
    }

    /**
     * Returns value of 'sfixed64_field' property
     *
     * @return int
     */
    public function getSfixed64Field()
    {
        return $this->get(self::SFIXED64_FIELD);
    }

    /**
     * Sets value of 'bool_field' property
     *
     * @param bool $value Property value
     *
     * @return null
     */
    public function setBoolField($value)
    {
        return $this->set(self::BOOL_FIELD, $value);
    }

    /**
     * Returns value of 'bool_field' property
     *
     * @return bool
     */
    public function getBoolField()
    {
        return $this->get(self::BOOL_FIELD);
    }

    /**
     * Sets value of 'string_field' property
     *
     * @param string $value Property value
     *
     * @return null
     */
    public function setStringField($value)
    {
        return $this->set(self::STRING_FIELD, $value);
    }

    /**
     * Returns value of 'string_field' property
     *
     * @return string
     */
    public function getStringField()
    {
        return $this->get(self::STRING_FIELD);
    }

    /**
     * Sets value of 'bytes_field' property
     *
     * @param string $value Property value
     *
     * @return null
     */
    public function setBytesField($value)
    {
        return $this->set(self::BYTES_FIELD, $value);
    }

    /**
     * Returns value of 'bytes_field' property
     *
     * @return string
     */
    public function getBytesField()
    {
        return $this->get(self::BYTES_FIELD);
    }

    /**
     * Appends value to 'repeated_field' list
     *
     * @param int $value Value to append
     *
     * @return null
     */
    public function appendRepeatedField($value)
    {
        return $this->append(self::REPEATED_FIELD, $value);
    }

    /**
     * Clears 'repeated_field' list
     *
     * @return null
     */
    public function clearRepeatedField()
    {
        return $this->clear(self::REPEATED_FIELD);
    }

    /**
     * Returns 'repeated_field' list
     *
     * @return int[]
     */
    public function getRepeatedField()
    {
        return $this->get(self::REPEATED_FIELD);
    }

    /**
     * Returns 'repeated_field' iterator
     *
     * @return ArrayIterator
     */
    public function getRepeatedFieldIterator()
    {
        return new \ArrayIterator($this->get(self::REPEATED_FIELD));
    }

    /**
     * Returns element from 'repeated_field' list at given offset
     *
     * @param int $offset Position in list
     *
     * @return int
     */
    public function getRepeatedFieldAt($offset)
    {
        return $this->get(self::REPEATED_FIELD, $offset);
    }

    /**
     * Returns count of 'repeated_field' list
     *
     * @return int
     */
    public function getRepeatedFieldCount()
    {
        return $this->count(self::REPEATED_FIELD);
    }

    /**
     * Sets value of 'embedded_field' property
     *
     * @param Foo $value Property value
     *
     * @return null
     */
    public function setEmbeddedField(Foo $value)
    {
        return $this->set(self::EMBEDDED_FIELD, $value);
    }

    /**
     * Returns value of 'embedded_field' property
     *
     * @return Foo
     */
    public function getEmbeddedField()
    {
        return $this->get(self::EMBEDDED_FIELD);
    }
}

/**
 * Bar message
 */
class Bar extends \ProtobufMessage
{
    /* Field index constants */
    const DOUBLE_FIELD = 1;

    /* @var array Field descriptors */
    protected static $fields = array(
        self::DOUBLE_FIELD => array(
            'name' => 'double_field',
            'required' => true,
            'type' => 1,
        ),
    );

    /**
     * Constructs new message container and clears its internal state
     *
     * @return null
     */
    public function __construct()
    {
        $this->reset();
    }

    /**
     * Clears message values and sets default ones
     *
     * @return null
     */
    public function reset()
    {
        $this->values[self::DOUBLE_FIELD] = null;
    }

    /**
     * Returns field descriptors
     *
     * @return array
     */
    public function fields()
    {
        return self::$fields;
    }

    /**
     * Sets value of 'double_field' property
     *
     * @param float $value Property value
     *
     * @return null
     */
    public function setDoubleField($value)
    {
        return $this->set(self::DOUBLE_FIELD, $value);
    }

    /**
     * Returns value of 'double_field' property
     *
     * @return float
     */
    public function getDoubleField()
    {
        return $this->get(self::DOUBLE_FIELD);
    }
}
