<?php

class BaseDerp {
  public function genDerp() {
    yield 'derp';
  }
}

class ShortDerp extends BaseDerp {}

$sd = new ShortDerp;
var_dump($sd->genDerp()->getOrigFuncName());
var_dump($sd->genDerp()->getCalledClass());
