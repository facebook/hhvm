<?hh

$foo = 'bar
baz'.' qux';

$sql = 'SELECT *
        FROM table
        WHERE foo > bar
        ORDER BY bar DESC';

execute('SELECT *
  FROM table
  WHERE foo > bar
  ORDER BY bar DESC'
);

execute(
  'SELECT *
    FROM table
    WHERE foo > bar
    ORDER BY bar DESC',
  $connection_pool,
);
