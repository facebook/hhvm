<?php

class MyReflectionFunction extends ReflectionFunctionAbstract {
  public function __toString() {
    return "MyReflectionFunction";
  }
}

echo new MyReflectionFunction;
