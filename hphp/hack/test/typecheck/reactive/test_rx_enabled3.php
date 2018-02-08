<?hh // strict

<<__Rx>>
function f(): void {
  if (HH\Rx\IS_ENABLED) {
    rx();
  }
  nonrx(); // should error
}

<<__Rx>>
function rx(): void {
}

function nonrx(): void {
}
