<?php

function main() {
  foreach ((new ReflectionFunction('date'))->getParameters() as $param) {
    var_dump($param->isOptional());
    var_dump($param->isDefaultValueAvailable());
    try {
      var_dump($param->getDefaultValue());
    } catch (ReflectionException $ex) {
      print($ex->getMessage() . "\n");
    }
  }
  foreach ((new ReflectionFunction('exif_read_data'))->getParameters()
      as $param) {
    var_dump($param->isOptional());
    var_dump($param->isDefaultValueAvailable());
    if ($param->isDefaultValueAvailable()) {
      var_dump($param->getDefaultValue());
    }
  }
}
main();
