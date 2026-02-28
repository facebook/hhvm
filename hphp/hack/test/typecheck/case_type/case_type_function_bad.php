<?hh
<<file: __EnableUnstableFeatures('case_types')>>

case type NoTwoFunctions = (function(): string) | (function(): int);

case type NoFunctionRef =
  | HH\FunctionRef<(function(): string)>
  | (function(): int);

case type NoClosure = Closure | (function(): int);
