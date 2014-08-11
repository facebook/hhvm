<?php

class C {
    public $a1;
    protected $a2;
    private $a3;
    static public $a4;
    static protected $a5;
    static private $a6;
}

class D extends C {
    public $a1;
    protected $a2;
    private $a3;
    static public $a4;
    static protected $a5;
    static private $a6;
}

for ($i = 1;$i <= 6;$i++) {
    $rp = new ReflectionProperty("C", "a$i");
    echo "C::a$i: ";
    var_dump($rp->getModifiers());
    $rp = new ReflectionProperty("D", "a$i");
    echo "D::a$i: ";
    var_dump($rp->getModifiers());
}

?>
