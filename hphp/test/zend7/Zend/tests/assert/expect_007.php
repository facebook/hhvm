<?php
$next = 1;
$data = array(
    "key" => "X-HTTP ",
    "value" => "testing"
);

class HeaderMalfunctionError extends AssertionError {}

assert (preg_match("~^([a-zA-Z0-9-]+)$~", $data["key"]), new HeaderMalfunctionError("malformed key found at {$next} \"{$data["key"]}\""));
?>
