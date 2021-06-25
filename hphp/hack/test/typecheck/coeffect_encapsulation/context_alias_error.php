<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newtype x as [] = [HH\Contexts\defaults];

function test()[\x]: void {}
