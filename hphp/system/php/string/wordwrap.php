<?php

function wordwrap($str, $width = 75, $break = PHP_EOL, $cut = false) {
  if ($str === null) {
    return "";
  }
  if ($str === "") {
    return "";
  }

  if ($break === "") {
    $bt = debug_backtrace();
    trigger_error("Break string cannot be empty in ".$bt[0]["file"]." on line ".
                  $bt[0]["line"], E_USER_WARNING);
    return false; // Zend returns false from a method
                  // supposed to return a string. Errr....
  }

  if ($width === 0 && $cut) {
    $bt = debug_backtrace();
    trigger_error("Can't force cut when width is zero in ".$bt[0]["file"].
                  " on line ".$bt[0]["line"], E_USER_WARNING);
    return false;
  }

  $str_len = strlen($str);
  $break_len = strlen($break);

  $ret = "";
  $last_start = 0;
  $last_space = 0;

  for ($current = 0; $current < $str_len; $current++) {
    $cur_char = $str[$current];
    if ($break_len === 1) {
      $check_break = $cur_char;
    } else {
      $check_break = substr($str, $current, $break_len);
    }
    // when we hit a break, copy to our return value and get laststart
    // and lastspace in order
    if ($check_break === $break) {
      $ret .= substr($str, $last_start, $current - $last_start + $break_len);
      $current += $break_len - 1;
      $last_start = $current + 1;
      $last_space = $current + 1;
      continue;
    }
    // If we hit a space, first decide if we are at a line break boundary. If
    // so, put in our return value and insert a break. Otherwise move on.
    if ($cur_char === ' ') {
      if ($current - $last_start >= $width) {
        $ret .= substr($str, $last_start, $current - $last_start).$break;
        $last_start = $current + 1;
      }
      $last_space = $current;
      continue;
    }
    // If cut is true, and we have seen width chars already without a space,
    // add to return value and break
    if (($current - $last_start) >= $width && $cut &&
        $last_start >= $last_space) {
      $ret .= substr($str, $last_start, $current - $last_start).$break;
      $last_start = $current;
      $last_space = $current;
      continue;
    }
    // If current char puts us over the width, put in return value, backup to
    // last space, insert break, and move back to laststart
    if ($current - $last_start >= $width && $last_start < $last_space) {
      $ret .= substr($str, $last_start, $last_space - $last_start).$break;
      $last_start = $last_space + 1;
      $last_space = $last_space + 1;
      continue;
    }
  }

  // stragglers
  if ($last_start !== $current) {
    $ret .= substr($str, $last_start, $current - $last_start);
  }

  return $ret;
}
