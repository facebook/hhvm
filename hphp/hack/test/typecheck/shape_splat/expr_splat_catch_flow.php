<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_expression',
)>>

function checkpoint(): void {}

function test(dynamic $operand): void {
  $x = 0;
  try {
    checkpoint(); // Seed catch flow before changing $x.
    $x = 'wrong';
    shape(...$operand);
  } catch (Throwable $_) {
    hh_show($x);
  }
}
