<?hh

function error_handler($errno, $errstr) :mixed{
  echo "Error: " . $errstr . " (" . $errno . ")\n";
}

<<__EntryPoint>>
function main() :mixed{
  set_error_handler(error_handler<>);
  var_dump(bcpow("2", "100.1"));
  var_dump(bcpow("2", "10000000000"));
  var_dump(bcpowmod("2.1", "3", "100"));
  var_dump(bcpowmod("2", "3.1", "100"));
  var_dump(bcpowmod("2", "3", "100.1"));
}
