<?hh

// expects argv[1] to be the C++ file, argv[2] to be the PHP file
if (empty($_SERVER['argv'][2])) {
  fwrite(STDERR, "Usage: {$_SERVER['argv'][0]} <extfile.cpp> <extfile.php>\n");
  exit(1);
}

function parse_php_functions(string $file):
                           ConstMap<string, Pair<string, ConstVector<string>>> {
  $source = file_get_contents($file);
  if (!$source) {
    return FrozenMap {};
  }
  
  // Don't handle methods yet, so function can't be indented
  static $function_regex =
           "#<<[^>]*__Native[^>]*>>\nfunction +([^(]+)\(([^)]+)\) *: *(.+?);#m";

  $functions = Map {};

  if (preg_match_all($function_regex, $source, $matches, PREG_SET_ORDER)) {
    foreach($matches as $match) {
      $name = $match[1];
      $argList = $match[2];
      $retType = explode('<', $match[3], 2)[0];
      $argTypes = Vector {};
      if ($argList) {
        $args = preg_split('/\s*,\s*/', $argList);
        foreach($args as $arg) {
          $type = preg_split('/\s*\$/', $arg)[0];
          $type = explode('<', $type, 2)[0];
          $argTypes[] = $type;
        }
      }
      $functions[$name] = Pair { $retType, $argTypes };
    }
  }
  return $functions;
}

function parse_cpp_functions(string $file):
                           ConstMap<string, Pair<string, ConstVector<string>>> {
  $source = file_get_contents($file);
  if (!$source) {
    return FrozenMap {};
  }
  
  // Don't handle methods yet, so function can't be indented
  static $function_regex =
            "#^(\S+) +HHVM_FUNCTION\(([^,)]+)(?:, *)?([^)]*)\)#m";

  $functions = Map {};

  if (preg_match_all($function_regex, $source, $matches, PREG_SET_ORDER)) {
    foreach($matches as $match) {
      $name = $match[2];
      $argList = $match[3];
      $retType = $match[1];
      $argTypes = Vector {};
      if ($argList) {
        $args = preg_split('/\s*,\s*/', $argList);
        foreach($args as $arg) {
          $type = preg_split('# */ *#', $arg)[0];
          $type = implode(' ', explode(' ', $type, -1));
          $argTypes[] = $type;
        }
      }
      $functions[$name] = Pair { $retType, $argTypes };
    }
  }
  return $functions;
}

function match_return_type(string $php, string $cpp): bool {
  if ($php[0] == '?' || $php[0] == '@') {
    $expected = 'Variant';
  } else {
    switch (strtolower($php)) {
      case 'bool':
      case 'boolean':
        $expected = 'bool';
        break;
      case 'int':
      case 'long':
        $expected = 'int64_t';
        break;
      case 'float':
      case 'double':
        $expected = 'double';
        break;
      case 'void':
        $expected = 'void';
        break;
      case 'string':
        $expected = 'String';
        break;
      case 'array':
        $expected = 'Array';
        break;
      case 'resource':
        $expected = 'Resource';
        break;
      case 'mixed':
      case 'callable':
        $expected = 'Variant';
        break;
      case 'object':
      default:
        $expected = 'Object';
        break;
    }
  }
  // Special case for ints
  if ($cpp == 'int') {
    $cpp = 'int64_t';
  }
  return $cpp == $expected;
}

function match_arg_type(string $php, string $cpp): bool {
  if($php[strlen($php)-1] == '&') {
    $expected = 'VRefParam';
  } else if ($php[0] == '?' || $php[0] == '@') {
    $expected = 'CVarRef';
  } else {
    switch (strtolower($php)) {
      case 'bool':
      case 'boolean':
        $expected = 'bool';
        break;
      case 'int':
      case 'long':
        $expected = 'int64_t';
        break;
      case 'float':
      case 'double':
        $expected = 'double';
        break;
      case 'void':
        // Shouldn't have void as an argument type
        return false;
      case 'string':
        $expected = 'const String&';
        break;
      case 'array':
        $expected = 'CArrRef';
        break;
      case 'resource':
        $expected = 'CResRef';
        break;
      case 'mixed':
      case 'callable':
        $expected = 'CVarRef';
        break;
      case 'object':
      default:
        $expected = 'CObjRef';
        break;
    }
  }
  // Special case for ints
  if ($cpp == 'int') {
    $cpp = 'int64_t';
  }
  return $cpp == $expected;
}

function check_types(ConstMap<string, Pair<string, ConstVector<string>>> $php,
                     ConstMap<string, Pair<string, ConstVector<string>>> $cpp):
                                                                          bool {
  $errored = false;
  foreach($php as $name => $types) {
    if (!isset($cpp[$name])) {
      $errored = true;
      printf("Unimplemented native function '%s'\n", $name);
      continue;
    }
    $cppTypes = $cpp[$name];
    if (!match_return_type($types[0], $cppTypes[0])) {
      $errored = true;
      printf("Mismatched return type for function '%s'. PHP: %s C++: %s\n",
              $name, $types[0], $cppTypes[0]);
    }
    if ($types[1]->count() != $cppTypes[1]->count()) {
      $errored = true;
      printf("Unequal number of arguments for function '%s'\n", $name);
      continue;
    }
    foreach($types[1] as $idx => $t) {
      if (!match_arg_type($t, $cppTypes[1][$idx])) {
        $errored = true;
        printf("Mismatched argument type for function '%s' at index '%d'."
                . " PHP: %s C++: %s\n", $name, $idx, $t, $cppTypes[1][$idx]);
      }
    }
  }
  return $errored;
}

$phpFuncs = parse_php_functions($_SERVER['argv'][2]);
$cppFuncs = parse_cpp_functions($_SERVER['argv'][1]);

if (check_types($phpFuncs, $cppFuncs)) {
  echo "See https://github.com/facebook/hhvm/wiki/Extension-API for what types",
        " map to what\n";
}
