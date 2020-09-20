<?hh

function t($a, $b = 'k', $c = 'm') {
 print $a.$b.$c;
}

 <<__EntryPoint>>
function main_1176() {
$a = 'T';
 $a('o');
 $a('o', 'p');
 $a('o', 'p', 'q');
}
