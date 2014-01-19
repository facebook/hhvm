<?php
abstract class A   { abstract public function b(bool $b1 = null);   }
class B extends A  {          public function b(Boolean $b1 = null) {} }

