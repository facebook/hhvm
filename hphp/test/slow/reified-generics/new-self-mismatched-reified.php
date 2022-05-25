<?hh

trait T {
  function f() { new self(); }
}

class C<reify T> {
  use T;
}


<<__EntryPoint>>
function main() {
  try {
    (new C)->f();
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}
