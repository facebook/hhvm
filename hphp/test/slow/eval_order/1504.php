<?php

function foo($i) {
   var_dump($i);
   return 'a';
 }


 <<__EntryPoint>>
function main_1504() {
${
foo(1)}
[foo(2)][foo(3)] = foo(4);
}
