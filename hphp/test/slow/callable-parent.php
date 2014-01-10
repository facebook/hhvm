<?php

class Base { }

class NoMagic extends Base {
  public function checkCallables() {
    $meths = array(
      'parent::foobar',
      array(get_class($this), 'foobar'),
    );

    foreach ($meths as $meth) {
      var_dump(is_callable($meth));
    }
  }
}

class Magic extends NoMagic {
  function __call($meth, $args) { }
}

$nomagic = new NoMagic();
$nomagic->checkCallables();

$magic = new Magic();
$magic->checkCallables();
