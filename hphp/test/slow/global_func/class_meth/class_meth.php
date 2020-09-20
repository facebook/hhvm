<?hh

trait P {
  static public function trait_meth() {
    return "trait_meth";
  }
}

class C {
  use P;
  static private function private_meth($s) {
    echo "private: $s\n";
  }
  static public function caller($f, $a) {
    $f($a);
  }
  static public function getCallable() {
    return class_meth(self::class, 'private_meth');
  }
  static public function mth() {
    return 1;
  }
}
<<__EntryPoint>> function main(): void {
$m = class_meth(C::class, 'mth');
var_dump($m());

$pub = class_meth(C::class, 'getCallable');
C::caller($pub(), 'created in C');

$tr = class_meth(C::class, 'trait_meth');
var_dump($tr());

$pri = class_meth(C::class, 'private_meth');
C::caller($pri, 'created outside');
}
