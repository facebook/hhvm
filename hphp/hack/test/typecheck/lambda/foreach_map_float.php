<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function testit():void {
  $m = Map { 3 => 3.2 };
  $results = Map {};

  foreach ($m as $theme_id => $score) {
    $current_score = $results->get($theme_id);
    if ($current_score !== null) {
      $score += $current_score;
    }
    $results->set($theme_id, $score);
  }
}
