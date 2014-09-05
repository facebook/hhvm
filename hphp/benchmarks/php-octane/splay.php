<?
// Copyright 2009 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Ported to PHP from Google's Octane v2.0 benchmarking suite for JavaScript.

// This benchmark is based on a JavaScript log processing module used
// by the V8 profiler to generate execution time profiles for runs of
// JavaScript applications, and it effectively measures how fast the
// JavaScript engine is at allocating nodes and reclaiming the memory
// used for old nodes. Because of the way splay trees work, the engine
// also has to deal with a lot of changes to the large tree object
// graph.


// Configuration.
define('kSplayTreeSize', 8000);
define('kSplayTreeModifications', 80);
define('kSplayTreePayloadDepth', 5);

$splayTree = null;
$splaySampleTimeStart = 0.0;

function GeneratePayloadTree($depth, $tag) {
    if ($depth == 0) {
        return array(
            "array"  => array(0, 1, 2, 3, 4, 5, 6, 7, 8, 9),
            "string" => "String for key $tag in leaf node"
        );
    } else {
        return array(
            "left"  => GeneratePayloadTree($depth - 1, $tag),
            "right" => GeneratePayloadTree($depth - 1, $tag)
        );
    }
}


function GenerateKey() {
    // The benchmark framework guarantees that Math.random is
    // deterministic; see base.js.
    return rand();
}

$splaySamples = 0;
$splaySumOfSquaredPauses = 0;

$SplayRMS = function() use (&$splaySumOfSquaredPauses, &$splaySamples) {
    $result = round(sqrt($splaySumOfSquaredPauses / $splaySamples) * 10000);
    return $result;
};

function SplayUpdateStats($time) {
    global $splaySamples, $splaySumOfSquaredPauses, $splaySampleTimeStart;

    $pause = $time - $splaySampleTimeStart;
    $splaySampleTimeStart = $time;
    $splaySamples++;
    $splaySumOfSquaredPauses += $pause * $pause;
}

function InsertNewNode() {
    global $splayTree;

    // Insert new node with a unique key.
    $key = null;
    do {
        $key = GenerateKey();
    } while (!is_null($splayTree->find($key)));
    $payload = GeneratePayloadTree(kSplayTreePayloadDepth, (string)$key);
    $splayTree->insert($key, $payload);
    return $key;
}


$SplaySetup = function() {
    global $splayTree, $performance, $splaySampleTimeStart;

    // Check if the platform has the performance.now high resolution timer.
    // If not, throw exception and quit.
    $splayTree = new SplayTree();
    $splaySampleTimeStart = $performance['now']();
    for ($i = 0; $i < kSplayTreeSize; $i++) {
        InsertNewNode();
        if (($i + 1) % 20 == 19) {
            SplayUpdateStats($performance['now']());
        }
    }
};


$SplayTearDown = function() {
    global $splayTree;

    // Allow the garbage collector to reclaim the memory
    // used by the splay tree no matter how we exit the
    // tear down function.
    $keys = $splayTree->exportKeys();
    $splayTree = null;

    $splaySamples = 0;
    $splaySumOfSquaredPauses = 0;

    // Verify that the splay tree has the right size.
    $length = count($keys);
    if ($length != kSplayTreeSize) {
        throw new Exception("Splay tree has wrong size: $length");
    }

    // Verify that the splay tree has sorted, unique keys.
    for ($i = 0; $i < $length - 1; $i++) {
        if ($keys[$i] >= $keys[$i + 1]) {
            throw new Exception("Splay tree not sorted");
        }
    }
};


$SplayRun = function() use (&$splayTree) {
    global $performance;

    // Replace a few nodes in the splay tree.
    for ($i = 0; $i < kSplayTreeModifications; $i++) {
        $key = InsertNewNode();
        $greatest = $splayTree->findGreatestLessThan($key);
        if (is_null($greatest))
            $splayTree->remove($key);
        else
            $splayTree->remove($greatest->key);
    }
    SplayUpdateStats($performance['now']());
};

class SplayTreeNode {
    public $key;
    public $value;
    public $left;
    public $right;

    /**
     * Constructs a Splay tree node.
     *
     * @param {number} key Key.
     * @param {*} value Value.
     */
    function __construct($key, $value) {
        $this->key = $key;
        $this->value = $value;
        $this->left = null;
        $this->right= null;
    }

    /**
     * Performs an ordered traversal of the subtree starting at
     * this SplayTree.Node.
     *
     * @param {function(SplayTree.Node)} f Visitor function.
     * @private
     */
    function traverse_($f) {
        $current = $this;
        while ($current) {
            $left = $current->left;
            if ($left)
                $left->traverse_($f);
            $f($current);
            $current = $current->right;
        }
    }
}

/**
 * Constructs a Splay tree.  A splay tree is a self-balancing binary
 * search tree with the additional property that recently accessed
 * elements are quick to access again. It performs basic operations
 * such as insertion, look-up and removal in O(log(n)) amortized time.
 *
 * @constructor
 */
class SplayTree {
    private $root_;

    function __construct() {
        $this->root_ = null;
    }

    /**
     * @return {boolean} Whether the tree is empty.
     */
    function isEmpty() {
        return is_null($this->root_);
    }

    /**
     * Inserts a node into the tree with the specified key and value if
     * the tree does not already contain a node with the specified key. If
     * the value is inserted, it becomes the root of the tree.
     *
     * @param {number} key Key to insert into the tree.
     * @param {*} value Value to insert into the tree.
     */
    function insert($key, $value) {
        if ($this->isEmpty()) {
            $this->root_ = new SplayTreeNode($key, $value);
            return;
        }
        // Splay on the key to move the last node on the search path for
        // the key to the root of the tree.
        $this->splay_($key);
        if ($this->root_->key == $key) {
            return;
        }
        $node = new SplayTreeNode($key, $value);
        if ($key > $this->root_->key) {
            $node->left = $this->root_;
            $node->right = $this->root_->right;
            $this->root_->right = null;
        } else {
            $node->right = $this->root_;
            $node->left = $this->root_->left;
            $this->root_->left = null;
        }
        $this->root_ = $node;
    }

    /**
     * Removes a node with the specified key from the tree if the tree
     * contains a node with this key. The removed node is returned. If the
     * key is not found, an exception is thrown.
     *
     * @param {number} key Key to find and remove from the tree.
     * @return {SplayTree.Node} The removed node.
     */
    function remove($key) {
        if ($this->isEmpty()) {
            throw new Exception("Key not found: $key");
        }
        $this->splay_($key);
        if ($this->root_->key != $key) {
            throw new Exception("Key not found: $key");
        }
        $removed = $this->root_;
        if (is_null($this->root_->left)) {
            $this->root_ = $this->root_->right;
        } else {
            $right = $this->root_->right;
            $this->root_ = $this->root_->left;
            // Splay to make sure that the new root has an empty right child.
            $this->splay_($key);
            // Insert the original right child as the right child of the new
            // root.
            $this->root_->right = $right;
        }
        return $removed;
    }

    /**
     * Returns the node having the specified key or null if the tree doesn't
     * contain a node with the specified key.
     *
     * @param {number} key Key to find in the tree.
     * @return {SplayTree.Node} Node having the specified key.
     */
    function find($key) {
        if ($this->isEmpty()) {
            return null;
        }
        $this->splay_($key);
        return $this->root_->key == $key ? $this->root_ : null;
    }

    /**
     * @return {SplayTree.Node} Node having the maximum key value.
     */
    function findMax($opt_startNode) {
        if ($this->isEmpty()) {
            return null;
        }
        $current = $opt_startNode ? $opt_startNode : $this->root_;
        while ($current->right) {
            $current = $current->right;
        }
        return $current;
    }


    /**
     * @return {SplayTree.Node} Node having the maximum key value that
     *     is less than the specified key value.
     */
    function findGreatestLessThan($key) {
        if ($this->isEmpty()) {
            return null;
        }
        // Splay on the key to move the node with the given key or the last
        // node on the search path to the top of the tree.
        $this->splay_($key);
        // Now the result is either the root node or the greatest node in
        // the left subtree.
        if ($this->root_->key < $key) {
            return $this->root_;
        } else if ($this->root_->left) {
            return $this->findMax($this->root_->left);
        } else {
            return null;
        }
    }

    /**
     * @return {Array<*>} An array containing all the keys of tree's nodes.
     */
    function exportKeys() {
        $result = [];
        if (!$this->isEmpty()) {
            $this->root_->traverse_(function($node) use (&$result) {
                array_push($result, $node->key);
            });
        }
        return $result;
    }

    /**
     * Perform the splay operation for the given key. Moves the node with
     * the given key to the top of the tree.  If no node has the given
     * key, the last node on the search path is moved to the top of the
     * tree. This is the simplified top-down splaying algorithm from:
     * "Self-adjusting Binary Search Trees" by Sleator and Tarjan
     *
     * @param {number} key Key to splay the tree on.
     * @private
     */
    function splay_($key) {
        if ($this->isEmpty()) {
            return;
        }
        // Create a dummy node.  The use of the dummy node is a bit
        // counter-intuitive: The right child of the dummy node will hold
        // the L tree of the algorithm.  The left child of the dummy node
        // will hold the R tree of the algorithm.  Using a dummy node, left
        // and right will always be nodes and we avoid special cases.
        $dummy = new SplayTreeNode(null, null);
        $left = $dummy;
        $right = $dummy;
        $current = $this->root_;
        while (true) {
            if ($key < $current->key) {
                if (is_null($current->left)) {
                    break;
                }
                if ($key < $current->left->key) {
                    // Rotate right.
                    $tmp = $current->left;
                    $current->left = $tmp->right;
                    $tmp->right = $current;
                    $current = $tmp;
                    if (is_null($current->left)) {
                        break;
                    }
                }
                // Link right.
                $right->left = $current;
                $right = $current;
                $current = $current->left;
            } else if ($key > $current->key) {
                if (is_null($current->right)) {
                    break;
                }
                if ($key > $current->right->key) {
                    // Rotate left.
                    $tmp = $current->right;
                    $current->right = $tmp->left;
                    $tmp->left = $current;
                    $current = $tmp;
                    if (is_null($current->right)) {
                        break;
                    }
                }
                // Link left.
                $left->right = $current;
                $left = $current;
                $current = $current->right;
            } else {
              break;
            }
        }
        // Assemble.
        $left->right = $current->left;
        $right->left = $current->right;
        $current->left = $dummy->right;
        $current->right = $dummy->left;
        $this->root_ = $current;
    }
}

$Splay = new BenchmarkSuite('Splay', [81491, 2739514], array(
    new Benchmark("Splay", true, false, 1400,
        $SplayRun, $SplaySetup, $SplayTearDown, $SplayRMS)
));

?>
