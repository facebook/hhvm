<?
// A simple implementation of lossless compression and decompression using
// Huffman coding.

$asciiMap = array();
$symbolMap = array();
define('SYMBOL_SIZE', 8);

for ($i = 0; $i < 256; ++$i) {
  $binary = decbin($i);
  while (strlen($binary) < SYMBOL_SIZE)
    $binary = "0$binary";
  $asciiMap[chr($i)] = array_map(function($i) {
    return (int)$i;
  }, str_split($binary));
  $symbolMap[$binary] = chr($i);
}

class Node {
  public $left;
  public $right;
  public $parent;
  public $symbol;
  public $weight;

  function __construct($symbol, $weight) {
    $this->left = null;
    $this->right = null;
    $this->parent = null;
    $this->symbol = $symbol;
    $this->weight = $weight;
  }

  function __toString() {
    return "Node ($this->symbol)";
  }

  function priority() {
    return -$this->weight;
  }

  function dump($tabs) {
    $prefix = "";
    for ($i = 0; $i < $tabs; ++$i)
      $prefix .= "  ";
    print($prefix . "(" . $this->symbol . ", " . $this->weight . ")\n");
  }
}

class Tree {
  public $root;
  public $leaves;

  function __construct() {
    $this->root = null;
    $this->leaves = array();
  }

  function dump() {
    if (!$this->root) {
      print("empty");
      return;
    }

    $tabs = 0;
    $q = array();
    array_push($q, array($this->root, false));

    while (count($q)) {
      $next = array_pop($q);
      if ($next[1]) {
        $tabs -= 1;
        continue;
      }

      $node = $next[0];
      $node->dump($tabs);
      $tabs++;
      array_push($q, array($node, true));

      if ($node->right)
        array_push($q, array($node->right, false));
      if ($node->left)
        array_push($q, array($node->left, false));
    }
  }
}

class PriorityQueue {
  public $heap;
  private $size;

  function __construct() {
    $this->heap = array();
    $this->size = 0;
  }

  function size() {
    return $this->size;
  }

  function enqueue($e) {
    $this->heap[$this->size++] = $e;
    $this->bubbleUp();
  }

  function bubbleUp() {
    $currentLocation = $this->size - 1;
    while ($currentLocation > 0) {
      $parentLocation = $currentLocation >> 1;
      $currentNode = $this->heap[$currentLocation];
      $parentNode = $this->heap[$parentLocation];
      if ($currentNode->priority() <= $parentNode->priority())
        break;
      $this->heap[$parentLocation] = $currentNode;
      $this->heap[$currentLocation] = $parentNode;
      $currentLocation = $parentLocation;
    }
  }

  function dequeue() {
    if ($this->size() == 0)
      return null;
    $result = $this->heap[0];
    $this->heap[0] = null;
    $this->filterDown();
    $this->size--;
    return $result;
  }

  function filterDown() {
    if (!$this->size)
      return;

    $holeLocation = 0;
    $last = $this->heap[$this->size - 1];
    while (true) {
      $child1Location = 2 * $holeLocation + 1;
      $child2Location = 2 * $holeLocation + 2;
      $child1 = null;
      $child2 = null;
      if ($child1Location < $this->size)
        $child1 = $this->heap[$child1Location];
      if ($child2Location < $this->size)
        $child2 = $this->heap[$child2Location];

      if (($child1
        && $child2
        && $child1->priority() > $child2->priority())
        || ($child1 && !$child2)) {
        $this->heap[$holeLocation] = $child1;
        $this->heap[$child1Location] = null;
        $holeLocation = $child1Location;
      } else if ($child2) {
        $this->heap[$holeLocation] = $child2;
        $this->heap[$child2Location] = null;
        $holeLocation = $child2Location;
      } else {
        break;
      }
    }

    // We filtered down to the last element, so we don't need to do
    // anything else.
    if ($holeLocation == $this->size - 1)
      return;

    // We need to fill in the hole that we created in the middle of the
    // final level.
    $this->heap[$holeLocation] = $last;
    $this->heap[$this->size - 1] = null;
  }
}

class Huffman {
  public static function buildTree($s)
  {
    $t = new Tree();
    $seenSymbols = array();
    $pq = new PriorityQueue();
    // Add leaf nodes for each symbol to the priority queue.
    for ($i = 0; $i < strlen($s); ++$i) {
      $c = $s[$i];
      if (array_key_exists($c, $seenSymbols))
        continue;
      $occurrences = 0;
      for ($j = $i; $j < strlen($s); ++$j) {
        if ($s[$j] != $c)
          continue;
        $occurrences++;
      }
      $node = new Node($c, $occurrences);
      $pq->enqueue($node);
      $t->leaves[$c] = $node;
      $seenSymbols[$c] = true;
    }

    if ($pq->size() == 0)
      return $t;

    // While there is more than one node left in the priority queue:
    while ($pq->size() > 1) {
      // Remove the two nodes of highest priority (lowest probability).
      $node1 = $pq->dequeue();
      $node2 = $pq->dequeue();
      // Create a new internal node with the two nodes as children with
      // p_total = p_1 + p_2
      $newNode = new Node(null, $node1->weight + $node2->weight);
      $newNode->left = $node1;
      $newNode->right = $node2;
      $node1->parent = $newNode;
      $node2->parent = $newNode;
      // Add the new node to the queue.
      $pq->enqueue($newNode);
    }

    // The final node in the priority queue is the root.
    $t->root = $pq->dequeue();
    return $t;
  }

  private static function encodeTree($t)
  {
    global $asciiMap;

    // Emit how big each leaf symbol is.
    // Emit a 0 for an internal node and a 1 for each leaf.
    $treeBits = array();
    $q = array();
    array_push($q, $t->root);
    while (count($q)) {
      $next = array_pop($q);
      if ($next->left && $next->right) {
        array_push($treeBits, 0);
        // Visit the left side first, so push the right side before
        // the left.
        array_push($q, $next->right);
        array_push($q, $next->left);
      } else {
        if ($next->left || $next->right)
          throw new Exception("Leaf nodes do not have children.");
        array_push($treeBits, 1);
        $symbolBits = $asciiMap[$next->symbol];
        for ($i = 0; $i < count($symbolBits); ++$i) {
          array_push($treeBits, $symbolBits[$i]);
        }
      }
    }
    return $treeBits;
  }

  public static function encode($s, $t)
  {
    if (!$t->root)
      return;

    // Encode the tree first.
    $allBits = Huffman::encodeTree($t);

    for ($i = 0; $i < strlen($s); ++$i) {
      $c = $s[$i];
      $current = $t->leaves[$c];
      $bits = array();
      while (true) {
        $parent = $current->parent;
        if (!$parent)
          break;
        if ($current === $parent->left)
          array_push($bits, 0);
        else
          array_push($bits, 1);
        $current = $parent;
      }
      $allBits = array_merge($allBits, array_reverse($bits));
    }

    return $allBits;
  }

  private static function decodeTree($treeBits) {
    $t = new Tree();
    if (!count($treeBits))
      return $t;

    $leaves = array();

    $helper = function($treeBits, $i) use (&$helper, &$leaves) {
      global $symbolMap;

      if ($treeBits[$i] == 1) {
        $i++;
        // TODO: Refactor Node so that it doesn't store the weight
        // internally.
        $symbolBits = array();
        for ($j = 0; $j < SYMBOL_SIZE; ++$j)
          array_push($symbolBits, $treeBits[$i + $j]);
        $i += SYMBOL_SIZE;
        $symbol = $symbolMap[join("", $symbolBits)];
        $node = new Node($symbol, 0);
        $leaves[$symbol] = $node;
        return array($i, $node);
      } else {
        $leftResult = $helper($treeBits, $i + 1);
        $leftChild = $leftResult[1];
        $rightResult = $helper($treeBits, $leftResult[0]);
        $rightChild = $rightResult[1];
        $node = new Node(null, 0);
        $node->left = $leftChild;
        $node->right = $rightChild;
        $leftChild->parent = $node;
        $rightChild->parent = $node;
        return array($rightResult[0], $node);
      }
    };

    $result = $helper($treeBits, 0);
    $t->root = $result[1];
    $t->leaves = $leaves;
    return array($result[0], $t);
  }

  public static function decode($bits) {
    $result = Huffman::decodeTree($bits);
    $startIndex = $result[0];
    $tree = $result[1];
    $current = $tree->root;
    $s = "";
    $i = $startIndex;
    while ($i < count($bits)) {
      if ($current->left && $current->right) {
        if ($bits[$i++])
          $current = $current->right;
        else
          $current = $current->left;
      } else {
        if ($current->left || $current->right)
          throw new Exception("Leaf must not have any children.");
        $s .= $current->symbol;
        $current = $tree->root;
      }
    }
    if ($current->left || $current->right) {
      throw new Exception("Leaf expected at end of input.");
    }
    $s .= $current->symbol;
    return $s;
  }
}

$s = join("\n", array(
  "4 Minute Warning - Radiohead",
  "",
  "This is just a nightmare",
  "Soon I'm gonna wake up",
  "Someone's gonna bring me 'round",
  "",
  "Running from the bombers",
  "Hiding in the forest",
  "Running through the fields",
  "Laying flat on the ground",
  "",
  "Just like everybody",
  "Stepping over hills",
  "Running from the underground",
  "",
  "This is your warning",
  "4 minute warning",
  "",
  "I don't wanna hear it",
  "I don't wanna know",
  "I just wanna run and hide",
  "",
  "This is just a nightmare",
  "But soon I'm gonna wake up",
  "Someone's gonna bring me 'round",
  "",
  "This is our warning",
  "4 minute warning"
));

$RunHuffman = function() {
  global $s;

  $t = Huffman::buildTree($s);
  $bits = Huffman::encode($s, $t);
  $s2 = Huffman::decode($bits);
  if ($s2 != $s) {
    echo "Expected:\n$s\n";
    echo "Actual:\n$s2\n";
    throw new Exception("Incorrect result");
  }
};

$Huffman = new BenchmarkSuite('Huffman', [100000], [
  new Benchmark('Huffman', true, false, 100000, $RunHuffman)
]);
?>
