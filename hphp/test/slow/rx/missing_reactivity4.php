<?hh // strict

interface I {}

<<__OnlyRxIfImpl(I::class)>>
function f(): void {}

<<__EntryPoint>> function main(): void {}
