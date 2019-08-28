<?hh
// by legolas558
<<__EntryPoint>> function main(): void {
$regex = '/(insert|drop|create|select|delete|update)([^;\']*('."('[^']*')+".')?)*(;|$)/i';

$sql = 'SELECT * FROM #__components';

  $m = null;
  if (preg_match_with_matches($regex, $sql, inout $m)) echo 'matched';
  else echo 'not matched';

print_r($m);
}
