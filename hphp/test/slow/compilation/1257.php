<?hh

function str() :mixed{
 return 'test';
}
 function test() :mixed{
 var_dump(str() - $a);
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
