<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function __construct(public int $fbid) { }
}

function testit(varray<C> $x):void {
  $vec = new Vector($x);
  $report_map = Map::fromItems(
    $vec->map(
      ($report) ==> Pair {
        $report->fbid,
        $report,
      }
  ));
}
