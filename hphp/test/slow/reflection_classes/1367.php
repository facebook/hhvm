<?php

class c {
  public $a = 'testa';

  protected $b = 'testb';
}

$c = new c();

$reflection = new \ReflectionClass($c);
$defaults = $reflection->getDefaultProperties();
print_r($defaults);
