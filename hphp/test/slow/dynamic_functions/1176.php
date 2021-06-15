<?hh
<<__DynamicallyCallable>>
function t($a, $b = 'k', $c = 'm') {
 print $a.$b.$c;
}

 <<__EntryPoint>>
function main_1176() {
$a = 't';
 $a('o');
 $a('o', 'p');
 $a('o', 'p', 'q');
}
