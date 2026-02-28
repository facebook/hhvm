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

  public static function shouldRefineMeButNotSubtype(
    this $exactly_me,
    HasConcreteConst $subtype_or_me,
    this::TMustBeConcrete $shoud_refine_me_but_not_subtype,
  ): this {
    return $exactly_me;
  }

  public static function shouldntRefineMeOrSubtype(
    this $exactly_me,
    HasConcreteConst $subtype_or_me,
    HasConcreteConst::TMustBeConcrete $shouldnt_refine_anything,
  ): this {
    return $exactly_me;
  }
}

function refEm(): void {
  $fptr = HasConcreteConst::shouldRefineMeButNotSubtype<>;
  expecting<
    HH\FunctionRef<(readonly function<
      Tthis as HasConcreteConst with {
        type TButThisIsAbstract = TButThisIsAbstract0;
        type TMustBeConcrete = TAndSoIsThis0
      },
      TAndSoIsThis0 as Upper,
      TButThisIsAbstract0 as IAmConfused with {
        type TAndSoIsThis = TAndSoIsThis0 },
    >(Tthis, HasConcreteConst, TAndSoIsThis0): Tthis)>,
  >($fptr);

  $gptr = HasConcreteConst::shouldntRefineMeOrSubtype<>;
  expecting<
    HH\FunctionRef<(readonly function<
      Tthis as HasConcreteConst,
      TAndSoIsThis0 as Upper,
    >(Tthis, HasConcreteConst, TAndSoIsThis0): Tthis)>,
  >($gptr);
}
