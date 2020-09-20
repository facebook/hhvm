<?hh

trait Hello {
   public function sayHelloWorld() {
     echo 'Hello'.$this->getWorld();
   }
   abstract public function getWorld();
}

class MyHelloWorld {
   private $world;
   use Hello;
   public function getWorld() {
     return $this->world;
   }
   public function setWorld($val) {
     $this->world = $val;
   }
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$o = new MyHelloWorld();
$o->setWorld(' World!');
$o->sayHelloWorld();
}
