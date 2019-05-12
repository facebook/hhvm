<?hh

class two {
  public static function foo() {
    echo "two\n";
    static::heh();
  }
}

class three extends two {
  public static function heh() {}
}

class doer {
  public function junk($x) {
    $x::foo();
  }
}

<<__EntryPoint>> function main(): void {
  $d = new two;
  $x = new doer;
  $x->junk($d);
}
