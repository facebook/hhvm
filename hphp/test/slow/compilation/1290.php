<?hh

function g() :mixed{
}
function test1() :mixed{
  return '' . g();
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
