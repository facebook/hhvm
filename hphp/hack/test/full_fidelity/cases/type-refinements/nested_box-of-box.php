<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface Box {
  const type T;
  function box_of_box_of_int(): Box with { type T as Box with { type T = int } };
}
