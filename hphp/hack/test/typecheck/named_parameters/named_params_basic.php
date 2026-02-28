<?hh
<<file: __EnableUnstableFeatures('named_parameters', 'named_parameters_use')>>

function foo(int $a, named int $b, named int $c): void {}

function test_function_calls(): void {
  foo(b=1, c=1, 1);
  foo(c=1, 1, b=1);
  foo(1, c=1, b=1);

  // Error: Missing required named parameter 'c'
  foo(1, b=2);

  // Error: Missing required named parameter 'b'
  foo(1, c=3);

  // Error: Extra named parameter 'd' not defined in function
  foo(1, b=2, c=3, d=4);
}
