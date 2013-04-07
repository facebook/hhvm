<?php
error_reporting(E_ALL);

trait A {
   public function smallTalk() {
     echo 'a';
   }
   public function bigTalk() {
     echo 'A';
   }
}

trait B {
   public function smallTalk() {
     echo 'b';
   }
   public function bigTalk() {
     echo 'B';
   }
}

class Talker {
   use A, B;
}

?>