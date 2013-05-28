<?php

$error = 'fatal error';
echo AdsConsoleRenderer::getInstance()->writeMsg('error', $error);
class AdsConsoleRenderer {
  public static function getInstance() {
    return new AdsConsoleRenderer();
  }
  function writeMsg($classname = '', $s = '') {
    echo $classname . "::" . $s;
  }
}
