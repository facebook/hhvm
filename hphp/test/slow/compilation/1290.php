<?hh

function g() {
}
function test1() {
  return '' . g();
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
