<?hh

function foo() :mixed{
  return 1;
}

function main() :mixed{
  return Pair { foo(), 1 };
}


<<__EntryPoint>>
function main_pair_construct() :mixed{
fb_setprofile(() ==> objprof_get_data());
var_dump(main());
}
