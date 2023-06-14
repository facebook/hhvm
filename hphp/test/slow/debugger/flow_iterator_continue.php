<?hh

class A extends RecursiveDirectoryIterator {
  public function current() {
    return 'current() called';
  }
}

<<__EntryPoint>> function main(): void {

  $it = new RecursiveIteratorIterator(
    new A(__DIR__), RecursiveIteratorIterator::SELF_FIRST
  );

  foreach ($it as $a) {
    var_dump($a);
  }
}
