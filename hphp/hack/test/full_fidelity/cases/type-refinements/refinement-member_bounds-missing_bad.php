<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

interface I {}

function bad_no_bounds1(I with { type T } $a): void {}

function bad_no_bounds2(): I with { type T; ctx C } {}

function bad_no_bounds_first(): I with { type T; ctx C = []; } {}

function bad_no_bounds_last(): I with { type T as int; ctx C; } {}
