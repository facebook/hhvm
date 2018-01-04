<?hh

function stringAndIntProvider((function (string, int...):void) $fn): void {
  $fn("Hello", 1, 2, 3, 4);
}

print "String and int provider string \$str, int ...\$x\n";
stringAndIntProvider((string $str, int ...$x) ==> {
  var_dump($str);
  var_dump($x);
});

print "String and int provider \$str, ...\$x\n";
stringAndIntProvider(($str, ...$x) ==> {
  var_dump($str);
  var_dump($x);
});

print "String and int provider \$str, ...\n";
stringAndIntProvider(($str, ...) ==> {
  var_dump($str);
  var_dump(func_get_args());
});


function stringAndTupleProvider(
  (function (string, (int, int)...):void) $fn
): void {
  $fn("Hello", tuple(1, 2), tuple(3, 4));
}

print "String and tuple provider \$str, ...\$tuples\n";
stringAndTupleProvider(($str, ...$tuples) ==> {
    var_dump($str);
    var_dump($tuples);
});

print "String and tuple provider \$str, (int, int) ...\$tuples\n";
stringAndTupleProvider((string $str, (int, int) ...$tuples) ==> {
    var_dump($str);
    var_dump($tuples);
});
