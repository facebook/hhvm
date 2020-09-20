<?hh

class C {
  public function __toString() {
    echo "__toString called\n";
    return "string";
  }
}

class Box {
  public function __construct(public $p) {}
}

<<__EntryPoint>>
function test() {
  echo "==== arrays ====\n";
  var_dump(vec[new C()] <= vec["a"]);
  var_dump(vec[new C()] >= vec["a"]);
  var_dump(vec["a"] <= vec[new C()]);
  var_dump(vec["a"] >= vec[new C()]);

  echo "==== objects ====\n";
  var_dump(new Box(new C()) <= new Box("a"));
  var_dump(new Box(new C()) >= new Box("a"));
  var_dump(new Box("a") <= new Box(new C()));
  var_dump(new Box("a") >= new Box(new C()));
}
