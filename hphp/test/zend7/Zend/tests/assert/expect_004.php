<?php
try {
    assert(false, "I require this to succeed");
} catch (AssertionError $ex) {
    var_dump($ex->getMessage());
}
?>
