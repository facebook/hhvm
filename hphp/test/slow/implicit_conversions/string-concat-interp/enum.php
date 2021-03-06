<?hh

enum myEnum: int {
  ONE = 1;
  TWO = 2;
  THREE = 3;
}

function assert(?string $in): myEnum {
  return myEnum::assert($in);
}


<<__EntryPoint>>
function main(): void {
  try {
    assert(null);
    echo "no assert!\n";
  } catch(Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
