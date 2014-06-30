<?php

try {
  $arrit = new ArrayIterator(42);
} catch(InvalidArgumentException $e) {
  echo "ArrIt::ctor: " . $e->getMessage() . "\n";
}

try {
  $arrobj = new ArrayObject(42);
} catch (InvalidArgumentException $e) {
  echo "ArrObj::ctor: " . $e->getMessage() . "\n";
}

try {
  $arrobj = new ArrayObject(array(42));
  $arrobj->exchangeArray(42);
} catch (InvalidArgumentException $e) {
  echo "ArrObj::exchangeArray: " . $e->getMessage() . "\n";
}
