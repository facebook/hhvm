<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

namespace myNS
{
  newtype X as [] = [write_props, \HH\Contexts\defaults];
}

namespace
{
  function test()[myNS\X]: void {}
}
