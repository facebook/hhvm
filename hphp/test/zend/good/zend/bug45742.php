<?php
class Constants {
    // Needs to be equal
    const A = 1;
    const B = 1;
}

class ArrayProperty {
    public static $array = array(
        Constants::A => 23,
        Constants::B => 42,
    );
}

var_dump( ArrayProperty::$array );
?>