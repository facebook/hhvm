<?hh
<<__EntryPoint>> function main(): void {
for ($i=0; $i<=5; $i++)
{
  echo "i=$i\n";
  $break_outer = false;
  switch($i) {
    case 0:
      echo "In branch 0\n";
      break;
    case 1:
      echo "In branch 1\n";
      break;
    case 2:
      echo "In branch 2\n";
      break;
    case 3:
      echo "In branch 3\n";
      $break_outer = true;
      break;
    case 4:
      echo "In branch 4\n";
      break;
    default:
      echo "In default\n";
      break;
  }
  if ($break_outer) break;
}
echo "hi\n";
}
