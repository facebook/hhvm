<?php

trait T {
   public $x=1;
   function dump_vars() {
     var_dump(get_class_vars('C'));
   }
}
class C {
      private $y=2;
      use T;
}
$o = new C;
$o->dump_vars();
