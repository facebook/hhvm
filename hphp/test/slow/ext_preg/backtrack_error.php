<?hh


<<__EntryPoint>>
function main_backtrack_error() :mixed{
$re = '{^(\s*\{\s*(?:"(?:\\\\["bfnrt/\\\\]|\\\\u[a-fA-F0-9]{4}|[^\0-\x09\x0a-\x1f\\\\"])*"\s*:\s*(?:[0-9.]+|null|true|false|"(?:\\\\["bfnrt/\\\\]|\\\\u[a-fA-F0-9]{4}|[^\0-\x09\x0a-\x1f\\\\"])*"|\[[^\]]*\]|\{(?:[^{}]*|\{(?:[^{}]*|\{(?:[^{}]*|\{(?:[^{}]*|\{[^{}]*\})*\})*\})*\})*\})\s*,\s*)*?)("require-dev"\s*:\s*(?:[0-9.]+|null|true|false|"(?:\\\\["bfnrt/\\\\]|\\\\u[a-fA-F0-9]{4}|[^\0-\x09\x0a-\x1f\\\\"])*"|\[[^\]]*\]|\{(?:[^{}]*|\{(?:[^{}]*|\{(?:[^{}]*|\{(?:[^{}]*|\{[^{}]*\})*\})*\})*\})*\}))(.*)}s';

$pass = '{
    "require": {
        "php": ">=5.3.3"
    },
    "require-dev": {
        "satooshi/php-coveralls": "dev-master"
    }
}';

$fail = '{
    "require": {
        "php": ">=5.3.3"
    }
}';

  $match = null;
  $error = null;
  $count = preg_match_with_matches_and_error($re, $pass, inout $match, inout $error);
  var_dump($count, $error === PREG_BACKTRACK_LIMIT_ERROR);

  $error = null;
  $count = preg_match_with_matches_and_error($re, $fail, inout $match, inout $error);
  var_dump($count, $error === PREG_BACKTRACK_LIMIT_ERROR);

  $error = null;
  $count = preg_replace_with_error($re, '', $fail, inout $error);
  var_dump($count, $error === PREG_BACKTRACK_LIMIT_ERROR);

  $error = null;
  $count = preg_replace_with_error($re, '', $pass, inout $error);
  var_dump($count, $error === PREG_BACKTRACK_LIMIT_ERROR);

  $error = null;
  $count = preg_split_with_error($re, $fail, inout $error);
  var_dump($count, $error === PREG_BACKTRACK_LIMIT_ERROR);

  $error = null;
  $count = preg_split_with_error($re, $pass, inout $error);
  var_dump($count, $error === PREG_BACKTRACK_LIMIT_ERROR);
}
