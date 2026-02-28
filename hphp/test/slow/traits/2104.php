<?hh

trait T {
   public $x=1;
   function dump_vars() :mixed{
     var_dump(get_class_vars('C'));
   }
}
class C {
      private $y=2;
      use T;
}

<<__EntryPoint>>
function main_2104() :mixed{
$o = new C;
$o->dump_vars();
}
