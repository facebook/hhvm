<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newtype X as [] = [write_props, \HH\Contexts\defaults];

newtype Y as [] = [X];

function test1()[X]: void {}

function test2()[Y]: void {}
