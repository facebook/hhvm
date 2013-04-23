<?php
echo "*** Testing the process isolations between a process and its forks ***\n";

$pid = pcntl_fork();

if ($pid > 0) {
  pcntl_wait($status); 
  echo "father is $pid\n";

  if (!isset($pid2)) 
  {
    echo "father ($pid) doesn't know its grandsons\n";
  }
} 
else 
{
  echo "son ($pid)\n";
  $pid2 = pcntl_fork();
  if ($pid2 > 0)
  {	
    pcntl_wait($status2); 
    echo "son is father of $pid2\n";
  } 
  else 
  {
    echo "grandson ($pid2)\n";
  }
}
?>