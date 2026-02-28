<?hh


function doAThing($followable, $comment_settings) :mixed{
  $interaction_settings = 0;
  if ($followable) {
    $interaction_settings |= 0x100;
  }
  if ($comment_settings !== null) {
    $interaction_settings |= (int)($comment_settings);
  }

  return $interaction_settings;
}
<<__EntryPoint>> function main(): void {
doAThing(1, '2');
echo "done\n";
}
