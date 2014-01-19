<?php

class JavaScriptPacker {
  public function foo() {
    $encode10 = $this->_getJSFunction('_encode10');
    var_dump($encode10);
    $encode36 = $this->_getJSFunction('_encode36');
    var_dump($encode36);
  }
  private function _getJSFunction($aName) {
    if (defined('self::JSFUNCTION'.$aName))
      return constant('self::JSFUNCTION'.$aName);
    else
      return '';
  }
  const JSFUNCTION_encode10 = 'function($charCode) { return $charCode; } ';
}
$obj = new JavaScriptPacker;
$obj->foo();
