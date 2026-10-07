<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TMergedAcrossFiles = shape(...TCrossFile, 'local' => string);

<<__EntryPoint>>
function main(): void {
  require_once __DIR__.'/shape_splat_cross_file.inc';
  var_dump(HH\type_structure_for_alias(nameof TMergedAcrossFiles));
}
