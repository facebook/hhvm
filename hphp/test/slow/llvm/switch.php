<?php


function is($i) {
  switch ($i) {
    case 12345:
      return 'first';
    case 12346:
      return 'second';
    case 12347:
      return 'third';
    case 12348:
      return 'fourth';
    default:
      return 'no';
  }
}

function ss($s) {
  switch ($s) {
    case 'nope1':
    case 'nope2':
    case 'nope3':
    case 'nope4':
    case 'nope5':
    case 'nope6':
    case 'nope7':
    default:
      return 'nope';

    case 'hello there':
      return 'hi!';
  }
}

var_dump(is(12346));
var_dump(ss('hello there'));
