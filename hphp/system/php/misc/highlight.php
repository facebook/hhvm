<?php

function highlight_file($filename, $return = false) {
  $data = file_get_contents($filename);
  if ($data === false) {
    return '';
  }

  return highlight_string($data, $return);
}

function highlight_string($data, $return = false) {
  $colors = [
    'html' => '#000000',
    'comment' => '#FF8000',
    'default' => '#0000BB',
    'string' => '#DD0000',
    'keyword' => '#007700',
  ];

  $output = '';
  $last_color = $colors['html'];
  $next_color = null;

  $output .= '<code><span style="color: '.$last_color."\">\n";

  foreach (token_get_all($data) as $token) {
    if (is_array($token)) {
      list($type, $string, $_) = $token;

      if ($type === T_WHITESPACE) {
        $output .= __HPHP_highlight_html_puts($string);
        continue;
      }

      $next_color = __HPHP_highlight_get_color($colors, $type);
    } else {
      $string = $token;
      $next_color = $colors['keyword'];
    }

    if ($last_color != $next_color) {
      if ($last_color != $colors['html']) {
        $output .= '</span>';
      }
      $last_color = $next_color;
      if ($last_color != $colors['html']) {
        $output .= '<span style="color: '.$last_color.'">';
      }
    }

    $output .= __HPHP_highlight_html_puts($string);
  }

  if ($last_color != $colors['html']) {
    $output .= "</span>\n";
  }
  $output .= "</span>\n</code>";

  if ($return) {
    return $output;
  } else {
    echo $output;
  }
}

function __HPHP_highlight_get_color($colors, $type) {
  switch ($type) {
    case T_INLINE_HTML:
      return $colors['html'];

    case T_COMMENT:
    case T_DOC_COMMENT:
      return $colors['comment'];

    case T_OPEN_TAG:
    case T_OPEN_TAG_WITH_ECHO:
      return $colors['default'];

    case T_CLOSE_TAG:
      return $colors['default'];

    case T_ENCAPSED_AND_WHITESPACE:
    case T_CONSTANT_ENCAPSED_STRING:
      return $colors['string'];

    case T_STRING:
    case T_VARIABLE:
    case T_DNUMBER:
    case T_LNUMBER:
      return $colors['default'];

    default:
      return $colors['keyword'];
  }
}

function __HPHP_highlight_html_puts($s) {
  $len = strlen($s);
  $out = '';
  for ($i = 0; $i < $len; $i++) {
    switch ($s[$i]) {
      case "\n":
        $out .= '<br />';
        break;
      case '<':
        $out .= '&lt;';
        break;
      case '>':
        $out .= '&gt;';
        break;
      case '&':
        $out .= '&amp;';
        break;
      case ' ':
        $out .= '&nbsp;';
        break;
      case "\t":
        $out .= '&nbsp;&nbsp;&nbsp;&nbsp;';
        break;
      default:
        $out .= $s[$i];
        break;
    }
  }
  return $out;
}

