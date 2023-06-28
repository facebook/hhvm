<?hh

function a()
:mixed{
  echo "hey\n";
}

function b($i)
:mixed{
  echo "$i\n";
}


function c($i,$j)
:mixed{
  echo "Counting from $i to $j\n";
  for ($k=$i; $k<=$j; $k++) {
    echo "$k\n";
  }
}


function factorial($n)
:mixed{
  if ($n==0 || $n==1) {
    return 1;
  } else {
    return factorial($n-1)*$n;
  }
}


function factorial2($start, $n)
:mixed{
  if ($n<=$start) {
    return $start;
  } else {
    return factorial2($start,$n-1)*$n;
  }
}


function call_fact()
:mixed{
  echo "(it should break at 5...)\n";
  for ($i=0; $i<=10; $i++) {
    if ($i == 5) break;
    $n=factorial($i);
    echo "factorial($i) = $n\n";
  }
}

function return4() :mixed{ return 4; }
function return7() :mixed{ return 7; }

function andi($i, $j)
:mixed{
    for ($k=$i ; $k<=$j ; $k++) {
        if ($k >5) continue;
        echo "$k\n";
    }
}


<<__EntryPoint>> function main(): void {
a();
b("blah");
a();
b("blah","blah");
c(7,14);

a();


for ($k=0; $k<10; $k++) {
  for ($i=0; $i<=10; $i++) {
    $n=factorial($i);
    echo "factorial($i) = $n\n";
  }
}


echo "and now, from a function...\n";

for ($k=0; $k<10; $k++) {
  call_fact();
}

echo "------\n";
$result = factorial(factorial(3));
echo "$result\n";

$result=factorial2(return4(),return7());
echo "$result\n";

andi (3,10);
}
