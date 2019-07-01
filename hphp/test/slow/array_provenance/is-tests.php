<?hh

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

<<__EntryPoint>>
function main() {
    $a = vec[1, 2, 3];
    $b = dict[4 => 5, 5 => 6];
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
}
