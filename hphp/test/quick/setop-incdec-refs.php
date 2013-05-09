<?php
// Copyright 2004-2013 Facebook. All Rights Reserved.

function properties() {
  $c = new stdclass;
  $a = 5;
  $c->foo =& $a;
  var_dump($c);
  $x = $c->foo += 10;
  var_dump($c);
  var_dump($a);

  $x = --$c->foo;
  var_dump($c);
  var_dump($a);
}

function elements() {
  $a = array();
  $v = 'a string';
  $a['ref'] =& $v;
  var_dump($a);
  $x = $a['ref'] .= ' tail';
  var_dump($a);
  var_dump($v);

  $x = $a['ref']++;
  var_dump($a);
  var_dump($v);
}
properties();
elements();

