<?php

interface ArrayAccess {
  public function offsetExists($index);
  public function offsetGet($index);
  public function offsetSet($index, $newvalue);
  public function offsetUnset($index);
}
