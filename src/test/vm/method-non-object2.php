<?php
// Method call on non-object using FPushObjMethod
$foo = 12;
$func = "bar";
$foo->$func();
echo "Hi\n";
