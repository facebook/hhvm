//// partial.php
<?hh

function any() {}

//// strict.php
<?hh // strict

function nullthrows<T>(?T $x): T {
  if ($x !== null) {
    return $x;
  }

  invariant_violation('lol');
}

async function f(): Awaitable<void> {
  $a = await any();
  $n = nullthrows($a);
  // That this works at all in strict mode is probably a bug, but just making
  // sure it doesn't give an empty position for now. (Until we remove the
  // restriction on calling methods on Tany in strict, of course.)
  $n->f();
}
