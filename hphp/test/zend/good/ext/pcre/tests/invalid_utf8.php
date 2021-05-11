<?hh

<<__EntryPoint>>
function main(): void {
  $string = urldecode("search%e4");
  $error = null;
  $result = preg_replace_with_error(
    "#(&\#x*)([0-9A-F]+);*#iu",
    "$1$2;",
    $string,
    inout $error,
  );
  var_dump($result);
  var_dump($error);

  echo "Done\n";
}
