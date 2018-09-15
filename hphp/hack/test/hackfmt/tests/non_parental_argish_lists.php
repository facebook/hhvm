<?hh

f(vec[
  $foo, // single-line comment forces the vec's rule to split, but not f's
  $bar,
]);

Vec\map($foo, $x ==> {
  bar($x);
  baz($x);
});

const dict<string, shape(
  'fooooooo' => string,
  'baaaaaar' => string,
  'baaaaaaz' => string,
  'quuuuuux' => string,
)> X = dict[];
