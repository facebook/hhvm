<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

<<__EntryPoint>>
function main(): void {
  // Basic closure with named args
  $closure = (named int $a, named int $b) ==> $a * 10 + $b;
  $result = \hphp_invoke_callable_named_args($closure, vec[], dict["a" => 3, "b" => 7]);
  var_dump($result);

  // Closure with positional and named args
  $closure2 = (int $x, named int $y) ==> $x * 100 + $y;
  $result2 = \hphp_invoke_callable_named_args($closure2, vec[5], dict["y" => 9]);
  var_dump($result2);

  // Closure with no named args
  $closure3 = (int $x) ==> $x + 1;
  $result3 = \hphp_invoke_callable_named_args($closure3, vec[41], null);
  var_dump($result3);
  // Second flavor of no named args
  $result4 = \hphp_invoke_callable_named_args($closure3, vec[41], dict[]);
  var_dump($result4);

  // Closure capturing a variable
  $multiplier = 7;
  $closure4 = (named int $val) ==> $val * $multiplier;
  $result5 = \hphp_invoke_callable_named_args($closure4, vec[], dict["val" => 6]);
  var_dump($result5);
}
