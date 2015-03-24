<?hh

function foo() {
  return 1;
}

function main() {
  return Pair { foo(), 1 };
}

fb_setprofile(() ==> objprof_get_data());
var_dump(main());
