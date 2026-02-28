<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

type ShapeType = shape(
  'x' => int,
  'y' => float,
);


function reKey<Ta as arraykey, Tv, Tk as arraykey>(
KeyedTraversable<Ta, Tv> $collection,
(function(Ta, Tv): Tk) $kgrip,
): Map<Tk, Tv> {
  return Map{};
}
function mapWithKey<Tk, Ta, Tb>(
KeyedTraversable<Tk, Ta> $collection,
(function(Tk, Ta): Tb) $mapping_function,
): darray<Tk, Tb>
{
  return dict[];
}
function testit(
    varray<ShapeType> $conversion_specs_shapes,
  ): darray<int, darray<string, num>> {
    return mapWithKey(
      $conversion_specs_shapes,
      ($_, $spec) ==> reKey(
        Shapes::toArray($spec),
        ($key, $_) ==> (string)$key,
      )->toDArray(),
    );
  }
