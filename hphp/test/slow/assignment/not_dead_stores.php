<?php

function getTestArray() {
  return array('opera', '12', '16');
}

function stringFunc() {
  $results = getTestArray();
  var_dump($results);
  if (!$results) return null;

  $groups = array();
  $prefix = 'foo.browser';

  if ($results[0]) {
    $groups[] = ($prefix .= '.'.$results[0]);

    if ($results[1]) {
      $groups[] = ($prefix .= '.'.$results[1]);

      if ($results[2]) {
        $groups[] = ($prefix .= '-'.$results[2]);
      }
    }
  }

  return $groups;
}

function intFunc() {
  $results = getTestArray();
  var_dump($results);
  if (!$results) return null;

  $groups = array();
  $prefix = 1;

  if ($results[0]) {
    $groups[] = ($prefix += $results[0]);

    if ($results[1]) {
      $groups[] = ($prefix += $results[1]);

      if ($results[2]) {
        $groups[] = ($prefix += $results[2]);
      }
    }
  }

  return $groups;
}

var_dump(stringFunc());
var_dump(intFunc());
