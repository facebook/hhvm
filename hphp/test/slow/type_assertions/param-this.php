<?hh

interface X {
  public function foo(this $x):mixed;
}

class Y implements X {
  public function foo(X $x) :mixed{ var_dump($x); }
}

class Z extends Y {
  public function foo(X $x) :mixed{ var_dump($x); }
}

class R implements X {
  public function foo(this $x) :mixed{ var_dump($x); }
}

<<__EntryPoint>> function main(): void { echo "Done.\n"; }
