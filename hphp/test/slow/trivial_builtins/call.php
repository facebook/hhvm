<?hh

// The divergent implementations are intentional to illustrate
// which function is being called

<<\TrivialHHVMBuiltinWrapper('g')>>
function f(): void {
  echo "f\n";
}

function g(): void {
  echo "g\n";
}

<<__EntryPoint>>
function main(): void {
  f();
}
