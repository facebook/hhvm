<?hh
<<__DynamicallyCallable>>
function t($a, $b = 'k', $c = 'm') :mixed{
 print $a.$b.$c;
}

 <<__EntryPoint>>
function main_1176() :mixed{
$a = 't';
 $a('o');
 $a('o', 'p');
 $a('o', 'p', 'q');
}
