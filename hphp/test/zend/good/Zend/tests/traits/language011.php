<?hh

trait Hello {
   public function sayHello() {
     echo 'Hello';
   }
}

trait World {
   public function sayHello() {
     echo ' World!';
   }
}


class MyClass {
   use Hello, World { sayHello as sayWorld; }
}

<<__EntryPoint>>
function entrypoint_language011(): void {
  error_reporting(E_ALL);

  $o = new MyClass();
  $o->sayHello();
  $o->sayWorld();
}
