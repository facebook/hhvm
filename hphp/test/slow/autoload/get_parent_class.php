<?php


<<__EntryPoint>>
function main_get_parent_class() {
spl_autoload_register(function ($class) {
  echo "Called autoload for $class\n";
  include __DIR__.'/autoload-class.inc';
});

var_dump(get_parent_class('C'));
}
