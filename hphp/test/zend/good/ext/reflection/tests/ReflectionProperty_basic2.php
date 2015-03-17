<?php

function reflectProperty($class, $property) {
    $propInfo = new ReflectionProperty($class, $property);
    echo "**********************************\n";
    echo "Reflecting on property $class::$property\n\n";
    echo "isDefault():\n";
    var_dump($propInfo->isDefault());
    echo "getModifiers():\n";
    var_dump($propInfo->getModifiers());
    echo "getDeclaringClass():\n";
    var_dump($propInfo->getDeclaringClass());
    echo "getDocComment():\n";
    var_dump($propInfo->getDocComment());
    echo "\n**********************************\n";
}

class TestClass {
    public $pub;
    static public $stat = "static property";
    /**
     * This property has a comment.
     */
    protected $prot = 4;
    private $priv = "keepOut";
}

reflectProperty("TestClass", "pub");
reflectProperty("TestClass", "stat");
reflectProperty("TestClass", "prot");
reflectProperty("TestClass", "priv");

?> 
