<?hh

function findValue($table, $v)  // where $table is 2x3 array
:mixed{
        for ($row = 0; $row <= 1; ++$row)
        {
                for ($colm = 0; $colm <= 2; ++$colm)
                {
                        if ($table[$row][$colm] == $v)
                        {
                                echo "$v was found at row $row, column $colm\n";
                                return;
                        }
                }
        }

        echo "$v was not found\n";
        return;
}

/*
// Try to jump into a while loop (which is inside a for loop); disallowed

goto label1;
for($i = 1; $j = 20, $i <= 10; $i++)
{
        while($j--)
        {
label1:
                ;
        }
}
*/
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  $table = dict[];
  $table[0] = vec[34];
  $table[0][] = -3;
  $table[0][] = 345;
  $table[1] = vec[123];
  $table[1][] = 9854;
  $table[1][] = -765;

  findValue($table, 123);
  findValue($table, -23);
}
