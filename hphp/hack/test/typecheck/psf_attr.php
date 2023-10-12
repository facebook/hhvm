<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.


<<__ProvenanceSkipFrame>>
function hgoldstein(): vec<int> {
  return vec[1, 2, 3];
}

final class Foobar {

  <<__ProvenanceSkipFrame>>
  public function lol(): dict<string, int> {
    return dict['rofl' => 42];
  }

}


function other_hgoldstein(): vec<int> {
  return vec[1, 2, 3];
}
