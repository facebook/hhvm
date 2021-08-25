<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newctx X as [] = [defaults];

function test()[\X]: void {}
