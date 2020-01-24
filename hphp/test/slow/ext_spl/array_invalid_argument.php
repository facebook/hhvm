<?hh


<<__EntryPoint>>
function main_array_invalid_argument() {
try {
  $arrit = new ArrayIterator(42);
} catch(InvalidArgumentException $e) {
  echo "ArrIt::ctor: " . $e->getMessage() . "\n";
}
}
