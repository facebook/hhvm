<?php
class MyExpectations extends AssertionError {
    public function __toString() {
        return sprintf(
            "[Message]: %s", __CLASS__);
    }
}

class One {
    public function __construct() {
        assert(false, (string) new MyExpectations());
    }
}
class Two extends One {}

new Two();
?>
