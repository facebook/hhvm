<?php

try {
  SplFixedArray::fromArray(['string'=>'string']);
} catch (InvalidArgumentException $e) {
  echo $e->getMessage();
  echo "\n";
}
