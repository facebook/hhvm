<?hh

class C<<<__Warn>> reify T> {}

function f<reify T>(T $x) {}

// Warn
f<(C<int>, string)>(tuple(new C<string>(), 'hi'));
// Error because second one errors despite the first one should warn
f<(C<int>, int)>(tuple(new C<string>(), 'hi'));
