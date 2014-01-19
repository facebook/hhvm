<?php

class F {
 function fruit() {
 yield 'apple';
 yield 'banana';
}
 }
$f = new F;
 foreach ($f->fruit() as $fruit) {
 var_dump($fruit);
}

