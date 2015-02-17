<?php

// increase max nesting to account for deep scripts
ini_set('xdebug.max_nesting_level', 2000);

// import PHP-Parser
require '../../../third-party2/PHP-Parser/trunk/src/PHP-Parser-trunk/lib/bootstrap.php';

// create global parser and printers
$parser = new PhpParser\Parser(new PhpParser\Lexer);
$printer = new PhpParser\PrettyPrinter\Standard;

// Globals
$prog_hash = array(); // array for memoizing
$minimum_prog = ""; // minimum program string
$minimum_stmt = null; // minimum statement count
$tests_run = 0; // total number of tests run

// find max nested statement depth
function stmt_depth(PhpParser\Node $stmt) {
  $max_depth = 1;
  if (property_exists($stmt,"subNodes") &&
      array_key_exists("stmts",$stmt->subNodes)) {
    foreach ($stmt->subNodes["stmts"] as $sub_stmt) {
      $depth = stmt_depth($sub_stmt) + 1;
      if ($depth > $max_depth) {
        $max_depth = $depth;
      }
    }
  }
  return $max_depth;
}

// find number of nested statements in statement array
function stmt_count($code) {
  $count = count($code);
  foreach ($code as $stmt) {
    if (property_exists($stmt,"subNodes") &&
      array_key_exists("stmts",$stmt->subNodes)) {
      $count += stmt_count($stmt->subNodes["stmts"]);
    }
  }
  return $count;
}

// determine max program depth starting at statement array
function prog_depth($php_code) {
  $max_depth = 0;
  foreach ($php_code as $stmt) {
    $depth = stmt_depth($stmt);
    $max_depth = max($depth,$max_depth);
  }
  return $max_depth;
}

// remove statement at index from code reference
// $code => array(Stmt)
function reduce_stmt_at_location($code , $location) {
  $max_loc = stmt_count($code);
  $total_seen = 0;
  $new_code = &$code;
  while($location < $max_loc && $total_seen < $max_loc) {
    $node_number = 0;
    var_dump($total_seen);
    foreach($new_code as $stmt) {
      if ($total_seen == $location) {
        array_splice($new_code,$node_number,1);
        return $code;
      } else {
        $total_seen++;
        if (property_exists($stmt,"subNodes") &&
          array_key_exists("stmts",$stmt->subNodes)) {
          //has sub node statements
          $local_stmt_count = stmt_count($stmt->subNodes["stmts"]);
          if ($total_seen + $local_stmt_count > $location) {
            // sub node should be removed
            $new_code=&$stmt->subNodes["stmts"];
            break;
          } else {
            $total_seen += $local_stmt_count;
          }
        }
      }
      $node_number++;
    }
  }
}

// run code on two command lines and look for differences
function test($php_code, $command1, $command2) {
  global $printer;
  global $tests_run;
  $filename = md5(time());
  $filename = $filename.".php";
  $code = $printer->prettyPrint($php_code);

  $file = fopen("./" . $filename,"w");
  fwrite($file, "<?php\n\n");
  fwrite($file, $code);
  fclose($file);
  echo "Testing Code:\n";
  echo "*************\n";
  echo $code;
  echo "\n\n";
  echo "testing command 1\n";
  $result1 = array();
  echo "$command1 $filename";
  $res_handle1 = exec("$command1 $filename", $result1);
  echo "testing command 2\n";
  $result2 = array();
  $res_handle2 = exec("$command2 $filename", $result2);
  var_dump($result1);
  var_dump($result2);
  unlink($filename);
  $tests_run += 1;
  if ($result1 === $result2) {
    return true;
  } else {
    return false;
  }
}

// Recursively test all sub scripts of provided statement list
function recursive_statement_reduce($stmts) {
  global $prog_hash;
  global $minimum_stmt;
  global $minimum_prog;
  global $argv;
  $code_serialized = serialize($stmts);
  for($x = 0; $x < stmt_count($stmts) ; $x++) {
    // the code has to be serialized and unserialized to get a deep copy
    $code = unserialize($code_serialized);
    $code = reduce_stmt_at_location($code,$x);
    $hash_val = md5(serialize($code));
    if (!array_key_exists($hash_val, $prog_hash)) {
      // code hasn't been analyzed ... yay!
      if ( !test($code,$argv[2],$argv[3])) {
        $stmt_count = stmt_count($code);
        if (is_null($minimum_stmt) || $stmt_count < $minimum_stmt) {
          $minimum_stmt = $stmt_count;
          $minimum_prog = serialize($code);
        }
        $prog_hash[$hash_val] = false;
      } else {
        $prog_hash[$hash_val] = true;
      }
      recursive_statement_reduce($code);
    }
  }
}
$php_code = file_get_contents($argv[1]);

// print results based on info in global variables
function print_result() {
  global $minimum_stmt;
  global $minimum_prog;
  global $printer;
  global $tests_run;
  echo "***************************************************\n";
  echo "****************** RESULTS ************************\n";
  echo "***************************************************\n";
  echo "Total test cases run : $tests_run\n";
  if (is_null($minimum_stmt)) {
    echo "No minimum case found\n";
  } else {
    echo "Minimum program found!!!!\n";
    echo "\n";
    $output_prog = unserialize($minimum_prog);
    echo $printer->prettyPrint($output_prog);
    echo "\n";
  }
}


// driver
try {
  // load code
  $stmts = $parser->parse($php_code);

  // reduction tests
  recursive_statement_reduce($stmts);

  // print test results
  print_result();
} catch (PhpParser\Error $e) {
  echo 'Parse Error: ',  $e->getMessage();
}
