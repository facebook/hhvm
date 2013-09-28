<?php

function f() {
 global $a;
 return ++$a;
 }
var_dump(array($a,f(),$a));
