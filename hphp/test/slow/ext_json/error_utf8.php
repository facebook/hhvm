<?php <<__EntryPoint>> function main() {
$data = "\xB1\x31";
$data = json_encode($data);
var_dump(json_last_error());
}
