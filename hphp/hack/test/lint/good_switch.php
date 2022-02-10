<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.


function each(
  Traversable<shape('name' => string)> $xs,
  (function (shape('name' => string)): void) $f,
): void {
  foreach ($xs as $x) {
    $f($x);
  }
}
type ShapeWithString = vec<shape(
    'name' => string,
)>;

final abstract class SwitchTest {

  public static function withLoop(ShapeWithString $inputs): void {
    foreach ($inputs as $input) {
      switch ($input['name']) {
        case 'case1':
          break;
        case 'case2':
          break;
        default:

      }
    }
  }

  public static function withLambda(ShapeWithString $inputs): void {
    each($inputs, $input ==> {
        $x = $input['name'];
        switch ($x) {
        case 'case1':
          break;
        case 'case2':
          break;
        default:

        }
      });
  }
}
