<?hh

trait Hello {
   public function saySomething() {
     echo 'Hello';
   }
}

trait World {
   public function saySomething() {
     echo ' World';
   }
}
 
class MyHelloWorld {
   use Hello, World {
     Hello::saySomething insteadof World;
	 World::saySomething as sayWorld;
   }
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(E_ALL);

  $o = new MyHelloWorld();
  $o->saySomething();
  $o->sayWorld();
}
