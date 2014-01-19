<?php
abstract class A   { abstract public function d(int $a1 = 0, int    $a2 = 2);     }
class B extends A  {          public function d(int $a1 = 0, double $a2 = 3.0) {} }

