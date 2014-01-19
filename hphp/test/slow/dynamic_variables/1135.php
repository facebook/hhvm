<?php

$a = 1;
 function t() {
 global $a;
$b = 'a';
 var_dump($$b);
}
 t();
