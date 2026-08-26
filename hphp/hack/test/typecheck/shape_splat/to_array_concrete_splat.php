<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'union_intersection_type_hints')>>

type TBase = shape('x' => int, 'y' => string);
type TExtended = shape(...TBase, 'z' => bool);

function test_to_array_concrete_splat(TExtended $s): void {
  hh_expect<darray<string, (int | bool | string)>>(Shapes::toArray($s));
}
