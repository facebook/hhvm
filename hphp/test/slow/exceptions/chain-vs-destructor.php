<?php

class X {
  function __destruct() {
    global $complexException, $previous;
    var_dump(__METHOD__);
    $previous = $complexException->getPrevious();
  }
}

function createExceptionWithNontrivialPreviousDestructor() {
  global $complexException;
  $complexException = new Exception("complex");
  $reflectionProp = (new ReflectionClass('Exception'))->getProperty('previous');
  $reflectionProp->setAccessible(true);
  $reflectionProp->setValue($complexException, array(new X()));
  return $complexException;
}

try {
  try {
    throw new Exception("simple");
  } finally {
    throw createExceptionWithNontrivialPreviousDestructor();
  }
} catch (Exception $e) {
  var_dump($e->getMessage());
}
var_dump($previous);
