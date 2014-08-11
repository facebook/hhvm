<?hh

function main() {
  $arr = miarray(
    0 => 'zero',
    5 => 'five',
    10 => 'ten',
    15 => 'fifteen',
    20 => 'twenty',
  );
  $res = shuffle($arr);
  var_dump($res);
  $arr[] = 'no warn';
}

main();
