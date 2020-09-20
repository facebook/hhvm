<?hh

trait Hello {
   public function saySomething() {
     echo 'Hello';
   }
}

trait World {
   public function saySomething() {
     echo 'World';
   }
}
 
class MyHelloWorld {
   use Hello, World {
     Hello::saySomething insteadof World;
   }
}
<<__EntryPoint>>
function main_entry(): void {

  error_reporting(E_ALL);

  $o = new MyHelloWorld();
  $o->saySomething();
}
