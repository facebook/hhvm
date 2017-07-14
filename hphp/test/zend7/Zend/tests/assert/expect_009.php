<?php
class One {
    public function __construct() {
    }
}
class Two extends One {
    public function __construct() {
        assert(false);
    }
}
new Two();
?>
