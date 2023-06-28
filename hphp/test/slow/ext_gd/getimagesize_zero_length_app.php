<?hh

<<__EntryPoint>> function main() :mixed{
  $x = null;
  var_dump(getimagesize(__DIR__."/getimagesize_zero_length_app.jpg", inout $x));
  foreach ($x as $k => $v) {
    echo "$k length " . strlen($v) . "\n";
  }
}
