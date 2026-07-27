<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Merge algebra: Req+Opt = Req(union), Opt+Req = Req(right)
type TDefaults = shape('theme' => string, 'lang' => string);
type TOverrides = shape(?'theme' => string);

type TConfig = shape(...TDefaults, ...TOverrides);

function test_optional_merge(TConfig $c): void {
  // theme: Req+Opt = Req(string|string) = Req(string)
  hh_expect<string>($c['theme']);
  // lang: only in defaults, stays Req(string)
  hh_expect<string>($c['lang']);
}
