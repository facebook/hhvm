<?hh //strict

function normal(string $s): int {
  return 1;
}

<<__Rx>>
function rx(): void {
  $l = () ==> {
    normal("text");
  };
  // Not OK - lambda is not reactive
  $l();
}
