<?hh
/*
 * Contains some reusable utilities for command line php scripts.
 */

//////////////////////////////////////////////////////////////////////
/*
 * General utilities.
 */

$saved_argv0 = $GLOBALS['argv'][0];
function error(string $message): void {
  global $saved_argv0;
  echo "$saved_argv0: $message\n";
  exit(1);
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
type OptionInfoMap = Map<string,OptInfo>;
type OptionMap     = Map<string,mixed>;

function parse_options(OptionInfoMap $optmap): OptionMap {
  $short_to_long     = Map {};
  $long_to_default   = Map {};
  $long_supports_arg = Map {};
  $long_requres_arg  = Map {};
  $all_longs         = Map {};

  foreach ($optmap as $k => $v) {
    $m = null;
    if (preg_match('/^([^:]*)(:(:(.*))?)?/', $k, $m)) {
      $k = $m[1];
      $all_longs[$k] = true;
      $long_supports_arg[$k] = isset($m[2]);
      $long_requires_arg[$k] = isset($m[2]) && !isset($m[3]);
      if (isset($m[4])) {
        $long_to_default[$k] = $m[4];
      } else {
        $long_to_default[$k] = false;
      }

      if ($v[0] != '') {
        $short_to_long[$v[0]] = $k;
      }
    } else {
      error("couldn't understand option map format");
    }
  }

  $ret = Map {};

  global $argv;
  array_shift($argv);
  while (count($argv) > 0) {
    $arg = $argv[0];

    if ($arg == "--") {
      array_shift($argv);
      break;
    }

    // Helper to try to read an argument for an option.
    $read_argument = function($long) use (&$argv,
                                           $long_supports_arg,
                                           $long_requires_arg,
                                           $long_to_default) {
      if (!$long_supports_arg[$long]) error("precondition");
      if ($long_requires_arg[$long]) {
        array_shift($argv);
        if (count($argv) == 0) {
          error("option --$long requires an argument");
        }
      } else {
        if (count($argv) < 1 || $argv[1][0] == '-') {
          return $long_to_default[$long];
        }
        array_shift($argv);
      }

      return $argv[0];
    };

    // Returns whether a given option is recognized at all.
    $opt_exists = function($opt) use ($all_longs) {
      return $all_longs->containsKey($opt);
    };

    // Long-style arguments.
    $m = null;
    if (preg_match('/^--([^=]*)(=(.*))?/', $arg, $m)) {
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

      $ret[$long] = $val;
      array_shift($argv);
      continue;
    }

    // Short-style arguments
    $m = null;
    if (preg_match('/^-([^-=]*)(=(.*))?/', $arg, $m)) {
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
        array_shift($argv);
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
      array_shift($argv);
      continue;
    }

    // Positional argument, presumably.
    break;
  }

  return $ret;
}

function display_help(string $message, OptionInfoMap $optmap): void {
  echo $message . "\n";
  echo "Options:\n\n";

  $first_cols = Map {};
  foreach ($optmap as $long => $info) {
    $has_arg = false;
    $vis = $long;
    if (substr($long, -1) == ':') {
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
  }

  $longest_col = max($first_cols->values()->map('strlen')->toArray());

  foreach ($first_cols as $long => $col) {
    $pad = str_repeat(' ', $longest_col - strlen($col) + 5);
    echo "    ".$col.$pad.$optmap[$long][1]."\n";
  }
  echo "\n";
}

//////////////////////////////////////////////////////////////////////
