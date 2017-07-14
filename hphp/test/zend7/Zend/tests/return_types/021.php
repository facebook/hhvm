<?php
class Foo {
    public static function getInstance() : self {
        return new static();
    }
}

class Bar extends Foo {}

var_dump(Foo::getInstance());
var_dump(Bar::getInstance());

