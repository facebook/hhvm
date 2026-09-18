<?hh
<<file:__EnableUnstableFeatures('named_parameters')>>
//
function apply_named<TInput, TOutput>(
  named (function(TInput)[_]: TOutput) $cb,
  named TInput $x,
)[ctx $cb]: TOutput {
  return $cb($x);
}

function apply_positional<TInput, TOutput>(
  (function(TInput)[_]: TOutput) $cb,
  TInput $x,
)[ctx $cb]: TOutput {
  return $cb($x);
}

function test(): void {
  apply_positional($x ==> $x['value'], shape('value' => 42));
  apply_named(x=shape('value' => 42), cb=$x ==> $x['value']);

  apply_named(cb=$x ==> $x['value'], x=shape('value' => 42));

  apply_named(
    x=shape('value' => 42),
    cb=function($x) { return $x['value']; },
  );

  apply_named(
    cb=function($x) { return $x['value']; },
    x=shape('value' => 42),
  );

  apply_named(
    cb=(shape('value' => int) $x) ==> $x['value'],
    x=shape('value' => 42),
  );
  apply_named<shape('value' => int), int>(
    cb=$x ==> $x['value'],
    x=shape('value' => 42),
  );

  apply_named(
    cb=($x): int ==> $x['value'],
    x=shape('value' => 42),
  );
}
