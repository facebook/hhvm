<?hh

<<__ProvenanceSkipFrame>>
function foo($v) {
    $ret = varray[];
    foreach ($v as $x) {
        $ret[] = $x;
    }
    return $ret;
}

<<__ProvenanceSkipFrame>>
function returns_static() {
    return varray[1, 2, 3];
}

<<__EntryPoint>>
function main() {
    $x = foo(varray[1, 2, 3]);
    var_dump(HH\get_provenance($x));
    var_dump(HH\get_provenance(returns_static()));
}
