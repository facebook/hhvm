<?hh

class C {
  const string KEY = 'KEY';

  const mixed X = shape(
    0 => 0,
    's' => 's',
    C::class => C::class,
    C::KEY => C::KEY,
  );
}
