<?hh

function g() {
}
function test1() {
  return 0 + g();
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
