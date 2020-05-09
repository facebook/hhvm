<?hh
error_reporting(E_ALL);

abstract class Base {
   public abstract function sayHello(varray $a);
}

class SubClass extends Base {
   public function sayHello(varray $a) {
     echo "World!\n";
   }
}

$s = new SubClass();
$s->sayHello(varray[]);


trait SayWorld {
   public function sayHello(Base $d) {
     echo 'World!';
   }
}

class MyHelloWorld extends Base {
   use SayWorld;
}

$o = new MyHelloWorld();
$o->sayHello(varray[]);

