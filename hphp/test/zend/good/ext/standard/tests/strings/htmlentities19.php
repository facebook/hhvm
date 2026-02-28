<?hh <<__EntryPoint>> function main(): void {
$tests = vec[
    "\x41\xC2\x3E\x42", // Unicode TR #36, 3.1.1; do not consume valid successor bytes
    "\xE3\x80\x22",    // Unicode TR #36, 3.6.1; use strategy #2
    "\x41\x98\xBA\x42\xE2\x98\x43\xE2\x98\xBA\xE2\x98", // example from HTML5, section 2.4
];

foreach ($tests as $test) {
    $a = htmlentities($test, ENT_QUOTES | ENT_SUBSTITUTE, "UTF-8");
    var_dump($a, bin2hex($a));
    $a = htmlspecialchars($test, ENT_QUOTES | ENT_SUBSTITUTE, "UTF-8");
    var_dump($a, bin2hex($a));
}
}
