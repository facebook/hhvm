<?hh

class C<reify T> {}

type Ta = C<(function (int, bool): string)>;
type Tc = C<(function (int, bool, float...): string)>;
