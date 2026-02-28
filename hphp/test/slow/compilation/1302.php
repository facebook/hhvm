<?hh

function foo() :mixed{
}
function test() :mixed{
  foo()->bar();
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
