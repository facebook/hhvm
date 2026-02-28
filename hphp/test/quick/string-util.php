<?hh


function strtobin($str) :mixed{
  $ret = '';
  $sep = '';
  for ($i = 0; $i < strlen($str); ++$i) {
    $char = $str[$i];
    $ret .= $sep . $char . ':' . ord($char);
    $sep = ', ';
  }
  return $ret;
}

function do_string($str) :mixed{
  var_dump(strtobin($str),
           strtobin(stripcslashes($str)));
}

<<__EntryPoint>> function main(): void {
  do_string("12345\\:2\\");
  do_string("12345\\:2\\4");
  do_string("12345\\:a");
  do_string("12345\\123");
  do_string("12345\\12345");
}
