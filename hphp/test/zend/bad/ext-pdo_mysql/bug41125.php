<?php

require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');

$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');

$search = "o'";
$sql = "SELECT 1 FROM DUAL WHERE 'o''riley' LIKE " . $db->quote('%' . $search . '%');
$stmt = $db->prepare($sql);
$stmt->execute();
print implode(' - ', (($r = @$stmt->fetch(PDO::FETCH_NUM)) ? $r : array())) ."\n";
print implode(' - ', $stmt->errorinfo()) ."\n";

print "-------------------------------------------------------\n";

$queries = array(
	"SELECT 1 FROM DUAL WHERE 1 = '?\'\''",
	"SELECT 'a\\'0' FROM DUAL WHERE 1 = ?",
	"SELECT 'a', 'b\'' FROM DUAL WHERE '''' LIKE '\\'' AND ?",
	"SELECT 'foo?bar', '', '''' FROM DUAL WHERE ?"
);

foreach ($queries as $k => $query) {
	$stmt = $db->prepare($query);
	$stmt->execute(array(1));
	printf("[%d] Query: [[%s]]\n", $k + 1, $query);
	print implode(' - ', (($r = @$stmt->fetch(PDO::FETCH_NUM)) ? $r : array())) ."\n";
	print implode(' - ', $stmt->errorinfo()) ."\n";
	print "--------\n";
}

$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);
$sql = "SELECT upper(:id) FROM DUAL WHERE '1'";
$stmt = $db->prepare($sql);

$id = 'o\'\0';
$stmt->bindParam(':id', $id);
$stmt->execute();
printf("Query: [[%s]]\n", $sql);
print implode(' - ', (($r = @$stmt->fetch(PDO::FETCH_NUM)) ? $r : array())) ."\n";
print implode(' - ', $stmt->errorinfo()) ."\n";

print "-------------------------------------------------------\n";

$queries = array(
	"SELECT 1, 'foo' FROM DUAL WHERE 1 = :id AND '\\0' IS NULL AND  2 <> :id",
	"SELECT 1 FROM DUAL WHERE 1 = :id AND '' AND  2 <> :id",
	"SELECT 1 FROM DUAL WHERE 1 = :id AND '\'\'' = '''' AND  2 <> :id",
	"SELECT 1 FROM DUAL WHERE 1 = :id AND '\'' = '''' AND  2 <> :id",
	"SELECT 'a', 'b\'' FROM DUAL WHERE '''' LIKE '\\'' AND 1",
	"SELECT 'a''', '\'b\'' FROM DUAL WHERE '''' LIKE '\\'' AND 1",
	"SELECT UPPER(:id) FROM DUAL WHERE '1'",
	"SELECT 1 FROM DUAL WHERE '\''",
	"SELECT 1 FROM DUAL WHERE :id AND '\\0' OR :id",
	"SELECT 1 FROM DUAL WHERE 'a\\f\\n\\0' AND 1 >= :id",
	"SELECT 1 FROM DUAL WHERE '\'' = ''''",
	"SELECT '\\n' '1 FROM DUAL WHERE '''' and :id'",
	"SELECT 1 'FROM DUAL WHERE :id AND '''' = '''' OR 1 = 1 AND ':id",
);

$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);
$id = 1;

foreach ($queries as $k => $query) {
	$stmt = $db->prepare($query);
	$stmt->bindParam(':id', $id);
	$stmt->execute();
	
	printf("[%d] Query: [[%s]]\n", $k + 1, $query);
	print implode(' - ', (($r = @$stmt->fetch(PDO::FETCH_NUM)) ? $r : array())) ."\n";
	print implode(' - ', $stmt->errorinfo()) ."\n";
	print "--------\n";
}

?>