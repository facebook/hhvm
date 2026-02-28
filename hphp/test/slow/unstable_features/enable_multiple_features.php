<?hh

// Can enable multiple features with one use of attribute

<<file:__EnableUnstableFeatures(
  'union_intersection_type_hints',
  'case_types',
)>>

case type t = bool;

function f(): (bool | string) {
  return 1;
}
