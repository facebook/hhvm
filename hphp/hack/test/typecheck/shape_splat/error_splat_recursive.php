<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Self-referential splat should error (cycle detection)
type TRec = shape(...TRec, 'x' => int);

function test(TRec $s): void {}
