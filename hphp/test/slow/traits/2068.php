<?php

trait T {
 function fruit() {
 yield 'apple';
 yield 'banana';
}
 }
class F {
 use T;
 }
foreach (F::fruit() as $fruit) {
 var_dump($fruit);
}

