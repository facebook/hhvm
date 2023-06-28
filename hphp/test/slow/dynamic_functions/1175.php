<?hh
<<__DynamicallyCallable>>
function t($a, $b = 'k') :mixed{
 print $a.$b;
}

 <<__EntryPoint>>
function main_1175() :mixed{
$a = 't';
 $a('o');
 $a('o', 'p');
}
