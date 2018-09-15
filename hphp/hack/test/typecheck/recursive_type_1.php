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
function Test(MyMap<string, string> $extra_data): void {
  $ref_id = (string)coalesce2(
    $extra_data->get('ref_id'), // This has type ?var_1
    $extra_data->get('attachment_id'), // this also has type ?var_1
  );
  ForceString($ref_id);

}
