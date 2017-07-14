<?php

class A {
    function foo(array $a): array {
        return $a;
    }
}

ReflectionClass::export("A");
