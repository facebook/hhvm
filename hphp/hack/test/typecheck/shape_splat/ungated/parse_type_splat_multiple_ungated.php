<?hh
// No __EnableUnstableFeatures attribute — every splat element should be
// independently rejected, one error per `...T`.

type A = shape('x' => int);
type B = shape('y' => string);
type C = shape(...A, 'z' => bool, ...B);

function test(C $s): void {}
