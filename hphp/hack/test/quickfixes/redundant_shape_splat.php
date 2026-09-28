<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Row = shape('x' => int);

function generic(shape(...Row) $row): void {}

function dynamic(shape(...dynamic) $row): void {}
