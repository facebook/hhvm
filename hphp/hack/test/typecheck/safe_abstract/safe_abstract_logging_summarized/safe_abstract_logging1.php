<?hh

<<__ConsistentConstruct>>
interface I1 {
  public static function smeth(): void;
}

final class Concrete {
  public static function smeth(): void {}
  public function meth(): void {}
}

// {"safe_news":0,"unsafe_news":1,"safe_calls":0,"unsafe_calls":0}
function new_unsafe(): void {
  if (1 < 3) {
    $class = I1::class;
  } else {
    $class = Concrete::class;
  }
  new $class();
}

// {"safe_news":0,"unsafe_news":0,"safe_calls":2,"unsafe_calls":0}
function new_safe(): void {
  $class2 = Concrete::class;
  Concrete::smeth();
  $class2::smeth();
}

// {"safe_news":0,"unsafe_news":0,"safe_calls":0,"unsafe_calls":1}
function static_call_unsafe(): void {
  if (1 < 3) {
    $class = I1::class;
  } else {
    $class = Concrete::class;
  }
  $class::smeth();
}

// {"safe_news":0,"unsafe_news":0,"safe_calls":2,"unsafe_calls":0}
function static_call_safe(): void {
  $class = Concrete::class;
  Concrete::smeth();
  $class::smeth();
}

function should_not_log(): void {
  Concrete::class;
  $x = new Concrete();
  $x->meth();
}
