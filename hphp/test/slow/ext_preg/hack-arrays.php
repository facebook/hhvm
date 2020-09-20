<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_preg_match() {
  echo "====================== preg_match ============================\n";

  $m = null;
  preg_match_all_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s\*(?:=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_SET_ORDER | PREG_FB_HACK_ARRAYS,
  );
  var_dump($m);

  preg_match_all_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s\*(?:=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_SET_ORDER | PREG_OFFSET_CAPTURE | PREG_FB_HACK_ARRAYS,
  );
  var_dump($m);

  preg_match_all_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s\*(?:=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_OFFSET_CAPTURE | PREG_FB_HACK_ARRAYS,
  );
  var_dump($m);

  preg_match_all_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s\*(?:=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_PATTERN_ORDER | PREG_FB_HACK_ARRAYS,
  );
  var_dump($m);

  preg_match_all_with_matches(
    '/((?P<foobaz1>(?:unsigned|struct)\s+)?\w+)(?P<foobaz2>\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\\[\s*\w*\s*\\])?\s*(?:=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_PATTERN_ORDER | PREG_FB_HACK_ARRAYS,
  );
  var_dump($m);

  preg_match_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s*(?:\=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_FB_HACK_ARRAYS,
  );
  var_dump($m);

  preg_match_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s*(?:\=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_OFFSET_CAPTURE | PREG_FB_HACK_ARRAYS,
  );
  var_dump($m);
}

function test_preg_grep() {
  echo "====================== preg_grep =============================\n";
  $strs1 = varray['a', '1', 'q6', 'h20'];
  $strs2 = darray['key1' => 'a', 'key2' =>'1', 'key3' => 'q6', 'key4' => 'h20'];
  var_dump(preg_grep('/^(\d|.\d)$/', $strs1, PREG_FB_HACK_ARRAYS));
  var_dump(preg_grep('/^(\d|.\d)$/', $strs1, PREG_GREP_INVERT | PREG_FB_HACK_ARRAYS));
  var_dump(preg_grep('/^(\d|.\d)$/', $strs2, PREG_FB_HACK_ARRAYS));
  var_dump(preg_grep('/^(\d|.\d)$/', $strs2, PREG_GREP_INVERT | PREG_FB_HACK_ARRAYS));
}

function test_preg_split() {
  echo "====================== preg_split =============================\n";
  var_dump(preg_split("//", "string", -1, PREG_FB_HACK_ARRAYS));
  var_dump(preg_split("//", "string", 3, PREG_FB_HACK_ARRAYS));
  var_dump(preg_split("//", "string", -1, PREG_SPLIT_OFFSET_CAPTURE | PREG_FB_HACK_ARRAYS));
  var_dump(preg_split("//", "string", 3, PREG_SPLIT_OFFSET_CAPTURE | PREG_FB_HACK_ARRAYS));
  var_dump(preg_split("//", "string", -1, PREG_SPLIT_OFFSET_CAPTURE | PREG_FB_HACK_ARRAYS | PREG_SPLIT_NO_EMPTY));
  var_dump(preg_split("//", "string", 3, PREG_SPLIT_OFFSET_CAPTURE | PREG_FB_HACK_ARRAYS | PREG_SPLIT_NO_EMPTY));
}


<<__EntryPoint>>
function main_hack_arrays() {
test_preg_match();
test_preg_grep();
test_preg_split();

var_dump(PREG_FB_HACK_ARRAYS === PREG_HACK_ARR);
}
