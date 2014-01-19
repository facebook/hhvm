<?php

function fruit() {
 yield 'apple';
 yield 'banana';
}
 foreach (fruit() as $fruit) {
 var_dump($fruit);
}

