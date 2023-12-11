<?hh <<__EntryPoint>> function main(): void {
$tests = vec[
    vec[8853, '&oplus;',  "e28a95"],
    vec[8855, '&otimes;', "e28a97"],
    vec[8869, '&perp;',   "e28aa5"],
    vec[8901, '&sdot;',   "e28b85"],
    vec[8968, '&lceil;',  "e28c88"],
    vec[8969, '&rceil;',  "e28c89"],
    vec[8970, '&lfloor;', "e28c8a"],
    vec[8971, '&rfloor;', "e28c8b"],
    vec[9001, '&lang;',   "e28ca9"],
    vec[9002, '&rang;',   "e28caa"]
];

foreach ($tests as $test) {
    var_dump(htmlentities(pack('H*', $test[2]), ENT_QUOTES, 'UTF-8'));
}

foreach ($tests as $test) {
    list(,$result) = unpack('H6', html_entity_decode($test[1], ENT_QUOTES, 'UTF-8'));
    var_dump($result);
}
}
