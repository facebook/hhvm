<?php
abstract class A   { abstract public function s(int $a1 = 0, int    $a2 = 2);   }
class B extends A  {          public function s(int $a1 = 0, string $a2 = "abc") {} }
