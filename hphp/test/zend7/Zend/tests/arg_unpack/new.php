<?php

class Foo {
    public function __construct(...$args) {
        var_dump($args);
    }
}

new Foo(...[]);
new Foo(...[1, 2, 3]);
new Foo(...[1], ...[], ...[2, 3]);

?>
