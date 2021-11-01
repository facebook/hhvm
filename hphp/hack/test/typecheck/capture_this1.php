<?hh

function f(): void {
  $a = function() use ($this) {};
}
