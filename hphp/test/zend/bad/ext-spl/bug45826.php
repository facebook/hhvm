<?php
$o = new ArrayObject();
$y = new StdClass;
$o->append($y);
$o->append($y);
$o->append($o);

var_dump($o[0] === $o[1]);
var_dump($o[2] === $o);

$s1 = serialize($o);
$s2 = $o->serialize();
var_dump($s1);
var_dump($s2);

$o1 =unserialize($s1);

var_dump($o1[0] === $o1[1]);
var_dump($o1[2] === $o1);

$o2 = new ArrayObject();
$o2->unserialize($s2);

var_dump($o2[0] === $o2[1]);
var_dump($o2[2] !== $o2);
var_dump($o2[2][2] === $o2[2]);

echo "#### Extending ArrayObject\n";
unset($o,$x,$s1,$s2,$o1,$o2);
class ArrayObject2 extends ArrayObject {
    public function serialize() {
        return parent::serialize();
    }

    public function unserialize($s) {
        return parent::unserialize($s);
    }
}

$o = new ArrayObject2();
$y = new StdClass;
$o->append($y);
$o->append($y);
$o->append($o);

var_dump($o[0] === $o[1]);
var_dump($o[2] === $o);

$s1 = serialize($o);
$s2 = $o->serialize();
var_dump($s1);
var_dump($s2);

$o1 =unserialize($s1);

var_dump($o1[0] === $o1[1]);
var_dump($o1[2] === $o1);

$o2 = new ArrayObject2();
$o2->unserialize($s2);

var_dump($o2[0] === $o2[1]);
var_dump($o2[2] !== $o2);
var_dump($o2[2][2] === $o2[2]);
?>