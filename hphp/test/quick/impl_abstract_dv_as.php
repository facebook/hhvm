<?php
abstract class A   { abstract public function s(int $a1 = 0, array  $a2 = null);   }
class B extends A  {          public function s(int $a1 = 0, string $a2 = null) {} }
