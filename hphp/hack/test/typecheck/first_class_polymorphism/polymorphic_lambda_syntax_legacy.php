<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

class One {}
class Two extends One {}
class Three extends Two {}

interface I<T> {
  public function blah(): T;
}


function doIt(): void {
  $_ = function<T>(T $x): T use() { return $x; };
  $_ = function<T>(T $x): T { return $x; };

  //  Allow bounds on type parameters
  $_ = function<T as One super Three>(T $x): T use() { return $x;};

  // Allow attributes _without_ arguments on type parameters
  $_ = function<<<__NoAutoBound>> T>(T $x): T use() { return $x;};

  // Support F-bounded polymorphism
  $_ = function<T as I<T>>(T $x): I<T> use() { return $x->blah(); };

}
