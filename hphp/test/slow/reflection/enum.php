<?hh

enum TestEnum : int {
  FOO = 2;
  BAR = 1;
}

function reflection_funcs() {
  echo '= ', __FUNCTION__, ' =', "\n";
  var_dump(enum_exists(TestEnum::class));
  var_dump(class_exists(TestEnum::class)); // true, similar to 'abstract class'
  var_dump(interface_exists(TestEnum::class));
  var_dump(trait_exists(TestEnum::class));
  var_dump(get_class_constants(TestEnum::class));
}

function main() {
  reflection_funcs();
}

main();
