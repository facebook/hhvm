<?php

function_call(
  function() use ($x) : int {},
  (function() use($x) :   int {})(),
  function() use ($x) : int  { $short_function = 'is short'; },
  function()use ($x)   : int {
    $longer_function = 'cannot as easily be fit into one line';
    $on_account_of = 'its multiple long statements';
  },
  $foo,
);
