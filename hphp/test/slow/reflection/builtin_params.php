<?php

function main() {
  foreach ((new ReflectionFunction('date'))->getParameters() as $param) {
    var_dump($param->isDefaultValueAvailable());
    var_dump($param->isOptional());
  }
}
main();
