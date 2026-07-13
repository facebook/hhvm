<?hh

function i1() :mixed{
        echo "i1\n";
        return 0;
}

function i2() :mixed{
        echo "i2\n";
        return 0;
}

function i3() :mixed{
        echo "i3\n";
        return 0;
}

function i4() :mixed{
        echo "i4\n";
        return 0;
}

function i5() :mixed{
        echo "i5\n";
        return 0;
}

function i6() :mixed{
        echo "i6\n";
        return 0;
}
<<__EntryPoint>> function main(): void {
$a = vec[vec[0]];
$b = vec[vec[1]];
$c = vec[vec[2]];

$__1=i1(); $__2=i2(); $__3=i3(); $__4=i4(); $__5=i5(); $__6=i6();
$__v = $c[$__5][$__6]; $b[$__3][$__4] = $__v; $a[$__1][$__2] = $__v;
var_dump($a);
var_dump($b);
var_dump($c);

$__1=i1(); $__2=i2(); $__3=i3(); $__4=i4(); $__5=i5(); $__6=i6();
$__v = -$c[$__5][$__6]; $b[$__3][$__4] = $__v; $a[$__1][$__2] = $__v;
var_dump($a);
var_dump($b);
var_dump($c);

$__1=i1(); $__2=i2(); $__3=i3(); $__4=i4(); $__5=i5(); $__6=i6();
$__bv = +$c[$__5][$__6]; $b[$__3][$__4] = $__bv; $__av = -$__bv; $a[$__1][$__2] = $__av;
var_dump($a);
var_dump($b);
var_dump($c);
}
