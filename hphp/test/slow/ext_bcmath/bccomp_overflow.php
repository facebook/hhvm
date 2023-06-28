<?hh


<<__EntryPoint>>
function main_bccomp_overflow() :mixed{
$intMaxPre = PHP_INT_MAX - 1;
$stringNormal = 'abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ';

$number_bccomp_10 = bccomp("2015.5", $stringNormal, $intMaxPre);
var_dump($number_bccomp_10);
}
