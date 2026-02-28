////main.php
<?hh

function good(NTIntA $a, NTIntB $b): void {
  if ($a === $b) { echo 'can happen'; }
}

function bad(NTIntA $a, NTStringB $b): void {
  if ($a === $b) { echo 'cannot happen'; }
}

////a.php
<?hh

newtype NTIntA = int;

////b.php
<?hh

newtype NTStringB = string;
newtype NTIntB = int;
