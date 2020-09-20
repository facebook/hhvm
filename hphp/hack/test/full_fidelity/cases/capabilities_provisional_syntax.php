<?hh

<<file: __EnableUnstableFeatures('coeffects_provisional')>>

function a(): void {}

function b()@{ mixed }: void {}

function c()@{ int + string }: void {}
