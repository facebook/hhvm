<?hh

function bug1($a, $b) :mixed{
$t1 = $a; $a++; $t2 = $a; $a++;
foreach ($b[$t1 + $t2] as $x) {
 echo $x;
 }
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
