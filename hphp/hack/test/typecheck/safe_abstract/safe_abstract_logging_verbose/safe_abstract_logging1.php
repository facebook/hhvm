<?hh

<<__ConsistentConstruct>>
interface I1 {
  public static function smeth(): void;
}

final class Concrete {
  public static function smeth(): void {}
  public function meth(): void {}
}

function new_unsafe(): void {
  if (1 < 3) {
    $class = I1::class;
  } else {
    $class = Concrete::class;
  }
  new $class();      // use_kind: new, is_safe: false
}

function new_safe(): void {
  $class2 = Concrete::class;
  Concrete::smeth(); // use_kind: new, is_safe: true
  $class2::smeth();  // use_kind: new, is_safe: true
}

function static_call_unsafe(): void {
  if (1 < 3) {
    $class = I1::class;
  } else {
    $class = Concrete::class;
  }
  $class::smeth(); //   use_kind: call, is_safe: false
}

function static_call_safe(): void {
  $class = Concrete::class;
  Concrete::smeth(); // use_kind: call, is_safe: true
  $class::smeth(); //   use_kind: call, is_safe: true
}

function should_not_log(): void {
  Concrete::class;
  $x = new Concrete();
  $x->meth();
}
