<?php

class MyClass {
  private function getKeyPrefix() {
    return 'foo:';
  }

  public function makeArray() {
    $data = array();
    $p = $this->getKeyPrefix();

    $data[$p.'a'] = 2;
    $data[$p.'b'] = 3;
    $data[$p.'c'] = 4;
    $data[$p.'d'] = 5;

    return $data;
  }
}

var_dump((new MyClass)->makeArray());
