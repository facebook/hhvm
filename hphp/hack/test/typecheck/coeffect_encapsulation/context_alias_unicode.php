<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newctx İ as [] = [defaults];

newctx i̇ as [] = [defaults];

newctx X as [] = [İ];

newctx Y as [İ] = [X];

newctx Z as [] = [i̇];

function test1()[X]: void {}

function test2()[Y]: void {}

function test3()[Z]: void {}

function test4()[İ]: void {}

function test5()[i̇]: void {}
