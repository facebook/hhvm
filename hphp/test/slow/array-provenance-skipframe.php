<?hh

<<__ProvenanceSkipFrame>>
function foo($v) {
    $ret = vec[];
    foreach ($v as $x) {
        $ret[] = $x;
    }
    return $ret;
}

<<__ProvenanceSkipFrame>>
function returns_static() {
    return vec[1, 2, 3];
}

<<__EntryPoint>>
function main() {
    $x = foo(vec[1, 2, 3]);
    var_dump(HH\get_provenance($x));
    var_dump(HH\get_provenance(returns_static()));
}
