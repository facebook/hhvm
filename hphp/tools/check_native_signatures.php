<?hh
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2014      Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
/*
A helper script to check that HNI signatures in a PHP file match how the
functions are defined in the C++ file.

Currently only matches functions, so won't match classes/methods.
*/
// expects argv[1] to be the C++ file, argv[2] to be the PHP file
if (empty($_SERVER['argv'][2])) {
  fwrite(STDERR, "Usage: {$_SERVER['argv'][0]} <extfile.cpp> <extfile.php>\n");
  exit(1);
}

function parse_php_functions(string $file):
                           ConstMap<string, Pair<string, ConstVector<string>>> {
  $source = file_get_contents($file);
  if (!$source) {
    return ImmMap {};
  }

  // Don't handle methods yet, so function can't be indented
  static $function_regex =
    "#<<[^>]*__Native([^>]*)>>\nfunction +([^(]*)\(([^)]+)\) *: *(.+?);#m";

  $functions = Map {};

  if (preg_match_all($function_regex, $source, $matches, PREG_SET_ORDER)) {
    foreach($matches as $match) {
      $nativeArgs = $match[1];
      $name = $match[2];
      if (strpos($nativeArgs, '"ActRec"') !== false) {
        // ActRec functions have a specific structure
        $retType = 'actrec';
        $argTypes = Vector {'actrec'};
      } else {
        $argList = $match[3];
        $retType = explode('<', $match[4], 2)[0];
        $argTypes = Vector {};
        if ($argList) {
          $args = preg_split('/\s*,\s*/', $argList);
          if (count($args) > 5) {
            $retType = 'actrec';
            $argTypes = Vector {'actrec'};
          } else {
            foreach($args as $arg) {
              $type = preg_split('/\s*\$/', $arg)[0];
              $type = explode('<', $type, 2)[0];
              if ($type == '...') {
                // Special case varargs
                $vargTypes = Vector {'int'};
                $vargTypes->addAll($argTypes);
                $vargTypes[] = 'array';
                $argTypes = $vargTypes;
              } else {
                $argTypes[] = $type;
              }
            }
          }
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
    return ImmMap {};
  }

  // Don't handle methods yet, so function can't be indented
  static $function_regex =
            "#^(?:static )?(\S+) +HHVM_FUNCTION\(([^,)]+)(?:, *)?([^)]*)\)#m";

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
  if ($php[0] == '?') {
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
      case 'actrec':
        $expected = 'TypedValue*';
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
  if ($php[0] == '@') {
     $php = substr($php, 1);
  }
  if ($php[0] == '?') {
    $expected = 'const Variant&';
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
        $expected = 'const Array&';
        break;
      case 'resource':
        $expected = 'const Resource&';
        break;
      case 'mixed':
      case 'callable':
        $expected = 'const Variant&';
        break;
      case 'actrec':
        $expected = 'ActRec*';
        break;
      case 'object':
      default:
        $expected = 'const Object&';
        break;
    }
  }
  // References must be a variant type
  if ($php[strlen($php)-1] == '&') {
    if ($expected != 'const Variant&') {
      return false;
    } else {
      $expected = 'VRefParam';
    }
  }
  $cpp = trim($cpp);
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
