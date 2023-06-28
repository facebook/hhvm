<?hh

function functionProvider((function ((function (int...):void)...): void) $fn) :mixed{
  $fn(
    ((...$x) ==> var_dump($x)),
    ((int ...$x) ==> var_dump($x)),
    ((...$x) ==> {
      var_dump($x);
    }),
    ((int ...$x) ==> {
      var_dump($x);
    }),
  );
}


<<__EntryPoint>>
function main_lambdas_variadic_functions() :mixed{
print "Function provider ...\$fns\n";
functionProvider((...$fns) ==> {
  foreach ($fns as $fn) {
    $fn(1, 2, 3);
  }
});

print "Function provider (function (int...):void) ...\$fns\n";
functionProvider(((function (int...):void) ...$fns) ==> {
  foreach ($fns as $fn) {
    $fn(1, 2, 3);
  }
});

print "Function provider lambda1\n";
$lambda1 = (...$fns) ==> {
  foreach($fns as $fn) {
    $fn(1, 2, 3);
  }
};

functionProvider($lambda1);

print "Function provider lambda2\n";
$lambda2 = ((function (int...):void) ...$fns) ==> {
  $lambda1(...$fns);
};

functionProvider($lambda2);
}
