<?php

class F {
 function fruit() {
 yield 'apple';
 yield 'banana';
}
 }
foreach (F::fruit() as $fruit) {
 var_dump($fruit);
}

