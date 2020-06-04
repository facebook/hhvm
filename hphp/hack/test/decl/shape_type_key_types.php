<?hh

type Shape = shape(
  0 => int,
  's' => string,
  C::class => classname<C>,
  C::KEY => string,
);
