<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class MyList<T> { }
class Two<T1,T2> { }

function foo<T1 as T2, T2 as T3, T3 as T1>(MyList<T1> $x): MyList<T3> {
//  hh_show_env();
  return $x;
}

class Boo {
  public static function bar<T1,T2,T3>(MyList<T1> $x): MyList<T3>
    where T1 as T2, T2 as T3, T3 as T1 {
//  hh_show_env();
  return $x;
}
}

class Bee {
  public static function hey<T1,T2,T3,Ta,Tb>(MyList<Two<T3,T1>> $x): MyList<Tb>
    where T1 as T3, T3 as T1, Two<T1,T1> as Tb, Tb as Two<T3,T1> {
//  hh_show_env();
  return $x;
}
}
