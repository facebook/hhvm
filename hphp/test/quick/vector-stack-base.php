<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

class dumper {
  private static $idx = 0;
  private $n;
  public $prop = 'default value';
  function __construct() {
    $this->n = self::$idx++;
    printf("dumper %d constructing\n", $this->n);
  }
  function __destruct() {
    printf("dumper %d destructing\n", $this->n);
    if (isset($this->arr)) {
      var_dump($this->arr);
    }
    var_dump($this);
  }
}

function makeObj() {
  return new dumper;
}

function &makeObjRef() {
  return new dumper();
}

function makeArr() {
  $a = array();
  $a['dumper'] = new dumper;
  return $a;
}

function &makeArrRef() {
  static $a = array();
  var_dump($a);
  $a['dumper'] = new dumper;
  return $a;
}

function main() {
  echo "Entering main\n";
  // SetM with array base on the stack
  makeArr()['dumper'] = new stdclass;
  makeArrRef()['dumper'] = 24;
  makeArrRef()[234] = 234;

  // More complex SetM with array base on the stack, then with a ref
  makeArr()['dumper']->prop = null;
  makeArrRef()['dumper']->propp = null;
  makeArrRef()['dumper']->proppp = null;


  // Clear out the reference to destruct the array
  $ref =& makeArrRef();
  var_dump($ref);
  $ref = null;

  // SetM and CGetM with an object base on the stack
  makeObj()->prop = 'foo';
  var_dump(makeObj()->prop);
  makeObjRef()->prop = 'bar';
  var_dump(makeObjRef()->prop[2]);

  // UnsetM
  unset(makeArr()['dumper']);

  echo "Done with main\n";
}

echo "Calling main\n";
main();
echo "Main has returned\n";

