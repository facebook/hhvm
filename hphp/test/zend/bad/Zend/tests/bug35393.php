<?php
class ObjectPath
{
    static protected $type = array(0=>'main');

    static function getType()
    {
        return self::$type;
    }
}
print_r(ObjectPath::getType());
$object_type = array_pop((ObjectPath::getType()));
print_r(ObjectPath::getType());
?>