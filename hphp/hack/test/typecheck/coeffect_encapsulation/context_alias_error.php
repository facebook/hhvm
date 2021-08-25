<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newctx x as [] = [defaults];

function test()[x]: void {}
