<?hh

class A extends RecursiveDirectoryIterator {
  public function current() {
    return 'current() called';
  }
}

function main() {

  $it = new RecursiveIteratorIterator(
    new A(__DIR__.'/../../sample_dir/'), RecursiveIteratorIterator::SELF_FIRST
  );

  foreach ($it as $a) {
    var_dump($a);
  }
}

<<__EntryPoint>>
function main_recursive_directory_iterator_keep_class() {
main();
}
