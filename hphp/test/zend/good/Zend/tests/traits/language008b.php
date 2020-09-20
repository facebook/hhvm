<?hh

trait HelloWorld {
   public function sayHello() {
     echo 'Hello World!';
   }
}

class MyClass {
   use HelloWorld { sayHello as private sayHelloWorld; }

   public function callPrivateAlias() {
      $this->sayHelloWorld();
   }
}


<<__EntryPoint>>
function entrypoint_language008b(): void {
  error_reporting(E_ALL);

  $o = new MyClass();
  $o->sayHello();
  $o->callPrivateAlias();
  $o->sayHelloWorld();
}
