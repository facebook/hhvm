<?hh

function skip_if_not_configured(): void {
  if (!getenv('MYSQL_TEST_HOST')) {
    exit('skip');
  }
}
