<?hh

interface X {
  public function foo(this $x);
}

class Y implements X {
  public function foo(X $x) { var_dump($x); }
}

class Z extends Y {
  public function foo(X $x) { var_dump($x); }
}

class R implements X {
  public function foo(this $x) { var_dump($x); }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
