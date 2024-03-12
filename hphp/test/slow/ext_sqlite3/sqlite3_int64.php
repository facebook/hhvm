<?hh

function VR(mixed $got, dict<string, mixed> $exp) {
  if ($got === false) {
    echo "Failed: no row\n";
  } else if ($got is null) {
    echo "Failed: error fetching the row\n";
  } else if ($got === $exp) {
    echo "Passed: row matched\n";
  } else {
    echo "Failed, expected:";
    var_dump($exp);
    echo "\nBut got:";
    var_dump($got);
    var_dump(debug_backtrace());
  }
}

function VS(int $x, int $y) {
  var_dump($x === $y);
  if ($x !== $y) {
    echo "Failed: $y\n"; echo "Got: $x\n";
    var_dump(debug_backtrace());
  }
}

function VERIFY($x) { VS((int)($x != false), (int)true); }

//////////////////////////////////////////////////////////////////////

function identity($r) {
 return (int)$r;
}

<<__EntryPoint>>
function main_ext_sqlite3() {
  $db = new SQLite3(':memory:');
  $db->exec("CREATE TABLE numbers (name STRING, val INTEGER)");

  $db->exec("INSERT INTO numbers VALUES ('small', 1234)");
  // All those values are bigger than an int32.
  $db->exec("INSERT INTO numbers VALUES ('large', 845868619338)");
  $db->exec("INSERT INTO numbers VALUES ('large_neg', -845787619309)");
  $db->exec("INSERT INTO numbers VALUES ('max64', " . PHP_INT_MAX . ")");
  $db->exec("INSERT INTO numbers VALUES ('min64', " . PHP_INT_MIN . ")");

  echo "testing query() and SQLite3Result\n";
  {
    $res = $db->query("SELECT * FROM numbers");
    VR($res->fetcharray(SQLITE3_ASSOC), dict["name" => "small", "val" => 1234]);
    VR($res->fetcharray(SQLITE3_ASSOC), dict["name" => "large", "val" => 845868619338]);
    VR($res->fetcharray(SQLITE3_ASSOC), dict["name" => "large_neg", "val" => -845787619309]);
    VR($res->fetcharray(SQLITE3_ASSOC), dict["name" => "max64", "val" => PHP_INT_MAX]);
    VR($res->fetcharray(SQLITE3_ASSOC), dict["name" => "min64", "val" => PHP_INT_MIN]);
    $res->finalize();
  }

  echo "testing bind with name\n";
  {
    $stmt = $db->prepare("SELECT * FROM numbers WHERE val = :val");
    VS($stmt->paramcount(), 1);

    $val = PHP_INT_MAX;
    VERIFY($stmt->bindvalue(":val", $val, SQLITE3_INTEGER));
    {
      $res = $stmt->execute();
      VR($res->fetcharray(SQLITE3_ASSOC), dict["name" => "max64", "val" => $val]);
      $res->finalize();
    }

    VERIFY($stmt->clear());
    VERIFY($stmt->reset());
  }

  {
    $stmt = $db->prepare("SELECT * FROM numbers WHERE val = :val");
    VS($stmt->paramcount(), 1);

    $val = PHP_INT_MIN;
    VERIFY($stmt->bindvalue(":val", $val, SQLITE3_INTEGER));
    {
      $res = $stmt->execute();
      VR($res->fetcharray(SQLITE3_ASSOC), dict["name" => "min64", "val" => $val]);
      $res->finalize();
    }

    VERIFY($stmt->clear());
    VERIFY($stmt->reset());
  }

  echo "testing bind with number\n";
  {
    $stmt = $db->prepare("SELECT * FROM numbers WHERE name in (?, ?, ?, ?)");
    VS($stmt->paramcount(), 4);

    VERIFY($stmt->bindvalue(1, "large", SQLITE3_TEXT));
    VERIFY($stmt->bindvalue(2, "large_neg", SQLITE3_TEXT));
    VERIFY($stmt->bindvalue(3, "max64", SQLITE3_TEXT));
    VERIFY($stmt->bindvalue(4, "min64", SQLITE3_TEXT));

    {
      $res = $stmt->execute();
      VR($res->fetcharray(SQLITE3_ASSOC), dict["name" => "large", "val" => 845868619338]);
      VR($res->fetcharray(SQLITE3_ASSOC), dict["name" => "large_neg", "val" => -845787619309]);
      VR($res->fetcharray(SQLITE3_ASSOC), dict["name" => "max64", "val" => PHP_INT_MAX]);
      VR($res->fetcharray(SQLITE3_ASSOC), dict["name" => "min64", "val" => PHP_INT_MIN]);
      $res->finalize();
    }

    VERIFY($stmt->clear());
    VERIFY($stmt->reset());
  }

  echo "testing UDF\n";
  {
    VERIFY($db->createfunction("identity", identity<>, 1));
    $res = $db->query("SELECT identity(val) FROM numbers");
    VR($res->fetcharray(SQLITE3_NUM), dict[0 => 1234]);
    VR($res->fetcharray(SQLITE3_NUM), dict[0 => 845868619338]);
    VR($res->fetcharray(SQLITE3_NUM), dict[0 => -845787619309]);
    VR($res->fetcharray(SQLITE3_NUM), dict[0 => PHP_INT_MAX]);
    VR($res->fetcharray(SQLITE3_NUM), dict[0 => PHP_INT_MIN]);
    $res->finalize();
  }

  $stmt->close();
  $db->close();
}
