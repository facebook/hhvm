<?php

trait T1 {
 function m1() {
 }
 function m2() {
 }
 }

class C1 {
 }
class C2 {
 use T1;
 }
class C3 {
 use T1 {
 m1 as a1;
 }
 }
class C4 {
 use T1 {
 m1 as a1;
 m2 as a2;
 }
 }

for ($c = "C1";
 $c <= "C4";
 $c++) {
    echo "class $c:\n";
    $r = new ReflectionClass($c);
    var_dump($r->getTraitAliases());
    echo "\n";
}
?>
