<?php

class Test {
    public function getGenFactory() {
        return function() {
            yield $this;
        };
    }
}

$genFactory = (new Test)->getGenFactory();
$gen = $genFactory();
$gen->next();
var_dump($gen->current());

