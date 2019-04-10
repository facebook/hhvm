<?hh

set_error_handler(
  (int $errno,
  string $errstr,
  string $errfile,
  int $errline,
  array $errcontext
  ) ==> {
    echo "ERROR: ".$errstr." on line ".(string)$errline."\n";
    return true;
  }
);

function f<reify T>($x): @T { return $x; }

f<int>(1);
f<num>(1);
f<int>(1.1);
f<num>(1.1);
f<int>(true);

f<bool>(true);
f<bool>(1);

f<shape('x' => int, 'y' => string)>(shape('x' => 1, 'y' => "hi"));
f<shape('x' => int, 'y' => string)>(shape('x' => 1, 'y' => 2));

f<(int, string)>(tuple(1, "hi"));
f<(int, string)>(tuple(1, 2));
