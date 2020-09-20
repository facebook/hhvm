<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function test(mixed $c, string $id):int {
  if (!($c is KeyedContainer<_, _>)) {
            return 1;
          }

          if (idx($c, $id) === null) {
            return 2;
          }
          return 3;
}
