<?php

function f(&$a) {
}
 class T {
}
 $a = new T();
 $a->b = 10;
 f($a->b);
 var_dump($a);
