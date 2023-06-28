<?hh

class MyDisposable implements IDisposable {
  public function __dispose(): void {
    echo "disposing\n";
  }
}

function foo(): void {
  using new MyDisposable();
  // UNSAFE BLOCK
  echo "in unsafe block\n";
}


<<__EntryPoint>>
function main_unsafe_block_in_using_block() :mixed{
foo();
}
