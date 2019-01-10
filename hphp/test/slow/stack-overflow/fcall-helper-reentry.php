<?php

function hello_my_name_is_mwang($x) {
  bar(hello_my_name_is_mwang($x));
}

function bar($x) {
  var_dump(__METHOD__);
}

function main() {
  $arr = array(1, 'foo', array('bar', 3), false);
  array_map('hello_my_name_is_mwang', $arr);
}


<<__EntryPoint>>
function main_fcall_helper_reentry() {
main();
}
