<?php

class A {
    /**
     * My Doc Comment for $a
     *
     */
    public $a = 2, $b, $c = 1;
    /**
     * My Doc Comment for $d
     */
    var $d;
    /**Not a doc comment */
    private $e;
    /**
     * Doc comment for $f
     */
    static protected $f;
}

class B extends A {
    public $a = 2;
    /** A doc comment for $b */
    var $b, $c = 1;
    /** A doc comment for $e */
    var $e;
}

foreach(array('A', 'B') as $class) {
    $rc = new ReflectionClass($class);
    $rps = $rc->getProperties();
    foreach($rps as $rp) {
        echo "\n\n---> Doc comment for $class::$" . $rp->getName() . ":\n";
        var_dump($rp->getDocComment());
    }
}

?>
