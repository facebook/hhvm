<?hh <<__EntryPoint>> function main(): void {
$tests = vec[
    "abc",
    "abc&amp;sfdsa",
    "test&#043;s &amp; some more &#68;",
    "test&#x2b;s &amp; some more &#X44;",
    "&; &amp &#a; &9; &#xyz;",
    "&kffjadfdhsjfhjasdhffasdfas;",
    "&#8787978789",
    "&",
    "&&amp;&",
    "&ab&amp;&",
];

foreach ($tests as $test) {
    var_dump(htmlentities($test, ENT_QUOTES, '', FALSE));
    var_dump(htmlspecialchars($test, ENT_QUOTES, '', FALSE));
}
}
