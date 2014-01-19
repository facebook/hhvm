<?php
  class A
  {
      public $p = "A::p";
      function showA()
      {
          echo $this->p . "\n";
      }
  }
  
  class B extends A
  {
      protected static $p = "B::p (static)";
      static function showB()
      {
          echo self::$p . "\n";
      }
  }
  
  
  $a = new A;
  $a->showA();
  
  $b = new B;
  $b->showA();
  B::showB();
?>