<?php

function php_strip_whitespace($file) {
  $data = file_get_contents($file);
  if ($data === false) {
    return "";
  }

  $tokens = token_get_all($data);
  $output = '';
  $prev_space = false;

  for ($i = 0; $i < count($tokens); $i++) {
    $token = $tokens[$i];

    if (!is_array($token)) {
      $output .= $token;
      $prev_space = false;
      continue;
    }

    list($type, $string, $line) = $token;
    switch ($type) {
      case T_WHITESPACE:
        if (!$prev_space) {
          $output .= ' ';
          $prev_space = true;
        }
        break;

      case T_COMMENT:
      case T_DOC_COMMENT:
        // don't reset $prev_space since we didn't output anything
        break;

      case T_END_HEREDOC:
        $output .= $string;
        $next_token = $tokens[++$i];
        if (!is_array($next_token)) {
          $output .= $next_token;
        } else if ($next_token[0] != T_WHITESPACE) {
          $output .= $next_token[1];
        }
        $output .= "\n";
        $prev_space = true;
        break;

      default:
        $output .= $string;
        $prev_space = false;
    }
  }

  return $output;
}
