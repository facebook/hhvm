<?hh //strict

function normal(string $s): int {
  return 1;
}

<<__Pure>>
function rx(): void {
  $l = <<__NonRx>>() ==> {
    normal("text");
  };
  // Not OK - lambda is not reactive
  $l();
}
