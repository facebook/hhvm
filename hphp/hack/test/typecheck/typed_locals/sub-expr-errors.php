<?hh
<<file: __EnableUnstableFeatures('typed_local_variables')>>

async function f(bool $b): Awaitable<bool> {

  ($a ==> { let $x:int; let $x:int; return false; })($b);

  throw ($a ==> { let $x:int; let $x:int; return new Exception(); })($b);

  concurrent {
    $x1 = await (async $a ==> { let $x:int; let $x:int; return false; })($b);
    $x2 = await (async $a ==> { let $x:int; let $x:int; return false; })($b);
  }

  if (
    ($a ==> { let $x:int; let $x:int; return false; })($b)
  ) { }

  do {} while (($a ==> { let $x:int; let $x:int; return false; })($b));

  while (($a ==> { let $x:int; let $x:int; return false; })($b)) {};

  for ($v = ($a ==> { let $x:int; let $x:int; return false; })($b); ($a ==> { let $x:int; let $x:int; return false; })($b); $v = ($a ==> { let $x:int; let $x:int; return false; })($b)) {}

  switch (($a ==> { let $x:int; let $x:int; return false; })($b)) { default: break; }

  foreach (($a ==> { let $x:int; let $x:int; return vec[]; })($b) as $v) {}

  let $y:int = ($a ==> { let $x:int; let $x:int; return 1; })($b);

  return ($a ==> { let $x:int; let $x:int; return false; })($b);
}
