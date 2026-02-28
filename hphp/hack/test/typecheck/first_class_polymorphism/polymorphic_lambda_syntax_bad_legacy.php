<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda','polymorphic_function_hints')>>

// Error because variance annotations are not allowed on type parameters of polymorphic lambdas
function bad1() : void {
  $_ = function<-T>(T $x):T use() { return $x;};
}

// Error because type parameters of polymorphic lambdas hints cannot be reified
function bad2(): void {
  $_ = function<reify T>(T $x): T use() { return $x;};
}


class MyAttr implements HH\ClassAttribute {
  public function __construct(private string $param) {}
}

// Error because type parameters of polymorphic lambdas cannot have attributes with arguments
function bad3(): void {
  $_ = function<<<__MyAttr("nope")>> T>(T $x):T use() { return $x;};
}

// Error because type parameters of polymorphic lambdas cannot shadow class or method/function type parameters
function bad4<T>(): void {
 $_ =  function<T>(T $x): T use() {return $x;};
}

class C<Tc> {
  public function bad5<Tm>(): void {
    $_ = function<Tm,Tc>(Tm $_ ,Tc $_):void use() {};
  }
}

// Errror because type parameter names cannot shadow preceding type parameter names
function bad6(): void {
  $_ = function<T,T>(T $_ ,T $_):void use() {};
}


// Error because type type parameters of polymorphic lambdas cannot shadow lambda type parameters in the outer scope
function bad7(): void {
  $_ = function<T>(T $x): (function<TInner>(TInner): void) use() {
    return function<T>(T $y):void use() {};
  };
}

// Error because we require type parameters to be specified explicitly in the signature of a polymorphic lambda
function bad8(): void {

  // Inferred return type
  $_ = function<T>(T $x) use() { return $x; };

  // Inferred parameter type
  $_ = function<T>($x): T use() { return $x;};
}

class Box<T> {}

// Error because we disallow wildcards in the signature of a polymorphic lambda
function bad9(): void {
  $_ = function<T>(_ $x): T use() { return $x;};

  $_ = function<T>(T $x): _  use() { return $x;};

  $_ = function<_>(_ $x): _ use() { return $x;};

  $_ = function<T>(Box<_> $x): Box<_> use() { return $x;};
}
