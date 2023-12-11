<?hh

function test() :mixed{
$a=vec[1,2,3];
 unset($a[0]);
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
