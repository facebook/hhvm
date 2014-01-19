<?php

function f1(int $i = 1) {
 var_dump($i);
 }
function f2(double $d = 5.5) {
 var_dump($d);
 }
function f3(bool $b = true) {
 var_dump($b);
 }
function f4(string $s = 'hello') {
 var_dump($s);
 }
f1();
 f2();
 f3();
 f4();
