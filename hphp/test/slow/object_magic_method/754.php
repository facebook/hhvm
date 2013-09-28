<?php

class MemberTest {
  private $data = array();
  public function __set($name, $value) {
    echo "Setting '$name' to '$value'
";
    $this->data[$name] = $value;
  }
  public function __get($name) {
    echo "Getting '$name'
";
    if (array_key_exists($name, $this->data)) {
      return $this->data[$name];
    }
    return null;
  }
  public function __unset($name) {
    echo "Unsetting '$name'
";
    unset($this->data[$name]);
    return 1;
  }
}
$obj = new MemberTest;
$obj->a = 1;
var_dump($obj->a);
var_dump(isset($obj->a));
unset($obj->a);
