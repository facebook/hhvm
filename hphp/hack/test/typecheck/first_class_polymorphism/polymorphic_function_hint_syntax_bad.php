<?hh

<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

// Error because variance annotations are not allowed on type parameters of polymorphic function hints
function bad1((function<-T>(T):void) $_ ): void {}

// Error because type parameters of polymorphic function hints cannot be reified
function bad2((function<reify T>(T):void) $_ ): void {}


class MyAttr implements HH\ClassAttribute {
  public function __construct(private string $param) {}
}

// Error because type parameters of polymorphic function hints cannot have attributes with parameters
function bad3((function<<<__MyAttr("nope")>> T>(T):void) $_ ): void {}

// Error because type parameters of polymorphic function hints cannot shadow class or method/function type parameters
function bad4<T>((function<T>(T):void) $_ ): void {}

class C<Tc> {
  public function bad5<Tm>((function<Tm,Tc>(Tm,Tc):void) $_ ): void {}
}

function bad6(
  (function<T>(_):T) $_,
  (function<T>(T):_) $_,
  (function<_>(_):_) $_,
): void {
}
