<?hh

trait Hello {
   public function hello() {
     echo 'Hello';
   }
}

trait World {
   public function world() {
     echo ' World!';
   }
}


class MyClass {
   use Hello, World { world as hello; }
}

<<__EntryPoint>>
function entrypoint_language014(): void {
  error_reporting(E_ALL);

  $o = new MyClass();
  $o->hello();
  $o->world();
}
