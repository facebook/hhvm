<?php

class Club {
  protected $app_id = 0;
}

class Glub extends Club {
  public function go() {
    var_dump(property_exists($this, 'app_id'));
  }
}

$g = new Glub();
$g->go();
var_dump(property_exists($g, 'app_id'));
