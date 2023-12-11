<?hh

abstract class Base {
   public abstract function sayHello(varray $a):mixed;
}

class SubClass extends Base {
   public function sayHello(varray $a) :mixed{
     echo "World!\n";
   }
}


trait SayWorld {
   public function sayHello(Base $d) :mixed{
     echo 'World!';
   }
}

class MyHelloWorld extends Base {
   use SayWorld;
}

<<__EntryPoint>>
function entrypoint_inheritance003(): void {
  error_reporting(E_ALL);

  $s = new SubClass();
  $s->sayHello(vec[]);

  $o = new MyHelloWorld();
  $o->sayHello(vec[]);
}
