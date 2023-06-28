<?hh

function bug1($a, $b) :mixed{
foreach ($b[$a++ + $a++] as $x) {
 echo $x;
 }
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
