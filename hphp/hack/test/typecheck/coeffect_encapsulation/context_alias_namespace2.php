<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

// one context is just name while other is fully qualified to test both get resolved correctly
newctx X as [] = [write_props, \HH\Contexts\defaults];

newctx Y as [] = [X];

function test1()[X]: void {}

function test2()[Y]: void {}
