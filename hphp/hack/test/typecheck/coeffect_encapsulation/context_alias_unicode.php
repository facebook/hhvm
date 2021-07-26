<?hh
<<file:__EnableUnstableFeatures('context_alias_declaration')>>

newtype İ as [] = [defaults];

newtype i̇ as [] = [defaults];

newtype X as [] = [İ];

newtype Y as [İ] = [X];

newtype Z as [] = [i̇];

function test1()[X]: void {}

function test2()[Y]: void {}

function test3()[Z]: void {}

function test4()[İ]: void {}

function test5()[i̇]: void {}
