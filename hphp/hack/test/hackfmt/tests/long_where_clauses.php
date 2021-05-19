<?hh

class C {
  public static function f___________________<
    Tx as A,
    Ty as B,
    Tz as C,
  >(
    Ty $y,
    Tz $z,
  ): void where Tx::Tt = Ty, Tx::Tu = Tz, Tx::Tv as D, Tx::Tw as E, {
    do_something($y, $z);
  }

  public static function g___________________<
    <<__Enforceable>> reify Tx as A___________________,
    Ty as B___________________,
    Tz as C___________________,
  >(
    Ty $y,
    Tz $z,
  ): void where Tx::Tt = Ty, Tx::Tu = Tz, Tx::Tv as D____________________, Tx::Tw as E_________________________, {
    do_something_else($y, $z);
  }

  public static function h___________________<
    <<__Enforceable>> reify Tx as A___________________,
    Ty__________________ as B___________________,
    Tz__________________ as C___________________,
  >(
    Ty__________________ $y,
    Tz__________________ $z,
    A___________________ $a,
    B___________________ $b,
  ): (
    A___________________,
    B___________________,
    B___________________,
    C___________________,
  ) where Tx::Tt = Ty__________________, Tx::Tu = Tz__________________, Tx::Tv as D____________________, Tx::Tw as E_________________________, {
    return tuple($a, $b, $y, $z);
  }
}
