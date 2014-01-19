<?php
class B implements Serializable {
    public function serialize() {
        throw new Exception("serialize");
        return NULL;
    }

    public function unserialize($data) {
    }
}

$data = array(new B);

try {
    serialize($data);
} catch (Exception $e) { 
    var_dump($e->getMessage());
}
?>