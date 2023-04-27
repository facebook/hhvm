<?hh

<<file:__EnableUnstableFeatures('type_const_multiple_bounds')>>

interface AVeryLongInterfaceNameThatBarelyFitsIn1Line {
  abstract const type T;
  abstract const type TVeryLongTypeConstName;
}

interface AnInterfaceWithNotSoLengthyName
  extends AVeryLongInterfaceNameThatBarelyFitsIn1Line {
  abstract const type TMediumLength;
  abstract const type TVeryLongTypeConstNameShouldGoOnASeparateLine;
}

interface AnInterface {}

interface Box {
  abstract const type T;
}

function param_fits_in_1line(Box with { type T = int } $_): void {}

function param_barely_fits_in_1line(Box with { type T = AnInterface } $_): void {}

function param_cannot_fit_in_1line(
  AVeryLongInterfaceNameThatBarelyFitsIn1Line with { type T = AVeryLongInterfaceNameThatBarelyFitsIn1Line } $_,
): void {}

function param_refinement_member_split(
  AnInterfaceWithNotSoLengthyName with {
    type TVeryLongTypeConstNameShouldGoOnASeparateLine = AVeryLongInterfaceNameThatBarelyFitsIn1Line
  } $_,
): void {}

function param_split_between_members(
  SomeInterface with { type T1 = int; type T2 = string; type T3 = float; } $_,
): void {}

function generic_fits_in_1line<
  TInner as shape(...),
  TInterface as AnInterfaceWithNotSoLengthyName with { type T = TInner },
>(TInterface $_): void {}

function generic_split_after_with<
  TInterface as AnInterfaceWithNotSoLengthyName with { type TMediumLength = int },
>(TInterface $_): void {}

function generic_split_between_members<
  TInterface as AnInterfaceWithNotSoLengthyName with {
    type T = int;
    type TMediumLength = string;
  },
>(TInterface $_): void {}
