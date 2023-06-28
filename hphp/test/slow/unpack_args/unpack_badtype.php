<?hh
function handler() :mixed{
  throw new Exception;
}
function test($x) :mixed{}
function main($a) :mixed{
  try {
    test(...$a);
  } catch (Exception $e) {}
}

<<__EntryPoint>>
function main_unpack_badtype() :mixed{
set_error_handler(handler<>);
main(new stdClass);
echo "OK\n";
}
