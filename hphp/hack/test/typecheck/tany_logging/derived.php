<?hh

function f(): void {
  $tany1 = popen("","");
  if (1 === 2) {
    $tany2 = $tany1;
  } else {
    $tany2 = 42;
  }
  $tany3 = $tany2;
  $tany3;
}
