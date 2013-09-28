<?php

function foo() {
 global $g;
 return $g ? -1 : 15;
}
 var_dump(TEST);
 define('TEST', foo());
 var_dump(TEST);
