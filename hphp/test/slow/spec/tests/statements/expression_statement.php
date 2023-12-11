<?hh

function DoIt() :mixed{ return 20; }

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
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  $i = 10;// $i is assigned the value 10; result (10) is discarded
  ++$i;   // $i is incremented; result (11) is discarded
  $i++;   // $i is incremented; result (11) is discarded
  DoIt(); // function DoIt is called; result (return value) is discarded

  $i;     // no side effects; result is discarded, so entirely vacuous
  123;    // likewise ...
  34.5 * 12.6 + 11.987;
  TRUE;

  while ($i-- > 0)
  {
      ;       // null statement
  }

  $i = 10;
  while ($i-- > 0)
  {
      continue;   // in this context, equivalent to using a null statement
  }

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
