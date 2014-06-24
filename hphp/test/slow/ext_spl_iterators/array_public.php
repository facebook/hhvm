<?php

class test {
  public $one = 1;
  private $priv = 2;
  public $two = 2;
  protected $prot = 3;
}

$ai = new ArrayIterator(new test());

foreach ($ai as $key=>$val) {
  echo $key;
}

echo $ai->count();
