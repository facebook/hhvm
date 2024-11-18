<?hh

type Shape = shape(
  0 => int,
  's' => string,
  nameof C => classname<C>,
  C::KEY => string,
);
