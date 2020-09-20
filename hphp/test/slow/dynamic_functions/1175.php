<?hh

function t($a, $b = 'k') {
 print $a.$b;
}

 <<__EntryPoint>>
function main_1175() {
$a = 'T';
 $a('o');
 $a('o', 'p');
}
