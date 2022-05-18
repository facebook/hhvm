<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface I {}

function bad_eq_and_as(): I with { type T as arraykey = int } {}

function bad_eq_and_super(): I with { ctx C super [defaults] = [] } {}

function bad_eq_and_both_super_and_as(
  I with { ctx C as [read_globals] super [globals] = [globals] } $_
): void {}
