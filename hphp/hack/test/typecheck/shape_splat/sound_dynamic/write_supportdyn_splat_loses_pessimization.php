<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

final class ShapeSplatWriteNonSdt {
  public function takes_int(int $_): void {}
}

function require_supportdyn_shape_splat_write_nonsdt(
  supportdyn<ShapeSplatWriteNonSdt> $_,
): void {}

function require_supportdyn_mixed(supportdyn<mixed> $_): void {}

// A write through supportdyn<shape(...T)> must pessimize the existing fields:
// the existing field may have originated from dynamic even though the newly
// written field is known precisely.
function write_supportdyn_splat_loses_pessimization<
  T as supportdyn<shape(...)>,
>(supportdyn<shape(...T, 'existing' => ShapeSplatWriteNonSdt)> $s): void {
  $s['added'] = 1;
  require_supportdyn_shape_splat_write_nonsdt($s['existing']);
}

function write_supportdyn_splat_preserves_unknown_fields(
  supportdyn<shape(...shape(...), 'existing' => int)> $s,
): void {
  $s['added'] = 1;
  require_supportdyn_mixed(Shapes::idx($s, 'unknown'));
}
