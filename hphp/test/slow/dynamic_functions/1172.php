<?hh
<<__DynamicallyCallable>>
function test($a) {
  print $a;
}

 <<__EntryPoint>>
function main_1172() {
  $a = 'test';
  $a('ok');
}
