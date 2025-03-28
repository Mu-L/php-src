--TEST--
PDO Common: Bug #38253 (PDO produces segfault with default fetch mode)
--EXTENSIONS--
pdo
--SKIPIF--
<?php
$dir = getenv('REDIR_TEST_DIR');
if (false == $dir) die('skip no driver');
require_once $dir . 'pdo_test.inc';
PDOTest::skip();
?>
--FILE--
<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.__DIR__ . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$pdo = PDOTest::factory();

$pdo->exec ("create table test38253 (id integer primary key, n varchar(255))");
$pdo->exec ("INSERT INTO test38253 (id, n) VALUES (1, 'hi')");

$pdo->setAttribute (PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_CLASS);
$stmt = $pdo->prepare ("SELECT * FROM test38253");
$stmt->execute();
var_dump($stmt->fetchAll());

$pdo->setAttribute (PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_FUNC);
$stmt = $pdo->prepare ("SELECT * FROM test38253");
$stmt->execute();
var_dump($stmt->fetchAll());

$pdo->setAttribute (PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_INTO);
$stmt = $pdo->prepare ("SELECT * FROM test38253");
$stmt->execute();
var_dump($stmt->fetch());

?>
--CLEAN--
<?php
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();
PDOTest::dropTableIfExists($db, "test38253");
?>
--EXPECTF--
Warning: PDOStatement::fetchAll(): SQLSTATE[HY000]: General error: No fetch class specified in %s on line %d

Warning: PDOStatement::fetchAll(): SQLSTATE[HY000]: General error in %s on line %d
array(0) {
}

Warning: PDOStatement::fetchAll(): SQLSTATE[HY000]: General error: No fetch function specified in %s on line %d

Warning: PDOStatement::fetchAll(): SQLSTATE[HY000]: General error in %s on line %d
array(0) {
}

Warning: PDOStatement::fetch(): SQLSTATE[HY000]: General error: No fetch-into object specified. in %s on line %d

Warning: PDOStatement::fetch(): SQLSTATE[HY000]: General error in %s on line %d
bool(false)
