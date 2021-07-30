<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newtype X as [] = [defaults];

function test()[X]: void {}
