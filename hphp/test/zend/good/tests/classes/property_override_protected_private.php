<?php
  class A
  {
      protected $p = "A::p";
      function showA()
      {
          echo $this->p . "\n";
      }
  }
  
  class B extends A
  {
      private $p = "B::p";
      function showB()
      {
          echo $this->p . "\n";
      }
  }
  
  
  $a = new A;
  $a->showA();
  
  $b = new B;
  $b->showA();
  $b->showB();
?>