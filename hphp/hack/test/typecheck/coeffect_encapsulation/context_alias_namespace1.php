<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

namespace myNS
{
  // one context is just name while other is fully qualified to test both get resolved correctly
  newctx X as [] = [write_props, defaults];
}

namespace
{
  function test()[myNS\X]: void {}
}
