<?php
class Test
{
   const FIRST = 1;
   const SECOND = 2;
   const THIRD = 3;

   protected static $array = [
       self::FIRST => 'first',
       'second',
       'third',
       4,
   ];

   public function __construct()
   {
       var_export(self::$array);
   }
}

$test = new Test();
?>

===DONE===
