<?hh
// `this` in the return type (or covariantly within it) is never an obtainable
// *parameter* occurrence, so it never contributes. Only parameters are analysed.
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class A {
  // return `this`, no this-params: obtainable=0 -> OK
  public static function s_ret(int $_): this {
    throw new Exception();
  }
  // return vec<this>: obtainable=0 -> OK
  public static function s_ret_vec(int $_): vec<this> {
    return vec[];
  }
  // one obtainable param + return this: obtainable=1 -> OK (return ignored)
  public static function s_one_param_ret(this $_): this {
    throw new Exception();
  }

  // instance returning this, no this-params: receiver(1) -> OK
  public function i_ret(int $_): this {
    return $this;
  }
}

function test(): void {
  $ok1 = A::s_ret<>;
  $ok2 = A::s_ret_vec<>;
  $ok3 = A::s_one_param_ret<>;
  $ok4 = meth_caller(A::class, 'i_ret');
}
