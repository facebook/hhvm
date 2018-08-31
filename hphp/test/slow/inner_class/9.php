<?php

class test {
 function p() {
 function inner() {
 print 'test';
}
 inner();
}
 }

 <<__EntryPoint>>
function main_9() {
$obj = new Test();
 $obj->p();
}
