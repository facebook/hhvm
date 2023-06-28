<?hh

class S1 { public static $v = 0; }
function test_1() :mixed{
    $v = ++S1::$v;
    echo "Outer function increments \$v to $v\n";
    $f = function() use($v) {
        echo "Inner function reckons \$v is $v\n";
    };
    return $f;
}

class S2 { public static $v = 0; }
function test_2() :mixed{
    $v = S2::$v;
    $f = function() use($v) {
        echo "Inner function reckons \$v is $v\n";
    };
    $v = ++S2::$v;
    echo "Outer function increments \$v to $v\n";
    return $f;
}

class S3 { public static $v = ""; }
function test_3() :mixed{
    $v = S3::$v .= 'b';
    echo "Outer function catenates 'b' onto \$v to give $v\n";
    $f = function() use($v) {
        echo "Inner function reckons \$v is $v\n";
    };
    $v = S3::$v .= 'a';
    echo "Outer function catenates 'a' onto \$v to give $v\n";
    return $f;
}

<<__EntryPoint>> function main(): void {
$f = test_1(); $f();
$f = test_1(); $f();

$f = test_2(); $f();
$f = test_2(); $f();

$f = test_3(); $f();
$f = test_3(); $f();
}
