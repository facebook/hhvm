<?php <<__EntryPoint>> function main() {
var_dump(
    json_decode('""'),
    json_decode('"..".'),
    json_decode('"'),
    json_decode('""""'),
    json_encode('"'),
    json_decode(json_encode('"')),
    json_decode(json_encode('""'))
);
}
