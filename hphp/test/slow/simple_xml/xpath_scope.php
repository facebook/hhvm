<?php

class Config {

  protected $_sections;

  public function __construct() {
    $config = simplexml_load_string('<config><sections><payment type="group" /><a /><b /></sections></config>');
    $this->_sections = $config->sections;
  }
    
  public function getSections() {
    return $this->_sections;
  }

}

$config = new Config();
$sections = $config->getSections();

// We should still have our original root object here,
// so we can do xpath lookups in the tree.
$node = $sections->xpath('payment[@type="group"]');
var_dump($node);
