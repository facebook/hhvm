<?hh

// Can enable multiple features with one use of attribute

<<file:__EnableUnstableFeatures(
  'union_intersection_type_hints',
  'class_level_where',
)>>

interface I where this as I {
  public function get(): (bool | string);
}
