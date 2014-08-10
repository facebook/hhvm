<?php
  class A
  {
      public static $p = "A::p (static)";
      static function showA()
      {
          echo self::$p . "\n";
      }
  }
  
  class B extends A
  {
      public $p = "B::p";
      function showB()
      {
          echo $this->p . "\n";
      }
  }
  
  
  A::showA();
  
  $b = new B;
  $b->showA();
  $b->showB();
?>
