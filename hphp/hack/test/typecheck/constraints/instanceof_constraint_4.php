<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface JJInterface {}

abstract class JJAbstract {}

final class JJGeneric<T as JJAbstract> implements JJInterface {}

function jj_f1(JJInterface $item): void {
  if ($item is JJGeneric<_>) {
    jj_f2($item);
  }
}

function jj_f2<T as JJAbstract>(JJGeneric<T> $item): void {}
