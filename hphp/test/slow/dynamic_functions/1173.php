<?hh
<<__DynamicallyCallable>>
function test($a, $b) {
  print $a.$b;
}

 <<__EntryPoint>>
function main_1173() {
  $a = 'test';
  $a('o','k');
}
