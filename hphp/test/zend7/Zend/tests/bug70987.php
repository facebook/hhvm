<?php

class foo {}
$bar = function () {
   return static::class;
};

var_dump($bar->call(new foo));

?>
