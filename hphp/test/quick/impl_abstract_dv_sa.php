<?php
abstract class A            { abstract public function s(string $s1 = null);   }
abstract class B extends A  {          public function s(array  $s1 = null) {} }

