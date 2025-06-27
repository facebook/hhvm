<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

function expecting<T>(T $_): void {}

class Upper {}

class Lower extends Upper {}

interface IAmConfused {
  abstract const type TAndSoIsThis as Upper;

}

interface IHasAbstractConst {
  abstract const type TButThisIsAbstract as IAmConfused;
}

abstract class HasConcreteConst implements IHasAbstractConst {
  const type TMustBeConcrete = this::TButThisIsAbstract::TAndSoIsThis;

  public static function returnAliasToAbstract(
    this $_,
    this::TMustBeConcrete $x,
  ): void {
  }
}

function refEm(): void {
  $fptr = HasConcreteConst::returnAliasToAbstract<>;

  expecting<
    HH\FunctionRef<(readonly function<
      TAndSoIsThis0 as Upper,
      TButThisIsAbstract0 as IAmConfused with {
        type TAndSoIsThis = TAndSoIsThis0 },
    >(
      HasConcreteConst with {
        type TButThisIsAbstract = TButThisIsAbstract0;
        type TMustBeConcrete = TAndSoIsThis0
      },
      TAndSoIsThis0,
    ): void)>,
  >($fptr);
}
