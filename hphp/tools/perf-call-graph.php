#!/usr/local/bin/php -j
<?php

# If $func looks like a mangled C++ symbol, attempt to demangle it, stripping
# off any trailing junk first.
function filter_func(string $func): string {
  static $cache = Map {};

  if (substr($func, 0, 2) === '_Z') {
    if (preg_match('/^(.+)\.isra\.\d+$/', $func, $matches) === 1) {
      $func = $matches[1];
    }
    if (!isset($cache[$func])) {
      $cache[$func] = trim(shell_exec("echo '$func' | c++filt"));
    }
    $func = $cache[$func];
  }

  return $func;
}

# Returns true iff $sample contains a line containing $func.
function contains_frame(Vector $sample, string $func): bool {
  foreach ($sample as $frame) {
    if (strstr($frame, $func) !== false) return true;
  }
  return false;
}

# Node is used to construct to call graph. It contains an inclusive count of
# samples for itself and all of its children, along with a map from function
# names to children.
class Node {
  private $kids = Map {};

  public function __construct(private $name, private $count = 0) {}

  # Add an edge from this function to $name, increasing the count of the child
  # Node by one and returning it.
  public function followEdge($name) {
    if (!isset($this->kids[$name])) $this->kids[$name] = new Node($name);

    $new_node = $this->kids[$name];
    ++$new_node->count;
    return $new_node;
  }

  # Print out the current node and all children.
  public function show(
    $total_count = null,
    $indent = '',
  ) {
    if ($total_count === null) {
      # We're the parent node and don't contain a useful name.
      $total_count = $this->count;
    } else {
      # Prune any subtrees that are <0.5% of the total to keep the output
      # readable.
      if ($this->count / $total_count < 0.005) return;
      printf("%s%.1f%% %s\n",
             $indent,
             $this->count / $total_count * 100,
             $this->name);
      $indent .= '- ';
    }

    $items = $this->kids->items()->toVector();
    if ($items->count() == 0) return;

    # If our only printable child is the body of this function, leave it out to
    # keep the results clean.
    if ($items[0][0] == '<body>' &&
        ($items->count() == 1 || $items[1][1]->count / $total_count < 0.005)) {
      return;
    }

    usort($items, ($a, $b) ==> $b[1]->count - $a[1]->count);
    foreach ($items as $pair) {
      $pair[1]->show($total_count, $indent);
    }
  }
}

# Build and print a perf-annotated call graph of each stack trace in $samples,
# truncated at the highest frame containing $top.
function treeify($samples, $top) {
  $root = new Node('', $samples->count());

  foreach ($samples as $stack) {
    for ($i = $stack->count() - 1; $i >= 0; --$i) {
      if (strpos($stack[$i], $top) !== false) break;
    }

    $node = $root;
    for (; $i >= 0; --$i) {
      $node = $node->followEdge($stack[$i]);
    }
    # Add a final entry for exclusive time spent in the body of the bottom
    # function.
    $node->followEdge('<body>');
  }

  $root->show();
}

function usage($script_name) {
  echo <<<EOT
Usage:

$script_name [symbol]

This script expects the output of "perf script -f ip,sym" on stdin. If the
optional symbol argument is present, a call graph of all frames containing that
symbol will be output, with the root of the frame truncated at the highest
frame containing the symbol. Note that the symbol may appear anywhere in the
function name, so using 'Foo::translate' will match both 'Foo::translate' and
'Foo::translateFrob'. If you just want Foo::translate, use 'Foo::translate('.
If symbol is not present, the total number of samples present will be printed
along with the number of samples that contain a few hardcoded functions.


EOT;
}

function main($argv) {
  ini_set('memory_limit', '64G');
  if (posix_isatty(STDIN)) {
    usage($argv[0]);
    return 1;
  }

  $samples = Vector {};
  $stack = null;

  while ($line = fgets(STDIN)) {
    $line = trim($line);
    if ($line === '' || $line[0] == '#') {
      if ($stack) {
        $samples[] = $stack;
        $stack = null;
      }
      continue;
    }

    if (preg_match('/^[a-f0-9]+ (.+)$/', $line, $matches) === 1) {
      if (!$stack) $stack = Vector {};
      $stack[] = filter_func($matches[1]);
    } else {
      echo "Unknown line '$line'\n";
    }
  }

  if ($stack) $samples[] = $stack;

  if (count($argv) == 2) {
    $functions = Set { $argv[1] };
  } else {
    $functions = Set {
      'MCGenerator::translate',
      'jit::selectTracelet(',
      'Translator::translateRegion(',
      'jit::optimizeRefcounts(',
      'jit::optimize(',
      'jit::allocateRegs(',
      'jit::genCode(',
      'getStackValue(',
      'IRBuilder::constrain',
      'relaxGuards(',
      'MCGenerator::retranslateOpt(',
      'IRTranslator::translateInstr(',
      'Type::subtypeOf(',
      'jit::regionizeFunc',
    };
  }

  $subsamples = Map {'all' => $samples};
  foreach ($functions as $f) {
    $subsamples[$f] = $samples->filter($s ==> contains_frame($s, $f));
  }

  if (count($argv) == 2) {
    $sub = $subsamples[$argv[1]];
    printf("Looking for pattern *%s*. %d of %d total samples (%.2f%%)\n\n",
           $argv[1], $sub->count(), $samples->count(),
           $sub->count() / $samples->count() * 100);
    treeify($subsamples[$argv[1]], $argv[1]);
  } else {
    foreach ($subsamples as $k => $v) {
      printf("%5d %s\n", $v->count(), $k);
    }
  }

  return 0;
}

exit(main($argv));
