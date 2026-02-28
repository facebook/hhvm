<?hh

class one {
  public function foo() :mixed{
    echo "one\n";
  }
}

class two {
  public static function foo() :mixed{
    echo "two\n";
    if (static::class === "three") {
      static::heh();
    }
  }
}

class three extends two {
  public static function heh() :mixed{ echo "three\n"; }
}

class doer {
  public function ijunk($x) :mixed{
    $x->foo();
  }
  public function sjunk($x) :mixed{
    $x::foo();
  }
}

<<__EntryPoint>> function main(): void {
  $b = new one;
  $d = new two;
  $x = new doer;
  $x->ijunk($b);
  $x->sjunk($d);
  $x->ijunk($b);
  $x->sjunk($d);
  $x->sjunk(new three);
  $x->ijunk($b);
  $x->sjunk(new three);
  $x->sjunk($d);
}
