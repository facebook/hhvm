<?php

class ParentClass
{
    private static $_OBJECTS;

    public static function Get()
    {
        self::$_OBJECTS[1] = new ChildClass();
        return self::$_OBJECTS[1];    
    }
}

class ChildClass extends ParentClass
{
    public $Manager;

    function __construct()
    {
        $this->Manager = $this;
    }

    public static function &GetCurrent()
    {
        return ChildClass::Get();
    }

    public static function &Get()
    {
        return parent::Get();
    }
}

$staff = ChildClass::GetCurrent();
?>