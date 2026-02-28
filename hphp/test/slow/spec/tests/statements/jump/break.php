<?hh

function findValue($table, $v)  // where $table is 2x3 array
:mixed{
        $valid = true;
        for ($row = 0; $valid && $row <= 1; ++$row)
        {
                for ($colm = 0; $colm <= 2; ++$colm)
                {
                        if ($table[$row][$colm] == $v)
                        {
                                echo "$v was found at row $row, column $colm\n";
                                $valid = false;
                                break; // yes, I know it goes to the wrong place
                        }
                }
        }

        echo "$v was not found\n";
        return;
}

// break;  // can't break from the outer-most level

function f($i)
:mixed{
        echo "$i\n";
        break;  // break is not rejected here until runtime
}
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

  //f(12);        // fails

  for ($i = 1; $i <= 3; ++$i)
  {
  //        f($i);        // fails
  }

  for ($i = 10; $i <= 40; $i +=10)
  {
          echo "\n\$i = $i: ";
          $break_after_switch = false;
          switch($i)
          {
          case 10: echo "ten"; break;
          case 20: echo "twenty"; $break_after_switch = true; break;
          case 30: echo "thirty"; break;
          }
          if ($break_after_switch) break;
          echo "\nJust beyond the switch";
  }
  echo "\n----------\n";
}
