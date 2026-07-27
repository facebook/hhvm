<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TOpenAlias = shape('p' => float, ...);

// Splat an OPEN shape via a type alias.
function splat_open_alias(shape(...TOpenAlias) $s): void {
  hh_expect<shape('p' => float, ...)>($s);
  hh_expect<float>($s['p']);
}
