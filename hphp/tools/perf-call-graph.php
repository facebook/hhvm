#!/usr/local/bin/php -j
<?php

require(__DIR__.'/perf-lib.php');

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

    usort(&$items, ($a, $b) ==> $b[1]->count - $a[1]->count);
    foreach ($items as $pair) {
      $pair[1]->show($total_count, $indent);
    }
  }
}

# Build and print a perf-annotated call graph of each stack trace in $samples,
# truncated at the highest frame containing $top.
function treeify(Vector $samples, bool $reverse, ?string $top) {
  $root = new Node('', $samples->count());
  $inc = $reverse ? 1 : -1;

  foreach ($samples as $stack) {
    $begin = $reverse ? 0 : $stack->count() - 1;
    $end = $reverse ? $stack->count() : -1;

    $i = $begin;
    if ($top !== null) {
      for (; $i !== $end; $i += $inc) {
        if (strpos($stack[$i], $top) !== false) break;
      }
    }

    $node = $root;
    for (; $i !== $end; $i += $inc) {
      $func = $stack[$i];
      if ($func === 'HHVM::retInlHelper') continue;
      $func = preg_replace('/^PHP::.+\.php::/', 'PHP::', $func);

      $node = $node->followEdge($func);
    }
    if (!$reverse) {
      # Add a final entry for exclusive time spent in the body of the bottom
      # function.
      $node->followEdge('<body>');
    }
  }

  $root->show();
}

function usage($script_name) {
  echo <<<EOT
Usage:

$script_name [--reverse] [symbol]...

This script expects the output of "perf script --fields comm,ip,sym" on stdin.
If no symbols are given, all samples will be combined into a single tree showing
the call graph, annotated with the inclusive cost of each node.

If one symbol is given, the call graph will instead be built from samples with
at least one frame containing the symbol. Samples will be truncated at the
highest frame containing the symbol. Note that the symbol may appear
anywhere in the function name, so using 'Foo::translate' will match both
'Foo::translate' and 'Foo::translateFrob'. If you just want Foo::translate, use
'Foo::translate('.

Finally, if multiple symbols are given, a summary will be printed showing the
total number of samples, and how many samples contain each symbol.

If the first argument is --reverse, the call graph will be inverted, showing
callers of any given symbols, rather than callees.

EOT;
}

function main($argv) {
  ini_set('memory_limit', '64G');
  if (posix_isatty(STDIN)) {
    usage($argv[0]);
    return 1;
  }
  array_shift(&$argv);

  $reverse = false;
  if (($argv[0] ?? null) === '--reverse') {
    $reverse = true;
    array_shift(&$argv);
  }
  $functions = new Set($argv);

  $samples = read_perf_samples(STDIN, 'hhvm');
  $subsamples = Map {'all' => $samples};
  foreach ($functions as $f) {
    $subsamples[$f] = $samples->filter($s ==> contains_frame($s, $f));
  }

  if ($functions->isEmpty()) {
    treeify($subsamples['all'], $reverse, null);
  } else if ($functions->count() === 1) {
    $func = $functions->firstValue();
    $sub = $subsamples[$func];
    printf("Looking for pattern *%s*. %d of %d total samples (%.2f%%)\n\n",
           $func, $sub->count(), $samples->count(),
           $sub->count() / $samples->count() * 100);
    treeify($sub, $reverse, $func);
  } else {
    foreach ($subsamples as $k => $v) {
      printf(
        "%8d %5.1f %s\n", $v->count(), $v->count() / $samples->count() * 100, $k
      );
    }
  }

  return 0;
}

exit(main($argv));
