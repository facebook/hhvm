<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function getRecursiveResult(
  Map<string, Map<string, mixed>> $s,
  bool $b,
): Map<string, mixed> {
  $item = Map {};
  foreach ($s as $name => $substructures) {
    foreach ($substructures as $args2 => $substructure) {
      invariant($substructure instanceof Map, 'map expected');
      $list = Vector {};
      if ($b) {
        $item[$name] = $list->map(
          (string $id) ==> {
            return getRecursiveResult($substructure, $b);
          },
        );
      } else {
        $item[$name] = $list;
      }
    }
  }
  return $item;
}
