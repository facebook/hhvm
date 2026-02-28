<?hh

function main() :mixed{
  $a = 2;
  switch ($a) {
    case ++$a:
      echo "correct!\n";
      break;
    case 2:
      echo "nope: two\n";
      break;
    default:
      echo "nope: default\n";
      break;
  }

  $a = vec[2];
  switch ($a[0]) {
    case ++$a[0]:
      echo "nope: pre-inc\n";
      break;
    case 2:
      echo "correct!\n";
      break;
    default:
      echo "nope: default\n";
      break;
  }

  $a = vec[10];
  $ten = 10;
  switch ($a[0]) {
    case $ten:
      echo "correct!\n";
      break;
    default:
      echo "nope\n";
      break;
  }
}

function strswitch() :mixed{
  $a = 'luke';
  switch ($a) {
    case 'leia':
      echo "nope\n";
      break;
    case 'luke':
      echo "yep\n";
      break;
    default:
      echo "whoops\n";
      break;
  }

  $x = '123';
  switch (HH\Lib\Legacy_FIXME\string_cast_for_switch($x, '123.0', null, null, dict[
  '123.0' => 123,
  '123' => 123,
], dict[
])) {
    case '123.0':
      echo "right\n";
      break;

    case '123':
      echo "wrong\n";
      break;
  }

  switch($x) {
    case 'wheeeeeee':
      return -1;
    case 'whooooooo':
      return -2;
    default:
      echo "cool\n";
  }

  return true;
}
<<__EntryPoint>>
function main_entry(): void {
  main();
  main();
  var_dump(strswitch());
  var_dump(strswitch());
}
