<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Illegal recursive cycle
type TCycle = shape(...TCycle, 'x' => int);

<<__EntryPoint>>
function main(): void {
  HH\type_structure_for_alias(nameof TCycle);
}
