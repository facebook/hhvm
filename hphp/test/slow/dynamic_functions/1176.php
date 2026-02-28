<?hh
<<__DynamicallyCallable>>
function t($a, $b = 'k', $c = 'm') :mixed{
 print $a.$b.$c;
}

 <<__EntryPoint>>
function main_1176() :mixed{
$a = 't';
 HH\dynamic_fun($a)('o');
 HH\dynamic_fun($a)('o', 'p');
 HH\dynamic_fun($a)('o', 'p', 'q');
}
