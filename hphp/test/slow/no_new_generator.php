<?hh

<<__EntryPoint>> function main(): void {
  try {
    var_dump(new Generator());
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
