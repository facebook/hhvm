<?php

class F {
 function fruit() {
 yield 'apple';
 yield 'banana';
}
 }


<<__EntryPoint>>
function main_2142() {
foreach (F::fruit() as $fruit) {
 var_dump($fruit);
}
}
