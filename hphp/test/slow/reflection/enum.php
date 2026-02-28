<?hh

enum TestEnum : int {
  FOO = 2;
  BAR = 1;
}

function reflection_class() :mixed{
  echo '= ', __FUNCTION__, ' =', "\n";
  $rc = new ReflectionClass(TestEnum::class);
  echo (string) $rc, "\n";
  var_dump($rc->isEnum());
  var_dump($rc->isInstantiable());
  var_dump($rc->isAbstract());
  var_dump($rc->isInterface());
  var_dump($rc->isTrait());
  var_dump($rc->isFinal());
  var_dump($rc->getConstants());
}

function reflection_funcs() :mixed{
  echo '= ', __FUNCTION__, ' =', "\n";
  var_dump(enum_exists(TestEnum::class));
  var_dump(class_exists(TestEnum::class)); // true, similar to 'abstract class'
  var_dump(interface_exists(TestEnum::class));
  var_dump(trait_exists(TestEnum::class));
  var_dump(get_class_constants(TestEnum::class));
}

<<__EntryPoint>> function main(): void {
  reflection_class();
  reflection_funcs();
}
