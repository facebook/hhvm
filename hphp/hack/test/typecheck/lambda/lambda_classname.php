<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function mymap<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): vec<Tv2> {
  $result = vec[];
  foreach ($traversable as $value) {
    $result[] = $value_func($value);
  }
  return $result;
}

function mymap_switched<Tv1, Tv2>(
  (function(Tv1): Tv2) $value_func,
  Traversable<Tv1> $traversable,
): vec<Tv2> {
  $result = vec[];
  foreach ($traversable as $value) {
    $result[] = $value_func($value);
  }
  return $result;
}

final class Foo {}

function getFooClasses(): vec<classname<Foo>> {
  return vec[Foo::class];
}

function createFoosLambda(): void {
  // Context is used to determine $class, so this is actually safe
  $instances = mymap(getFooClasses(), $class ==> new $class());
}

function createFoosLambda_switched(): void {
  // Context is used to determine $class, so this is actually safe
  $instances = mymap_switched($class ==> new $class(), getFooClasses());
}

function createFoosLoop(): void {
  $instances = vec[];
  foreach (getFooClasses() as $class) {
    // all good here
    $instances[] = new $class();
  }
}
