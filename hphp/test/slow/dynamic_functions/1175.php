<?hh
<<__DynamicallyCallable>>
function t($a, $b = 'k') {
 print $a.$b;
}

 <<__EntryPoint>>
function main_1175() {
$a = 't';
 $a('o');
 $a('o', 'p');
}
