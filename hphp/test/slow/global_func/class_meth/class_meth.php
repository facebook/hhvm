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
    return self::private_meth<>;
  }
  static public function mth() {
    return 1;
  }
}
<<__EntryPoint>> function main(): void {
$m = C::mth<>;
var_dump($m());

$pub = C::getCallable<>;
C::caller($pub(), 'created in C');

$tr = C::trait_meth<>;
var_dump($tr());

$pri = C::private_meth<>;
C::caller($pri, 'created outside');
}
