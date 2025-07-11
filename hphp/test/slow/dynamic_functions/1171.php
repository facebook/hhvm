<?hh
<<__DynamicallyCallable>>
function test() :mixed{
  print 'ok';
}

 <<__EntryPoint>>
function main_1171() :mixed{
  $a = 'test';
  HH\dynamic_fun($a)();
}
