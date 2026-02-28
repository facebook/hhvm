<?hh

function g() :mixed{
}
function test1() :mixed{
  return 0 + g();
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
