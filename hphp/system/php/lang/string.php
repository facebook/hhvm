<?php

interface XHPChild {}

interface Stringish extends XHPChild {
  public function __toString();
}
