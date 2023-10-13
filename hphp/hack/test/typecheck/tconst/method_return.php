<?hh

abstract class X {
  abstract const type T as arraykey = arraykey;

  final public function __construct(private this::T $val) {}

  public function get(): this::T {
    return $this->val;
  }

  public function alias_this_get(): this::T {
    $this_for_hack = $this;
    return $this_for_hack->val;
  }

  public function alias_lvar_get(): this::T {
    $lvar = $this->val;
    return $lvar;
  }

  public function new_static_get(): this::T {
    return (new static($this->val))->val;
  }

  public static function static_get(): this::T {
    // UNSAFE
  }
}

class Y extends X {
  const type T = int;

  use X_Trait;

  public function get_int(): this::T {
    return 0;
  }

  public function parent_get(): this::T {
    return parent::get();
  }
}

trait X_Trait {
  require extends X;

  public function trait_get(): this::T {
    return $this->get();
  }

  public function parent_get(): this::T {
    return parent::get();
  }
}

function test(Y $y, X $x): (int, arraykey) {
  return tuple($y->get(), $x->get());
}

function test2(Y $y, X $x): (int, arraykey, X::T, int) {
  return
    tuple($y::static_get(), $x::static_get(), X::static_get(), Y::static_get());
}
