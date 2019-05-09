<?php
<<__EntryPoint>> function main() {
$db = new PDO( 'sqlite::memory:');

$db->sqliteCreateFunction('bar-alias', 'bar');
}
