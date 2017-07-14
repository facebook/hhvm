<?php

class AA implements ArrayAccess {
    private $data = [];
    public function offsetExists($name) {
        echo "offsetExists($name)\n";
        return array_key_exists($name, $this->data);
    }
    public function &offsetGet($name) {
        echo "offsetGet($name)\n";
        if (!array_key_exists($name, $this->data)) {
            throw new Exception('Unknown offset');
        }
        return $this->data[$name];
    }
    public function offsetSet($name, $value) {
        echo "offsetSet($name)\n";
        $this->data[$name] = $value;
    }
    public function offsetUnset($name) {
        echo "offsetUnset($name)\n";
        unset($this->data[$name]);
    }
}

$aa = new AA;
var_dump(isset($aa[0][1][2]));
var_dump(isset($aa[0]->foo));
var_dump($aa[0] ?? 42);
var_dump($aa[0][1][2] ?? 42);

$aa[0] = new AA;
$aa[0][1] = new AA;
var_dump(isset($aa[0][1][2]));
var_dump($aa[0][1][2] ?? 42);

?>
