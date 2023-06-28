<?hh

<<__EntryPoint>>
function main() :mixed{
  try {
    // normal serialization format
    var_dump(unserialize('O:1:"X":1:{s:1:"p";i:0;}'));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
