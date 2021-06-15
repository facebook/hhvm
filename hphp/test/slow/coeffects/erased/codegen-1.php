<?hh

function f()[codegen]: void {
  g();
}
function g(): void {
  echo "call succeeded";
}

<<__EntryPoint>>
function main(): void {
  f();
}
