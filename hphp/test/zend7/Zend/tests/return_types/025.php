<?php

$c = function(): self { return $this; };
class Bar { }
var_dump($c->call(new Bar));

