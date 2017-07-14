<?php

class A {
    function foo(): A {}
}

class B extends A {
    function foo(): StdClass {}
}

