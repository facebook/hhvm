<?hh

<<file:__EnableUnstableFeatures('match_statements')>>

case type CT<T> = int where T = int | string where T = string;

function f<T>(CT<T> $ct): void {
  match ($ct) {
    _: int => ;
    _: string => ;
  }
}
