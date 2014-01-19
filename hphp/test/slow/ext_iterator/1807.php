<?php

class Proxy extends IteratorIterator {
  function __construct($i) {
    parent::__construct($i);
  }
}
$i = new Proxy(new ArrayIterator(range(0, 5)));
foreach ($i as $v) {
 var_dump($v);
 }
