<?php
class A {
    public function __sleep() {
        throw new Exception("Failed");
    }
}

class B implements Serializable {
    public function serialize() {
        return NULL;
    }

    public function unserialize($data) {
    }
}

$data = array(new A, new B);

try {
    serialize($data);
} catch (Exception $e) { 
    var_dump($e->getMessage());
}
?>