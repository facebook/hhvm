<?php

class Test implements IteratorAggregate {
    protected $data;

    public function __construct(array $data) {
        $this->data = $data;
    }

    public function getData() {
        return $this->data;
    }

    public function &getIterator() {
        foreach ($this->data as $key => &$value) {
            yield $key => $value;
        }
    }
}

$test = new Test([1, 2, 3, 4, 5]);
foreach ($test as &$value) {
    $value *= -1;
}

var_dump($test->getData());

?>