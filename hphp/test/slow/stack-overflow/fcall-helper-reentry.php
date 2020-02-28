<?hh

function hello_my_name_is_mwang($x) {
  bar(hello_my_name_is_mwang($x));
}

function bar($x) {
  var_dump(__METHOD__);
}

function main() {
  $arr = varray[1, 'foo', varray['bar', 3], false];
  array_map(fun('hello_my_name_is_mwang'), $arr);
}


<<__EntryPoint>>
function main_fcall_helper_reentry() {
main();
}
