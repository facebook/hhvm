<?php
$stamp = 1112427000;
print strftime('%c %Z',strtotime('now',$stamp)) ."\n";
print strftime('%c %Z',strtotime('tomorrow',$stamp)) ."\n";
print strftime('%c %Z',strtotime('+1 day',$stamp)) ."\n";
print strftime('%c %Z',strtotime('+2 day',$stamp)) ."\n";
?>