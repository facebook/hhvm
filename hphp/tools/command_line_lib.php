<?hh
/*
 * Contains some reusable utilities for command line php scripts.
 */

require_once(__DIR__.'/command_line_lib_UNSAFE.php');

function error(string $message): void {
  error_unsafe($message);
}

//////////////////////////////////////////////////////////////////////
/*
 * Option parsing.
 *
 * Fill out a OptionInfoMap and then call parse_options($map).  It
 * returns a Map<string,mixed>, where the mixed is false for flag
 * options or the value of the option for options that take arguments.
 *
 * The value of $GLOBALS['argv'] is shifted to reflect the consumed
 * options.
 *
 * Example:
 *
 *  function main(): void {
 *    $optmap = Map {
 *      'long-name'   => Pair { 'l', 'help message' },
 *      'with-arg:'   => Pair { 'a', 'with required argument' },
 *      'with-opt::'  => Pair { '',  'with optional argument' },
 *      'def-opt::12' => Pair { '',  'with defaulted argument' },
 *      'help'        => Pair { 'h', 'display help' },
 *      'long-other'  => Pair { '',  'this has no short version' },
 *    };
 *    $opts = parse_options($optmap);
 *    if ($opts->containsKey('help')) {
 *      return display_help(
 *        "String that goes ahead of generic help message",
 *        $optmap,
 *      );
 *    }
 *  }
 *
 *
 * Rationale:
 *
 *   Apparently php's getopt() builtin is a pile.
 *
 */

type OptionInfo    = Pair<string,string>;
type OptionInfoMap = Map<string,OptionInfo>;
type OptionMap     = Map<string,mixed>;

function parse_options(OptionInfoMap $optmap): OptionMap {
  return parse_options_UNSAFE($optmap);
}

function remove_argument(array<string> &$argv, $index) {
  if ($index < 0 || $index >= count($argv)) {
    return;
  }

  unset($argv[$index]);
  $argv = array_values($argv);
}

function parse_options_impl(OptionInfoMap $optmap, array<string> &$argv): OptionMap {
  $short_to_long     = Map {};
  $long_to_default   = Map {};
  $long_supports_arg = Map {};
  $long_requires_arg = Map {};
  $long_set_arg      = Map {};
  $all_longs         = Map {};

  foreach ($optmap as $k => $v) {
    $m = null;
    if (preg_match('/^([^:]*)(\[\])/', $k, $m)) {
      invariant($m !== null, "Regex must return match!");
      $k = $m[1];
      $all_longs[$k] = true;
      $long_supports_arg[$k] = true;
      $long_requires_arg[$k] = true;
      $long_set_arg[$k] = true;
    } else if (preg_match('/^([^:]*)(:(:(.*))?)?/', $k, $m)) {
      invariant($m !== null, "Regex must return match!");
      $k = $m[1];
      $all_longs[$k] = true;
      $long_supports_arg[$k] = isset($m[2]);
      $long_requires_arg[$k] = isset($m[2]) && !isset($m[3]);
      if (isset($m[4])) {
        $long_to_default[$k] = $m[4];
      } else {
        $long_to_default[$k] = false;
      }
      $long_set_arg[$k] = false;

      if ($v[0] != '') {
        $short_to_long[$v[0]] = $k;
      }
    } else {
      error("couldn't understand option map format");
    }
  }

  $ret = Map {};

  array_shift($argv);
  $pos_args_count = 0;
  while (count($argv) - $pos_args_count > 0) {
    $arg = $argv[$pos_args_count];

    if ($arg == "--") {
      remove_argument($argv, $pos_args_count);
      break;
    }

    // Helper to try to read an argument for an option.
    $read_argument = function($long) use (&$argv,
                                           $pos_args_count,
                                           $long_supports_arg,
                                           $long_requires_arg,
                                           $long_to_default,
                                           $long_set_arg) {
      if (!$long_supports_arg[$long]) error("precondition");
      if ($long_requires_arg[$long] || $long_set_arg[$long]) {
        remove_argument($argv, $pos_args_count);
        if (count($argv) - $pos_args_count == 0) {
          error("option --$long requires an argument");
        }
      } else {
        if (count($argv) - $pos_args_count == 0 ||
            $argv[$pos_args_count + 1][0] == '-') {
          return $long_to_default[$long];
        }
        remove_argument($argv, $pos_args_count);
      }

      return $argv[$pos_args_count];
    };

    // Returns whether a given option is recognized at all.
    $opt_exists = function($opt) use ($all_longs) {
      return $all_longs->containsKey($opt);
    };

    // Long-style arguments.
    $m = null;
    if (preg_match('/^--([^=]*)(=(.*))?/', $arg, $m)) {
      assert($m);
      $long = $m[1];
      $has_val = !empty($m[3]);
      $val = $has_val ? $m[3] : false;

      if (isset($m[2]) && !$has_val) {
        error("option --$long had an equal sign with no value");
      }
      if (!$opt_exists($long)) {
        error("unrecognized option --$long");
      }
      if ($has_val && !$long_supports_arg[$long]) {
        error("option --$long does not take an argument");
      }
      if (!$has_val && $long_supports_arg[$long]) {
        $val = $read_argument($long);
      }

      if ($long_set_arg[$long]) {
        if (!$ret->containsKey($long)) {
          $ret[$long] = new Set(null);
        }
        $ret[$long]->add($val);
      } else {
        $ret[$long] = $val;
      }
      remove_argument($argv, $pos_args_count);
      continue;
    }

    // Short-style arguments
    $m = null;
    if (preg_match('/^-([^-=]*)(=(.*))?/', $arg, $m)) {
      assert($m);
      $shorts = $m[1];
      $has_val = !empty($m[3]);
      $val = $has_val ? $m[3] : false;

      if (isset($m[2]) && !$has_val) {
        error("option -$shorts had an equal sign with no value");
      }

      if (!$has_val && strlen($shorts) > 1) {
        // Support mashed together short flags.  Only allowed when
        // there's no arguments.
        foreach (str_split($shorts) as $s) {
          if (!$short_to_long->containsKey($s)) {
            error("unrecognized option -$s");
          }
          $long = $short_to_long[$s];
          if ($long_requires_arg[$long]) {
            error("option -$s requres an argument");
          }
          $ret[$short_to_long[$s]] = $long_to_default[$long];
        }
        remove_argument($argv, $pos_args_count);
        continue;
      }

      $s = $shorts[0];
      if (!$short_to_long->containsKey($s)) {
        error("unrecognized option -$s");
      }
      $long = $short_to_long[$s];
      if ($has_val && !$long_supports_arg[$long]) {
        error("option -$s does not take an argument");
      }
      if (!$has_val && $long_supports_arg[$long]) {
        $val = $read_argument($long);
      }

      $ret[$long] = $val;
      remove_argument($argv, $pos_args_count);
      continue;
    }

    // Positional argument, presumably.
    $pos_args_count++;
  }

  return $ret;
}

function display_help(string $message, OptionInfoMap $optmap): void {
  echo $message . "\n";
  echo "Options:\n\n";

  $first_cols = Map {};
  foreach ($optmap as $long => $info) {
    $has_arg = false;
    $has_opt = false;
    $vis = $long;
    if (substr($long, -2) == '::') {
      $has_opt = true;
      $vis = substr($long, 0, -2);
    }
    elseif (substr($long, -1) == ':') {
      $has_arg = true;
      $vis = substr($long, 0, -1);
    }
    $vis = preg_replace('/::/', '=', $vis);

    $first_cols[$long] =
      $info[0] != ''
        ? '-'.$info[0].'  --'.$vis
        : '    --'.$vis
        ;
    if ($has_arg) {
      $first_cols[$long] .= '=arg';
    }
    elseif ($has_opt) {
      $first_cols[$long] .= '[=optarg]';
    }
  }

  $longest_col = max($first_cols->values()->map(fun('strlen'))->toArray());

  foreach ($first_cols as $long => $col) {
    $pad = str_repeat(' ', $longest_col - strlen($col) + 5);
    echo "    ".$col.$pad.$optmap[$long][1]."\n";
  }
  echo "\n";
}

//////////////////////////////////////////////////////////////////////
