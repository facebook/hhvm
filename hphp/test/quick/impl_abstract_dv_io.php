<?php
abstract class A   { abstract public function o(int $a1 = 0, int $a2 = 2);   }
class B extends A  {          public function o(int $a1 = 0, object $a2 = null) {} }
