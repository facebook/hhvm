<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function getThing():string { return "a"; }
}

function genMapWithKey<Tk as arraykey, Ta, Tb>(
    KeyedTraversable<Tk, Ta> $collection,
    (function(Tk, Ta): Tb) $mapping_function,
  ): Map<Tk, Tb> {
    return Map{};
  }

function genMap<Ta, Tb>(
    Traversable<Ta> $collection,
    (function(Ta): Tb) $mapping_function,
  ): Vector<Tb> {
    return Vector{};
  }

function make_map<Tv>(Tv $v):Map<string,Tv> {
  return Map { 's' => $v };
}
function genConfigFromBank(
    Map<string, darray<int,C>> $actions_list,
    darray<int,C> $action,
  ): void {
    $actions_list = make_map($action);
    $action_names = genMapWithKey<string,_,_>(
      $actions_list,
      ($distance_level, $actions) ==> {
        return genMap<_,string>($actions, $action ==> $action->getThing());
      },
    );
}
