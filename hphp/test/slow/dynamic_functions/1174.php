<?hh
<<__DynamicallyCallable>>
function t($a = 'k') :mixed{
 print $a;
}

 <<__EntryPoint>>
function main_1174() :mixed{
$a = 't';
 HH\dynamic_fun($a)();
 HH\dynamic_fun($a)('o');
}
