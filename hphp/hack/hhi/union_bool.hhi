<?hh
<<file:__EnableUnstableFeatures('case_types')>>

namespace HH\Runtime {

/**
 * Types for common patterns in legacy PHP APIs,
 * where FALSE is used to denote failure.
 */
  case type BoolOrInt = bool | int;
  case type BoolOrString = bool | string;
  case type BoolOrFloat = bool | float;
  case type BoolOrNum = bool | num;

}
