<?hh

function hello_my_name_is_mwang($x) :mixed{
  bar(hello_my_name_is_mwang($x));
}

function bar($x) :mixed{
  var_dump(__METHOD__);
}

function main() :mixed{
  $arr = vec[1, 'foo', vec[bar<>, 3], false];
  array_map(hello_my_name_is_mwang<>, $arr);
}


<<__EntryPoint>>
function main_fcall_helper_reentry() :mixed{
main();
}
