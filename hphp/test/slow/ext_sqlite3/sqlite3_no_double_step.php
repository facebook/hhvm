<?hh

// Test that SELECT queries are not executed twice when using the standard
// execute()->fetcharray() pattern. Verifies correct row data and iteration
// behavior after the buffered first-row fix.

<<__EntryPoint>>
function main_sqlite3_no_double_step(): void {
  $db = new SQLite3(':memory:');
  $db->exec("CREATE TABLE t (v TEXT)");
  $db->exec("INSERT INTO t VALUES ('row1')");
  $db->exec("INSERT INTO t VALUES ('row2')");
  $db->exec("INSERT INTO t VALUES ('row3')");

  // --- Test 1: query() + multi-row fetcharray() iteration ---
  echo "=== Test 1: query() + fetcharray loop ===\n";
  $res = $db->query("SELECT v FROM t");
  $rows = vec[];
  while ($row = $res->fetcharray(SQLITE3_ASSOC)) {
    $rows[] = $row['v'];
  }
  $res->finalize();
  echo "rows: " . implode(', ', $rows) . "\n";
  echo "count: " . count($rows) . "\n";
  echo "pass: " . (count($rows) === 3 ? "yes" : "no") . "\n\n";

  // --- Test 2: prepare() + execute() + fetcharray() ---
  echo "=== Test 2: prepare + execute + fetcharray ===\n";
  $stmt = $db->prepare("SELECT v FROM t WHERE v = :id");
  $stmt->bindvalue(':id', 'row1', SQLITE3_TEXT);
  $res = $stmt->execute();
  $row = $res->fetcharray(SQLITE3_ASSOC);
  echo "value: " . $row['v'] . "\n";
  // Second fetcharray should return false (only one matching row).
  $row2 = $res->fetcharray(SQLITE3_ASSOC);
  echo "next: " . ($row2 === false ? "false" : "unexpected") . "\n";
  $res->finalize();
  echo "\n";

  // --- Test 3: Re-execute same statement (tests reset-before-bind) ---
  echo "=== Test 3: re-execute same statement ===\n";
  $stmt->reset();
  $stmt->clear();
  $stmt->bindvalue(':id', 'row2', SQLITE3_TEXT);
  $res = $stmt->execute();
  $row = $res->fetcharray(SQLITE3_ASSOC);
  echo "value: " . $row['v'] . "\n";
  $row2 = $res->fetcharray(SQLITE3_ASSOC);
  echo "next: " . ($row2 === false ? "false" : "unexpected") . "\n";
  $res->finalize();
  $stmt->close();
  echo "\n";

  // --- Test 4: Write query (INSERT) + fetcharray returns false ---
  echo "=== Test 4: write query + fetcharray ===\n";
  $res = $db->query("INSERT INTO t VALUES ('row4')");
  $row = $res->fetcharray();
  echo "fetcharray after INSERT: " . ($row === false ? "false" : "unexpected") . "\n\n";
  $res->finalize();

  // --- Test 5: columntype() after execute, before fetcharray ---
  echo "=== Test 5: columntype after execute ===\n";
  $res = $db->query("SELECT v FROM t WHERE v = 'row1'");
  // With the fix, the cursor is positioned on a row, so columntype works.
  $ctype = $res->columntype(0);
  echo "columntype: " . $ctype . "\n";
  echo "is_text: " . ($ctype === SQLITE3_TEXT ? "yes" : "no") . "\n";
  $res->finalize();

  $db->close();
}
