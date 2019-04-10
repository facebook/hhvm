<?hh

class C<<<__Warn>> reify T> {}

function f<reify T>(T $x) {}

// Warn
f<shape('a' => C<int>, 'b' => string)>(shape('a' => new C<string>(), 'b' => 'hi'));
// Error because second one errors despite the first one should warn
f<shape('a' => C<int>, 'b' => int)>(shape('a' => new C<string>(), 'b' => 'hi'));
