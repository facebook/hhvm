<?hh

function test($s) {
  $a = darray['abc' => 1, 'abcd' => 2];
  $s .= 'c';
 var_dump($a[$s]);
  $s .= 'd';
 var_dump($a[$s]);
}

<<__EntryPoint>>
function main_170() {
test('ab');
}
