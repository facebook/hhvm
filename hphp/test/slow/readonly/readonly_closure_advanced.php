<?hh

function takes_mutable(
  (function():void) $f
) : void {
    $f();
  }

function takes_readonly(
  readonly (readonly function(): void) $f
) : void {
    $f();
  }

<<__EntryPoint>>
function test(): void {
  $y = readonly (): void ==> {
    echo "readonly fun\n";
  };
  takes_mutable($y);
  takes_readonly($y);

  $z = (): void ==> {
    echo "mutable fun\n";
  };
  takes_mutable($z);
  takes_readonly($z); // error here
}
