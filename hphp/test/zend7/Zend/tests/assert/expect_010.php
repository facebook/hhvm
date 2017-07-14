<?php
class One {
    public function __construct() {
        assert(false);
    }
}
class Two extends One {}

new Two();
?>
