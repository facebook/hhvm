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

$a[i1()][i2()] = ($b[i3()][i4()] = $c[i5()][i6()]);
var_dump($a);
var_dump($b);
var_dump($c);

$a[i1()][i2()] = $b[i3()][i4()] = -$c[i5()][i6()];
var_dump($a);
var_dump($b);
var_dump($c);

$a[i1()][i2()] = -($b[i3()][i4()] = +($c[i5()][i6()]));
var_dump($a);
var_dump($b);
var_dump($c);
}
