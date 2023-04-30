<?hh <<__EntryPoint>> function main(): void {
$tmp_sqllite = tempnam(sys_get_temp_dir(), 'vmpdotest');
$source = "sqlite:$tmp_sqllite";
$dbh = new PDO($source);
$stmt = $dbh->query("SELECT 1+1");
echo $stmt->queryString;

unset($stmt);
unset($dbh);
unlink($tmp_sqllite);
}
