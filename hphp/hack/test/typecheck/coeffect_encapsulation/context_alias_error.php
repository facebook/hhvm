<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newtype x as [] = [defaults];

function test()[x]: void {}
