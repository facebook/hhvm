<?php

trait T1 {
}
trait T2 {
}
trait T3 {
 use T2;
 }
class C1 {
}
class C2 {
 use T1;
 }
class C3 {
 use T1, T2;
 }
class C4 {
 use T3;
 }
class C5 extends C4 {
}
interface I1 {
}
echo "T1:\n";
var_dump(class_uses('T1'));
$rt1 = new ReflectionClass('T1');
var_dump($rt1->getTraitNames());
echo "\nT3:\n";
var_dump(class_uses('T3'));
$rt3 = new ReflectionClass('T3');
var_dump($rt3->getTraitNames());
echo "\nC1:\n";
var_dump(class_uses('C1'));
$rc1 = new ReflectionClass('C1');
var_dump($rc1->getTraitNames());
echo "\nC2:\n";
var_dump(class_uses('C2'));
$rc2 = new ReflectionClass('C2');
var_dump($rc2->getTraitNames());
echo "\nC3:\n";
var_dump(class_uses('C3'));
$rc3 = new ReflectionClass('C3');
var_dump($rc3->getTraitNames());
echo "\nC4:\n";
var_dump(class_uses('C4'));
$rc4 = new ReflectionClass('C4');
var_dump($rc4->getTraitNames());
echo "\nC5:\n";
var_dump(class_uses('C5'));
$rc5 = new ReflectionClass('C5');
var_dump($rc5->getTraitNames());
echo "\nI1:\n";
var_dump(class_uses('I1'));
$ri1 = new ReflectionClass('I1');
var_dump($ri1->getTraitNames());
?>
