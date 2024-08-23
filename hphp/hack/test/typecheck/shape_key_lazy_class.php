<?hh

class C {
  const type T = shape(self::class => int);
}
type T = shape(C::class => string);
