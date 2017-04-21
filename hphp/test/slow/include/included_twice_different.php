<?php

$file = (dirname(__FILE__) . '/included_twice_different1.out');

// If we write a file multiple times really fast then we can modify the file
// during the same time tick as last time and the change won't be
// noticed. Unfortunate but difficult to fix (we'd need an inotify handler which
// is probably overkill).  Thus - the sleep calls in the follow code.  It would
// probably be reasonable to usleep() but some filesystems won't support
// subsecond modification times.

// Ensure that if we include the file multiple times as quick as possible we
// only add it to the get_included_files() list once.  Do this a couple times to
// ensure that we're not getting a false negative when the clock flips over.
for ($i = 0; $i < 2; $i++) {
  if ($i != 0) sleep(1);
  file_put_contents($file, "<?php\n\$j=1;\n");
  include($file);
  include($file);
  echo count(array_filter(get_included_files(), $x ==> $x == $file)) . "\n";
}

$file = (dirname(__FILE__) . '/included_twice_different2.out');

// Ensure that if we write the file multiple times that the file is actually
// re-read and not cached.
file_put_contents($file, "<?php\n\$i=1;\n");
include($file);
echo $i . "\n";
sleep(1);
file_put_contents($file, "<?php\n\$i=2;\n");
include($file);
echo $i . "\n";

?>
