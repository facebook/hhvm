<?hh

<<__EntryPoint>>
function main(): void {
  try {
    new __PHP_Incomplete_Class();
    echo "FAIL\n";
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
