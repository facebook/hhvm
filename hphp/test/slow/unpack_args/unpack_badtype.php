<?hh
function handler() {
  throw new Exception;
}
function test($x) {}
function main($a) {
  try {
    test(...$a);
  } catch (Exception $e) {}
}

<<__EntryPoint>>
function main_unpack_badtype() {
set_error_handler(fun('handler'));
main(new stdclass);
echo "OK\n";
}
