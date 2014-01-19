<?php
abstract class A   { abstract public function i(int $a1 = 0, string $a2 = null);   }
class B extends A  {          public function i(int $a1 = 0, int    $a2 = null) {} }

