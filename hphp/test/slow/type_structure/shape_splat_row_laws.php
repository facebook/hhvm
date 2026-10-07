<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TOpenBase = shape('x' => int, ...);

/* empty shape is left and right identity for merge to this normalizes to
   shape('x' => int, ...) */
type TOpenIdentity = shape(...shape(), ...TOpenBase, ...shape());


type TTypedBase = shape('x' => int, string...);

/* As above, empty shape is identity for merge so this normalizes to
   shape('x' => int, string...) */
type TTypedIdentity = shape(...shape(), ...TTypedBase, ...shape());

/* Spreading nothing gives you the bottom row then lifting back into a type
   with shape gives you the bottom type, i.e. nothing */
type TOpenThenBottom = shape(...TOpenBase, ...nothing);
type TBottomThenTyped = shape(...nothing, ...TTypedBase);

function structure(string $alias): dict<arraykey, mixed> {
  $ts = HH\type_structure_for_alias($alias);
  $result = dict['kind' => $ts['kind']];
  if (array_key_exists('fields', $ts)) {
    $result['fields'] = $ts['fields'];
  }
  if (array_key_exists('allows_unknown_fields', $ts)) {
    $result['allows_unknown_fields'] = $ts['allows_unknown_fields'];
  }
  if (array_key_exists('variadic_type', $ts)) {
    $result['variadic_type'] = $ts['variadic_type'];
  }
  return $result;
}

<<__EntryPoint>>
function main(): void {
  invariant(
    structure(nameof TOpenIdentity) === structure(nameof TOpenBase),
    'empty shape was not identity for an untyped-open row',
  );
  invariant(
    structure(nameof TTypedIdentity) === structure(nameof TTypedBase),
    'empty shape was not identity for a typed-open row',
  );
  $open_bottom = structure(nameof TOpenThenBottom);
  invariant(
    $open_bottom === dict['kind' => TypeStructureKind::OF_NOTHING],
    'bottom did not absorb the open prefix and discard its shape data',
  );
  $bottom_typed = structure(nameof TBottomThenTyped);
  invariant(
    $bottom_typed === dict['kind' => TypeStructureKind::OF_NOTHING],
    'bottom did not absorb the typed-open suffix and discard its shape data',
  );

  echo 'open: '.json_encode(
    structure(nameof TOpenIdentity),
    JSON_FB_FORCE_HACK_ARRAYS,
  )."\n";
  echo 'typed: '.json_encode(
    structure(nameof TTypedIdentity),
    JSON_FB_FORCE_HACK_ARRAYS,
  )."\n";
  echo 'open bottom: '.json_encode(
    $open_bottom,
    JSON_FB_FORCE_HACK_ARRAYS,
  )."\n";
  echo 'bottom typed: '.json_encode(
    $bottom_typed,
    JSON_FB_FORCE_HACK_ARRAYS,
  )."\n";
}
