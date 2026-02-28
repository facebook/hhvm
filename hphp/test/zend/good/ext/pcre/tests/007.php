<?hh

function evil($x) :mixed{
  ZendGoodExtPcreTests007::$txt[3] = "\xFF";
  var_dump($x);
  return $x[0];
}

abstract final class ZendGoodExtPcreTests007 {
  public static $txt;
}

<<__EntryPoint>>
function main(): void {
  ZendGoodExtPcreTests007::$txt = "ola123";
  $count = -1;
  $error = null;
  $result = preg_replace_callback_with_error(
    '#.#u',
    evil<>,
    ZendGoodExtPcreTests007::$txt,
    -1,
    inout $count,
    inout $error,
  );
  var_dump($result);
  var_dump(ZendGoodExtPcreTests007::$txt);
  var_dump($error === null);

  $count = -1;
  $error = null;
  $result = preg_replace_callback_with_error(
    '#.#u',
    evil<>,
    ZendGoodExtPcreTests007::$txt,
    -1,
    inout $count,
    inout $error,
  );
  var_dump($result);
  var_dump($error === PREG_BAD_UTF8_ERROR);

  echo "Done!\n";
}
