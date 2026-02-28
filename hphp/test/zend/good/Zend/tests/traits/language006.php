<?hh

trait Hello {
   public function sayHelloWorld() :mixed{
     echo 'Hello'.$this->getWorld();
   }
   abstract public function getWorld():mixed;
}

class MyHelloWorld {
   private $world;
   use Hello;
   public function getWorld() :mixed{
     return $this->world;
   }
   public function setWorld($val) :mixed{
     $this->world = $val;
   }
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$o = new MyHelloWorld();
$o->setWorld(' World!');
$o->sayHelloWorld();
}
