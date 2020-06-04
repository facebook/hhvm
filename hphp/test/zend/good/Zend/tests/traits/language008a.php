<?hh

trait HelloWorld {
   public function sayHello() {
     echo 'Hello World!';
   }
}

class MyClass {
   use HelloWorld { sayHello as protected; }
}

<<__EntryPoint>>
function entrypoint_language008a(): void {
  error_reporting(E_ALL);


  $o = new MyClass;
  $o->sayHello();
}
