<?hh

function print_something_multiple_times($something,$times)
:mixed{
  echo "----\nIn function, printing the string \"$something\" $times times\n";
  for ($i=0; $i<$times; $i++) {
    echo "$i) $something\n";
  }
  echo "Done with function...\n-----\n";
}

function some_other_function()
:mixed{
  echo "This is some other function, to ensure more than just one function works fine...\n";
}


// This is a lie.
<<__EntryPoint>> function main(): void {
echo "Before function declaration...\n";
echo "After function declaration...\n";

echo "Calling function for the first time...\n";
print_something_multiple_times("This works!",10);
echo "Returned from function call...\n";

echo "Calling the function for the second time...\n";
print_something_multiple_times("This like, really works and stuff...",3);
echo "Returned from function call...\n";

some_other_function();
}
