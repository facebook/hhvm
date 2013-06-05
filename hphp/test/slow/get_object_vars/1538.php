<?php

class A {};
function main() {
  var_dump(get_object_vars(new A));
  var_dump(get_object_vars(false));
  var_dump(get_object_vars(true));
  var_dump(get_object_vars('hello'));
  var_dump(get_object_vars(5));
}
main();
