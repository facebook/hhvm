<?hh

abstract class Box {
  abstract const type T;
  abstract public function get(): this::T;
  abstract public function set(this::T $x): void;
};

function f(): void {
  $a = 42;
  $v = Vector{};
  $w = "";
  $f = (Box $x) ==> {
    $v[] = $x->get();
    $x->set($v[0]);
  };
}
