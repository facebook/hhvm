<?php

function test1() {
 $a = array(__FUNCTION__, __LINE__);
 return $a;
 }
function test2() {
 $a = array(__FUNCTION__, __LINE__);
 return $a;
 }
var_dump(test1());
 var_dump(test2());
