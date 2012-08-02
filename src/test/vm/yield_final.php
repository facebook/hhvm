<?php
trait C { final function method1() { yield 1; } }
class A { use C; }
class B extends A { use C; }
