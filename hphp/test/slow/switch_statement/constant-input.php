<?php
// Copyright 2004-present Facebook. All Rights Reserved.

function s1() {
  switch (5) {
    case 1: return 'hi';
    case 2: case 3: case 4: case 5: return 'lol';
  }
}

function s2() {
  switch (1234) {
    case -5: return 'no';
    case -6: return 'nope';
    case -7: case -8: return 'still no';
  }
  return 'hi again!';
}

var_dump(s1(), s2());
