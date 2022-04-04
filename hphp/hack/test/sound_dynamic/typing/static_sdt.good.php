<?hh

<<__SupportDynamicType>>
class A<+T as supportdyn<mixed> > {
  public function __construct(private ~T $x) {}
}

<<__SupportDynamicType>>
function get(): ~C { return new C(); }

<<__SupportDynamicType>>
class C {
  public function gen(): A<~?string> {
    return new A("");
  }
}

<<__SupportDynamicType>>
class S {
  public static function remA<T as supportdyn<mixed> >(A<~T> $a): ~T {
    throw new Exception();
  }
}

<<__SupportDynamicType>>
function remA2<T as supportdyn<mixed> >(A<~T> $a): ~T { throw new Exception(); }

<<__SupportDynamicType>>
function getMessage(): void {
  $x = get();
  $y = $x->gen();
  remA2($y);
  S::remA($y);
}
