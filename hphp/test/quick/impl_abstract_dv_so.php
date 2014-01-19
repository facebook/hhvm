<?php
abstract class A   { abstract public function o(int $a1 = 0, string  $a2 = null);   }
class B extends A  {          public function o(int $a1 = 0, object  $a2 = null) {} }

