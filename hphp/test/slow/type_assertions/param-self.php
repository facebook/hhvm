<?hh

interface X {
  public function foo(self $x);
}

class Y implements X {
  public function foo(X $x) { var_dump($x); }
}

class Z extends Y {
  public function foo(X $x) { var_dump($x); }
}

class R implements X {
  public function foo(self $x) { var_dump($x); }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
