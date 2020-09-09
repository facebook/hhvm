<?hh // strict

interface I {}

<<__OnlyRxIfImpl(I::class)>>
function f(): void {}
