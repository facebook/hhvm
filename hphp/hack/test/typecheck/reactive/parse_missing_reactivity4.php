<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface I {}

<<__OnlyRxIfImpl(I::class)>>
function f(): void {}
