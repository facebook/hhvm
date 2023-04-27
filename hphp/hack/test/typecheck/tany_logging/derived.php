<?hh

function f(): HH\FIXME\TANY_MARKER<string> {
  return "a";
}

function g(): void {
  $tany1 = f();
  if (1 === 2) {
    $tany2 = $tany1;
  } else {
    $tany2 = 42;
  }
  $tany3 = $tany2;
  $tany3;
}
