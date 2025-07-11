<?hh
<<__DynamicallyCallable>>
function t($a, $b = 'k') :mixed{
 print $a.$b;
}

 <<__EntryPoint>>
function main_1175() :mixed{
$a = 't';
 HH\dynamic_fun($a)('o');
 HH\dynamic_fun($a)('o', 'p');
}
