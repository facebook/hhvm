<?php

abstract class A {
    abstract function foo(): A;
}

class B extends A {
    function foo(): StdClass {}
}

