<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_preg_match() :mixed{
  echo "====================== preg_match ============================\n";

  $m = null;
  preg_match_all_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s\*(?:=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_SET_ORDER,
  );
  var_dump($m);

  preg_match_all_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s\*(?:=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_SET_ORDER | PREG_OFFSET_CAPTURE,
  );
  var_dump($m);

  preg_match_all_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s\*(?:=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_OFFSET_CAPTURE,
  );
  var_dump($m);

  preg_match_all_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s\*(?:=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_PATTERN_ORDER,
  );
  var_dump($m);

  preg_match_all_with_matches(
    '/((?P<foobaz1>(?:unsigned|struct)\s+)?\w+)(?P<foobaz2>\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\\[\s*\w*\s*\\])?\s*(?:=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_PATTERN_ORDER,
  );
  var_dump($m);

  preg_match_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s*(?:\=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    0,
  );
  var_dump($m);

  preg_match_with_matches(
    '/((?:(?:unsigned|struct)\s+)?\w+)(?:\s*(\*+)\s+|\s+(\**))(\w+(?:\[\s*\w*\s*\])?)\s*(?:(=)[^,;]+)?((?:\s*,\s*\**\s*\w+(?:\[\s*\w*\s*\\])?\s*(?:\=[^,;]+)?)*)\s*;/S',
    'unsigned int xpto = 124; short a, b;',
    inout $m,
    PREG_OFFSET_CAPTURE,
  );
  var_dump($m);
}

function test_preg_grep() :mixed{
  echo "====================== preg_grep =============================\n";
  $strs1 = vec['a', '1', 'q6', 'h20'];
  $strs2 = dict['key1' => 'a', 'key2' =>'1', 'key3' => 'q6', 'key4' => 'h20'];
  var_dump(preg_grep('/^(\d|.\d)$/', $strs1, 0));
  var_dump(preg_grep('/^(\d|.\d)$/', $strs1, PREG_GREP_INVERT));
  var_dump(preg_grep('/^(\d|.\d)$/', $strs2, 0));
  var_dump(preg_grep('/^(\d|.\d)$/', $strs2, PREG_GREP_INVERT));
}

function test_preg_split() :mixed{
  echo "====================== preg_split =============================\n";
  var_dump(preg_split("//", "string", -1, 0));
  var_dump(preg_split("//", "string", 3, 0));
  var_dump(preg_split("//", "string", -1, PREG_SPLIT_OFFSET_CAPTURE));
  var_dump(preg_split("//", "string", 3, PREG_SPLIT_OFFSET_CAPTURE));
  var_dump(preg_split("//", "string", -1, PREG_SPLIT_OFFSET_CAPTURE | PREG_SPLIT_NO_EMPTY));
  var_dump(preg_split("//", "string", 3, PREG_SPLIT_OFFSET_CAPTURE | PREG_SPLIT_NO_EMPTY));
}


<<__EntryPoint>>
function main_hack_arrays() :mixed{
test_preg_match();
test_preg_grep();
test_preg_split();

}
