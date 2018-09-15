<?php

function test() {
 $a = 10;
 $b = 'test';
   var_dump(compact('ab'));
   var_dump(compact('a', 'ab', 'b'));
   var_dump(compact('a', array('ab', 'b')));
}


 <<__EntryPoint>>
function main_1156() {
test();
}
