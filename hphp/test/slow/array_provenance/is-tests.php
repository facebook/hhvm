<?hh

type AShape = shape("foo" => int, "bar" => int);
type ATuple = (int, int, int);

function coerce_dict($arr) {
    try {
        var_dump($arr as dict<_,_>);
    } catch (Exception $e) {
        var_dump("failed");
    }
}

function coerce_vec($arr) {
    try {
        var_dump($arr as vec<_>);
    } catch (Exception $e) {
        var_dump("failed");
    }
}

function coerce_vec_or_dict($arr) {
    try {
        var_dump($arr as vec_or_dict);
    } catch (Exception $e) {
        var_dump("failed");
    }
}

function coerce_shape($arr) {
    try {
        var_dump($arr as AShape);
    } catch (Exception $e) {
        var_dump("failed");
    }
}

function coerce_tuple($arr) {
    try {
        var_dump($arr as ATuple);
    } catch (Exception $e) {
        var_dump("failed");
    }
}

<<__EntryPoint>>
function main() {
    $a = vec[1, 2, 3];
    $b = dict["foo" => 5, "bar" => 6];
    print "---- is vec_or_dict ----\n";
    var_dump($a is vec_or_dict);
    var_dump($b is vec_or_dict);
    print "---- as vec_or_dict ----\n";
    coerce_vec_or_dict($a);
    coerce_vec_or_dict($b);
    print "---- is vec ----\n";
    var_dump($a is vec<_>);
    var_dump($b is vec<_>);
    print "---- as vec ----\n";
    coerce_vec($a);
    coerce_vec($b);
    print "---- is dict ----\n";
    var_dump($a is dict<_, _>);
    var_dump($b is dict<_, _>);
    print "---- as dict ----\n";
    coerce_dict($a);
    coerce_dict($b);
    print "---- is_vec ----\n";
    var_dump(is_vec($a));
    var_dump(is_vec($b));
    print "---- is_dict ----\n";
    var_dump(is_dict($a));
    var_dump(is_dict($b));
    print "--- is \$tuple ----\n";
    var_dump($a is ATuple);
    var_dump($b is ATuple);
    print "--- as \$tuple ----\n";
    coerce_tuple($a);
    coerce_tuple($b);
    print "--- is \$shape ----\n";
    var_dump($a is AShape);
    var_dump($b is AShape);
    print "--- as \$tuple ----\n";
    coerce_shape($a);
    coerce_shape($b);

    $c = vec[false, true, dict[]];
    $d = dict["george"=> "curious"];
    print "--- is \$tuple  (no match) ----\n";
    var_dump($c is ATuple);
    var_dump($d is ATuple);
    print "--- as \$tuple (no match) ----\n";
    coerce_tuple($c);
    coerce_tuple($d);
    print "--- is \$shape (no match) ----\n";
    var_dump($c is AShape);
    var_dump($d is AShape);
    print "--- as \$tuple (no match) ----\n";
    coerce_shape($c);
    coerce_shape($d);
}
