<?hh
<<file: __EnableUnstableFeatures('polymorphic_lambda')>>

function letsGo(): void {
  $out = Vector{};

  $f = function<T>(T $x) : void  ==> { $out[0] = $x; };

  $f(5);

  hh_show($out);
}
