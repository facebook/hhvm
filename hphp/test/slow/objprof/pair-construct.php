<?hh

function foo() {
  return 1;
}

function main() {
  return Pair { foo(), 1 };
}


<<__EntryPoint>>
function main_pair_construct() {
fb_setprofile(() ==> objprof_get_data());
var_dump(main());
}
