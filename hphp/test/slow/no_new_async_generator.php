<?hh

<<__EntryPoint>> function main(): void {
  try {
    var_dump(new AsyncGenerator());
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
