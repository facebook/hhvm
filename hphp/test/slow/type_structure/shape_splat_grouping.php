<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TA = shape('a' => int, 'shared' => int, string...);
type TB = shape('b' => bool, 'shared' => string);
type TC = shape('c' => float, 'shared' => arraykey);

/* Normalizes to shape
   ('shared' => string, 'a' => int, 'b' => bool, string...)
                  4             1           2      4 */
type TAB = shape(...TA, ...TB);

/* Normalizes to
   shape('shared' => arraykey, 'a' => int, 'b' => bool, 'c' => float, string...)
                        7              1           2             3     4 */
type TLeftGrouped = shape(...TAB, ...TC);

/* Normalizes to shape('shared' => arraykey, 'b' => bool, 'c' => float)
                                      7              2             3 */
type TBC = shape(...TB, ...TC);

/* Normalizes to
   shape('shared' => arraykey, 'a' => int, 'b' => bool, 'c' => float, string...)
                       7               1            2            3     4 */
type TRightGrouped = shape(...TA, ...TBC);

/* Normalizes to
   shape('shared' => arraykey, 'a' => int, 'b' => bool, 'c' => float, string...)
                       7               1            2            3     4 */
type TFlat = shape(...TA, ...TB, ...TC);

function summary(string $alias): dict<arraykey, mixed> {
  $ts = HH\type_structure_for_alias($alias);
  return dict[
    'fields' => $ts['fields'],
    'allows_unknown_fields' => $ts['allows_unknown_fields'],
    'variadic_type' => $ts['variadic_type'],
  ];
}

<<__EntryPoint>>
function main(): void {
  $left = summary(nameof TLeftGrouped);
  $right = summary(nameof TRightGrouped);
  $flat = summary(nameof TFlat);

  //     shape(...shape(...TA, ...TB), ...TC)
  // === shape(...TA, ...shape(...TB, ...TC))
  // === shape(...TA, ...TB, ...TC)
  invariant($left === $flat, 'left grouping changed the result');
  invariant($right === $flat, 'right grouping changed the result');

  echo json_encode($flat, JSON_FB_FORCE_HACK_ARRAYS)."\n";
}
