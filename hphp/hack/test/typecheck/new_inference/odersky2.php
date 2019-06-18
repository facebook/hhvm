<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class MyList<Ta> {
  /* HH_FIXME[4110] */
  public function match<Tb>(ListVisitor<Ta,Tb> $v): Tb {
  }
}

type ListVisitor<Ta, Tb> = shape(
  'caseNil' => (function() : Tb),
  'caseCons' => (function(Ta, MyList<Ta>): Tb)
);

/* HH_FIXME[4110] */
function nil<Ta>(): MyList<Ta> {
}
/* HH_FIXME[4110] */
function cons<Ta>(Ta $x, MyList<Ta> $xs) : MyList<Ta> {
}

function append<Tc>(MyList<Tc> $xs, MyList<Tc> $ys) : MyList<Tc> {
  return $xs->match(
    shape('caseNil' => () ==> $ys,
          'caseCons' => ($x, $xs1) ==> cons($x, append($xs1, $ys))));
}
