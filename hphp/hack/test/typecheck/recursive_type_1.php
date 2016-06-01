<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

final class MyMap<Tk, Tv> {
  public function get(Tk $k): ?Tv {
    return null;
  }
}

function coalesce2<Tr, Ta as Tr, Tb as Tr>(?Ta $a, Tb $b): Tr {
  return $a ?? $b;
}

function ForceString(string $x): void {}
function Test(
  /* this will work! MyMap<string,string> */ mixed $extra_data,
): void {
  invariant($extra_data instanceof MyMap, 'for hack');

  // At this point etxra_data has type MyMap<string,var_1>
  // for some unification variable var_1

  // $ref_id has fresh type Tr
  // But we need that
  //   ?var_1 <: ?Ta
  //   ?var_1 <:  Tb
  //   Ta <: Tr
  //   Tb <: Tr
  // Hence var_1 <: Ta
  // A solution is: Tr=string, Ta = string, Tb = ?string, var_1 = string
  $ref_id = (string) coalesce2(
    $extra_data->get('ref_id'), // This has type ?var_1
    $extra_data->get('attachment_id'), // this also has type ?var_1
  );
  ForceString($ref_id);

}
