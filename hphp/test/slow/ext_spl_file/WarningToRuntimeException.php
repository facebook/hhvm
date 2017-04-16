<?php

$spl = new SplFileInfo('notexisting');

try {
  var_dump($spl->getPerms());
} catch (RuntimeException $e) {
  echo get_class($e) . ": " . $e->getMessage() . "\n";
}

try {
  var_dump($spl->getInode());
} catch (RuntimeException $e) {
  echo get_class($e) . ": " . $e->getMessage() . "\n";
}
try {
  var_dump($spl->getSize());
} catch (RuntimeException $e) {
  echo get_class($e) . ": " . $e->getMessage() . "\n";
}
try {
  var_dump($spl->getOwner());
} catch (RuntimeException $e) {
  echo get_class($e) . ": " . $e->getMessage() . "\n";
}
try {
  var_dump($spl->getGroup());
} catch (RuntimeException $e) {
  echo get_class($e) . ": " . $e->getMessage() . "\n";
}
try {
  var_dump($spl->getATime());
} catch (RuntimeException $e) {
  echo get_class($e) . ": " . $e->getMessage() . "\n";
}
try {
  var_dump($spl->getMTime());
} catch (RuntimeException $e) {
  echo get_class($e) . ": " . $e->getMessage() . "\n";
}
try {
  var_dump($spl->getCTime());
} catch (RuntimeException $e) {
  echo get_class($e) . ": " . $e->getMessage() . "\n";
}
