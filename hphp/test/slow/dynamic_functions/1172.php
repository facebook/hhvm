<?hh
<<__DynamicallyCallable>>
function test($a) :mixed{
  print $a;
}

 <<__EntryPoint>>
function main_1172() :mixed{
  $a = 'test';
  $a('ok');
}
