<?hh

<<__EntryPoint>>
function main() {
  try {
    bcsub("123", "123", 4294967295);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
