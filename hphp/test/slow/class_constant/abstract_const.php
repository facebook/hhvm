<?hh


interface I {
  abstract const X;
  const Y = 'I::Y';
  const Z = self::Y . ' via Z'; // value of Z isn't available to the pre-class
}

class C implements I {
  const X = 'C::X';
}

interface J extends I {
  const X = 'J::X';
}

function reflect() {
  echo "\n",'== ', __FUNCTION__, ' ==', "\n";
  var_dump(get_class_constants(I::class));
  $rc = new ReflectionClass(I::class);
  var_dump($rc->getConstants());
  var_dump($rc->getAbstractConstantNames());
  var_dump($rc->hasConstant('X'));

  var_dump(get_class_constants(C::class));
  $rc = new ReflectionClass(C::class);
  var_dump($rc->getConstants());
  var_dump($rc->getAbstractConstantNames());
  var_dump($rc->hasConstant('X'));

  var_dump(get_class_constants(J::class));
  $rc = new ReflectionClass(J::class);
  var_dump($rc->getConstants());
  var_dump($rc->getAbstractConstantNames());
  var_dump($rc->hasConstant('X'));
}

function main() {
  var_dump(C::X);
  var_dump(C::Y);
  var_dump(C::Z);

  var_dump(J::X);
  var_dump(J::Y);
  var_dump(J::Z);

  reflect();
}

main();
