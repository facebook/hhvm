<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

class One {}
class Two extends One {}
class Three extends Two {}

interface I<T> {
  public function blah(): T;
}


function doIt(): void {
  // Allow single expression and block syntax
  $_ = function<T>(T $x): T ==> { return $x; };
  $_ = function<T>(T $x): T ==> $x;

  //  Allow bounds on type parameters
  $_ = function<T as One super Three>(T $x): T ==> $x;

  // Allow attributes _without_ arguments on type parameters
  $_ = function<<<__NoAutoBound>> T>(T $x): T ==> $x;

  // Support F-bounded polymorphism
  $_ = function<T as I<T>>(T $x): I<T> ==> { return $x->blah(); };

}
