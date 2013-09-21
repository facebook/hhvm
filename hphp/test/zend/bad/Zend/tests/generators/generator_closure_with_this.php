<?php

class Test {
    public function getGenFactory() {
        return function() {
            yield $this;
        };
    }
}

$genFactory = (new Test)->getGenFactory();
var_dump($genFactory()->current());

?>