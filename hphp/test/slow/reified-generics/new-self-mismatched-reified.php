<?hh

trait T {
  function f() :mixed{ new self(); }
}

class C<reify T> {
  use T;
}


<<__EntryPoint>>
function main() :mixed{
  try {
    (new C)->f();
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}
