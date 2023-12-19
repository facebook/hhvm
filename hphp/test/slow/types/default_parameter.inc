<?hh

class string {
}

class int {
}

class float {
}

class bool {
}

class resource {
}

class arraykey {
}

class num {
}

class Foo {
}


function test($prefix) {
  $hints = vec[
    // Primitives without HH namespace
    "bool",
    "int",
    "num",
    "arraykey",
    "float",
    "string",
    // Object
    "Foo",
    // Resource
    "resource",
    "HH\\resource",
    // Primitives
    "array",
    "HH\\bool",
    "HH\\int",
    "HH\\num",
    "HH\\arraykey",
    "HH\\float",
    "HH\\string",
    // Hack only
    "HH\\Traversable",
    "HH\\KeyedTraversable",
    "HH\\Container",
    "HH\\KeyedContainer",
    "XHPChild",
    "Stringish"
  ];

  $default_values = dict[
    "b" => "false",
    "i" => "1337",
    "f" => "3.1415926",
    "s" => "\"foo\"",
    "a" => "array()",
    "n" => "null",
    "r" => "STDIN"
  ];

  set_error_handler(
    function(
      $errno,
      $errstr
    ) {
      echo "Catchable fatal error ($errno): $errstr\n";
      // ON ERROR RESUME NEXT;
      return true;
    }
  );

  $i = 0;
  $tldr_table = dict[];
  foreach ($hints as $h) {
    $tldr_row = vec[];
    echo "##### Parameter type: $h #####\n";
    foreach ($default_values as $dv) {
      echo "== Default value: $dv ==\n";
      // Function names are prepended with a prefix so that force_hh and
      // no_force_hh tests are compiled separately.
      $function = sprintf("%s_test_%d", $prefix, $i);
      array_push($tldr_row, eval(
        "function $function($h \$val = $dv) {
           var_dump(\$val);
         }
        $function();
        return true;"
      ));
      echo "\n";
      $i++;
    }
    $tldr_table[$h] = $tldr_row;
  }

  echo "\n###############################################\n";
  echo "tl;dr; Table:\n";
  $padding = max(array_map("strlen", $hints));

  // Head
  echo str_pad("", $padding);
  foreach (array_keys($default_values) as $type) {
    echo "|$type";
  }
  echo "\n";

  // Body
  foreach ($tldr_table as $hint => $row) {
    echo str_pad($hint, $padding);
    foreach ($row as $result) {
      echo "|";
      echo $result ? "O" : "X";
    }
    echo "\n";
  }
}
