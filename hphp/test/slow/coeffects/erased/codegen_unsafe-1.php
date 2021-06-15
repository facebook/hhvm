<?hh

function f()[codegen_unsafe]: void {
  g();
}
function g(): void {
  echo "call succeeded";
}

<<__EntryPoint>>
function main(): void {
  f();
}
