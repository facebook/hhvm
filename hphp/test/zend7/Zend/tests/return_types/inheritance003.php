<?php

interface A {
    function foo(): A;
}

class B implements A {
    function foo(): StdClass {}
}

