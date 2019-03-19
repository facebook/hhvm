<?php
class first {
  public static $a, $b;

   function me() { echo "first"; }

   function who() { 

     $this->me();
     self::$a->me();
     self::$b->me();
     self::$b = new second();
     $this->me();
     self::$a->me();
     self::$b->me();
   }
}

class second {

   function who() { 

      $this->me();
      first::$a->me();
      first::$b->me();
   }
   function me() { echo "second"; }
}

first::$a = new first();
first::$b = &first::$a;

first::$a->who();
first::$b->who();

echo "\n";
?>
===DONE===
