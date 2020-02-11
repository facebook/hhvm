<?hh <<__EntryPoint>> function main(): void {
$tests = varray[
    varray[8853, '&oplus;',  "e28a95"],
    varray[8855, '&otimes;', "e28a97"],
    varray[8869, '&perp;',   "e28aa5"],
    varray[8901, '&sdot;',   "e28b85"],
    varray[8968, '&lceil;',  "e28c88"],
    varray[8969, '&rceil;',  "e28c89"],
    varray[8970, '&lfloor;', "e28c8a"],
    varray[8971, '&rfloor;', "e28c8b"],
    varray[9001, '&lang;',   "e28ca9"],
    varray[9002, '&rang;',   "e28caa"]
];

foreach ($tests as $test) {
    var_dump(htmlentities(pack('H*', $test[2]), ENT_QUOTES, 'UTF-8'));
}

foreach ($tests as $test) {
    list(,$result) = unpack('H6', html_entity_decode($test[1], ENT_QUOTES, 'UTF-8'));
    var_dump($result);
}
}
