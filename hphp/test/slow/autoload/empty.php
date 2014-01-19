<?php

spl_autoload_register(function($class) {
  var_dump($class);
});

var_dump(method_exists('', 'foo'));
