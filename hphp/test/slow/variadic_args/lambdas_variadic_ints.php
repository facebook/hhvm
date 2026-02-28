<?hh

function intProvider((function (int...):void) $fn):void {
  $fn(1, 2, 3, 4, 5);
}


<<__EntryPoint>>
function main_lambdas_variadic_ints() :mixed{
print "Int provider ...\$x\n";
intProvider((...$x) ==> {
  var_dump($x);
});

print "Int provider int...\$x\n";
intProvider((int...$x) ==> {
  var_dump($x);
});

print "Int provider ...\n";
intProvider((...$args) ==> {
  var_dump($args);
});

print "Int provider long closure: int...\$x\n";
intProvider(function (int ...$x) {
  var_dump($x);
});

print "Int provider long closure: ...\$x\n";
intProvider(function (...$x) {
  var_dump($x);
});

print "Int provider lambda1\n";
$lambda1 = (...$x) ==> {
  var_dump($x);
};

intProvider($lambda1);

print "Int provider lambda2\n";
$lambda2 = (int ...$x) ==> {
  var_dump($x);
};

intProvider($lambda2);
}
