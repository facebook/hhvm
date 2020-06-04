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
   use Hello, World { hello as world; }
}

<<__EntryPoint>>
function entrypoint_language010(): void {
  error_reporting(E_ALL);

  $o = new MyClass();
  $o->hello();
  $o->world();
}
