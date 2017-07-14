<?php
try {
    assert(false);
} catch (AssertionError $ex) {
    var_dump($ex->getMessage());
}
?>
