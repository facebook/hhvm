<?hh
// package pkg1
<<file: __EnableUnstableFeatures('require_package')>>

<<__EntryPoint, __RequirePackage(123)>> // parse error
function test() : void {}
