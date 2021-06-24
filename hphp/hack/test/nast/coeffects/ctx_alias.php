<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newtype X as [] = [HH\Contexts\defaults];

function test()[\X]: void {}
