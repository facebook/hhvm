<?hh

class two {
  public static function foo() :mixed{
    echo "two\n";
    static::heh();
  }
}

class three extends two {
  public static function heh() :mixed{}
}

class doer {
  public function junk($x) :mixed{
    $x::foo();
  }
}

<<__EntryPoint>> function main(): void {
  $d = new two;
  $x = new doer;
  $x->junk($d);
}
