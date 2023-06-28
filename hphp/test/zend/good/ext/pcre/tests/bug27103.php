<?hh
function iter($ar)
:mixed{
    foreach ($ar as $c) {
        echo htmlentities($c, 0, "UTF-8"), ": ", strlen($c), "\n";
    }
}
<<__EntryPoint>> function main(): void {
$teststr = "\xe2\x82\xac hi there";
iter(preg_split('//u', $teststr, -1, PREG_SPLIT_NO_EMPTY));
$matches = null;
preg_match_all_with_matches('/./u', $teststr, inout $matches);
iter($matches[0]);
}
