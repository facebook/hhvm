<?php
class C { public static $p = 'original'; }
class D extends C {	}
class E extends D {	}

echo "\nInherited static properties refer to the same value across classes:\n";
var_dump(C::$p, D::$p, E::$p);

echo "\nChanging one changes all the others:\n";
D::$p = 'changed.all';
var_dump(C::$p, D::$p, E::$p);

echo "\nBut because this is implemented using PHP references, the reference set can easily be split:\n";
$ref = 'changed.one';
D::$p =& $ref;
var_dump(C::$p, D::$p, E::$p);
?>
==Done==