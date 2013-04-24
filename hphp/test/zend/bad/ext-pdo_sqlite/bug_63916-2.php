<?php
$num = PHP_INT_MAX; // 32 bits
$conn = new PDO('sqlite::memory:');
$conn->query('CREATE TABLE users (id INTEGER NOT NULL, num INTEGER NOT NULL, PRIMARY KEY(id))');

$stmt = $conn->prepare('insert into users (id, num) values (:id, :num)');
$stmt->bindValue(':id', 1, PDO::PARAM_INT);
$stmt->bindValue(':num', $num, PDO::PARAM_INT);
$stmt->execute();

$stmt = $conn->query('SELECT num FROM users');
$result = $stmt->fetchAll(PDO::FETCH_COLUMN);

var_dump($num,$result[0]);

?>