<?hh

const TPATTERN = "/pattern/";
const TBACKTRACK_PATTERN = "/.*\p{N}/";
const TINPUT = '0123456789';

function test_fn($name, $base, $with_error) {
  echo "=== $name ===\n";

  echo "- no error -\n";
  $base(TPATTERN);
  var_dump(preg_last_error() === PREG_NO_ERROR);
  $error = null;
  $with_error(TPATTERN, inout $error);
  var_dump($error === null);

  echo "- error induced -\n";
  $base(TBACKTRACK_PATTERN);
  var_dump(preg_last_error() === PREG_BACKTRACK_LIMIT_ERROR);
  $error = null;
  $with_error(TBACKTRACK_PATTERN, inout $error);
  var_dump($error === PREG_BACKTRACK_LIMIT_ERROR);

  echo "- error state preserved -\n";
  $base(TPATTERN);
  var_dump(preg_last_error() === PREG_NO_ERROR);
  $error = null;
  $with_error(TBACKTRACK_PATTERN, inout $error);
  var_dump($error === PREG_BACKTRACK_LIMIT_ERROR);
  var_dump(preg_last_error() === PREG_NO_ERROR);
}

function test_preg_grep() {
  test_fn(
    'preg_grep',
    $p ==> preg_grep($p, varray[TINPUT]),
    ($p, inout $e) ==> preg_grep_with_error($p, varray[TINPUT], inout $e),
  );
}

function test_preg_match() {
  test_fn(
    'preg_match',
    $p ==> preg_match($p, TINPUT),
    ($p, inout $e) ==> preg_match_with_error($p, TINPUT, inout $e),
  );
}

function test_preg_match_with_matches() {
  $_m = null;
  test_fn(
    'preg_match_with_matches',
    $p ==> preg_match_with_matches($p, TINPUT, inout $_m),
    ($p, inout $e) ==>
      preg_match_with_matches_and_error($p, TINPUT, inout $_m, inout $e),
  );
}

function test_preg_match_all() {
  test_fn(
    'preg_match_all',
    $p ==> preg_match_all($p, TINPUT),
    ($p, inout $e) ==> preg_match_all_with_error($p, TINPUT, inout $e),
  );
}

function test_preg_match_all_with_matches() {
  $_m = null;
  test_fn(
    'preg_match_all_with_matches',
    $p ==> preg_match_all_with_matches($p, TINPUT, inout $_m),
    ($p, inout $e) ==>
      preg_match_all_with_matches_and_error($p, TINPUT, inout $_m, inout $e),
  );
}

function test_preg_replace() {
  test_fn(
    'preg_replace',
    $p ==> preg_replace($p, 'asdf', TINPUT),
    ($p, inout $e) ==> preg_replace_with_error($p, 'asdf', TINPUT, inout $e),
  );
}

function test_preg_replace_with_count() {
  $_c = null;
  test_fn(
    'preg_replace_with_count',
    $p ==> preg_replace_with_count($p, 'asdf', TINPUT, 123, inout $_c),
    ($p, inout $e) ==> preg_replace_with_count_and_error(
      $p,
      'asdf',
      TINPUT,
      123,
      inout $_c,
      inout $e,
    ),
  );
}

function test_preg_split() {
  test_fn(
    'preg_split',
    $p ==> preg_split($p, TINPUT),
    ($p, inout $e) ==> preg_split_with_error($p, TINPUT, inout $e),
  );
}

<<__EntryPoint>>
function main_preg_last_error() {
  // Set low backtrack limit to trigger PREG_BACKTRACK_LIMIT_ERROR
  ini_set('pcre.backtrack_limit', 1);

  test_preg_grep();
  test_preg_match();
  test_preg_match_with_matches();
  test_preg_match_all();
  test_preg_match_all_with_matches();
  test_preg_replace();
  test_preg_replace_with_count();
  test_preg_split();
}
