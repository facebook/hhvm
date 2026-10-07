<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Illegal mutually recursive cycle
type TCycleLeft = shape(...TCycleRight, 'left' => int);
type TCycleRight = shape(...TCycleLeft, 'right' => string);

<<__EntryPoint>>
function main(): void {
  HH\type_structure_for_alias(nameof TCycleLeft);
}
