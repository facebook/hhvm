<?php
interface Collection extends IteratorAggregate {
    function getIterator(): Iterator;
}

class SomeCollection implements Collection {
    function getIterator(): Generator {
        foreach ($this->data as $key => $value) {
            yield $key => $value;
        }
    }
}

$some = new SomeCollection();
var_dump($some->getIterator());

