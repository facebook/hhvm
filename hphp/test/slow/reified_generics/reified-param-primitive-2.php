<?hh

set_error_handler(
  (int $errno,
  string $errstr,
  string $errfile,
  int $errline,
  array $errcontext
  ) ==> {
    echo "Error: ".$errstr." on line ".(string)$errline."\n";
    return true;
}, E_ALL & ~E_WARNING
);

class C<<<__Warn>> reify T> {}

function f<reify T>(T $x) {}

// Warn
f<shape('a' => C<int>, 'b' => string)>(shape('a' => new C<string>(), 'b' => 'hi'));
// Error because second one errors despite the first one should warn
f<shape('a' => C<int>, 'b' => int)>(shape('a' => new C<string>(), 'b' => 'hi'));

// Warn
f<(C<int>, string)>(tuple(new C<string>(), 'hi'));
// Error because second one errors despite the first one should warn
f<(C<int>, int)>(tuple(new C<string>(), 'hi'));
