<?php
$instance = new class('foo') {
    public function __construct($i) {
    }
};
var_dump(serialize($instance));
?>
