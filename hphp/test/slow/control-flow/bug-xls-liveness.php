<?hh

function strip_hh_prefix($str, $nonobject_types_only = false) {
  if (!is_string($str)) {
    return $str;
  }
  if (!is_int(stripos($str, 'HH\\'))) {
    // Bail out early if $str doesn't contain 'HH\'
    return $str;
  }
  $nonobject_types = ImmSet {
    'bool', 'boolean', 'int', 'integer', 'float', 'double', 'real', 'num',
    'string', 'resource', 'mixed', 'void', 'this', 'arraykey',
  };
  $len = strlen($str);
  $in_literal = '';
  $out = '';
  $c = ' ';
  for ($i = 0; $i < $len; ++$i) {
    $prev = $c;
    $c = $str[$i];
    if ($in_literal) {
      if ($c === '\\') {
        $out .= $c;
        ++$i;
        if ($i >= $len) break;
        $c = $str[$i];
        $out .= $c;
        continue;
      }
      if ($c === $in_literal) {
        $in_literal = '';
      }
    } else {
      if (($c === 'H' || $c === 'h') &&
          strtoupper(substr($str, $i, 3)) === "HH\\" &&
          !ctype_alnum($prev) && $prev !== '_' && $prev !== '\\') {
        if ($nonobject_types_only) {
          $sub = substr($str, $i + 3, 9);
          $sub_len = strlen($sub);
          $k = 0;
          for (; $k < $sub_len; ++$k) {
            $sub_c = $sub[$k];
            if (!ctype_alnum($sub_c) && $sub_c !== '_' && $sub_c !== '\\') {
              break;
            }
          }
          $sub = strtolower(substr($sub, 0, $k));
          $strip = ($nonobject_types->contains($sub));
        } else {
          $strip = true;
        }
        if ($strip) {
          $i += 2;
          $c = '\\';
          continue;
        }
      }
      if ($c === '\'' || $c === '"') {
        $in_literal = $c;
      }
    }
    $out .= $c;
  }
  return $out;
}

function main() {
  var_dump(strip_hh_prefix("HH\\bool", true));
  var_dump(strip_hh_prefix("HH\\boolean", true));
  var_dump(strip_hh_prefix("HH\\int", true));
  var_dump(strip_hh_prefix("HH\\integer", true));
  var_dump(strip_hh_prefix("HH\\float", true));
  var_dump(strip_hh_prefix("HH\\double", true));
  var_dump(strip_hh_prefix("HH\\real", true));
  var_dump(strip_hh_prefix("HH\\num", true));
  var_dump(strip_hh_prefix("HH\\string", true));
  var_dump(strip_hh_prefix("HH\\resource", true));
  var_dump(strip_hh_prefix("HH\\mixed", true));
  var_dump(strip_hh_prefix("HH\\void", true));
  var_dump(strip_hh_prefix("HH\\this", true));
  var_dump(strip_hh_prefix("HH\\arraykey", true));
}

main();
