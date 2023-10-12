<?hh // strict

// Should be on a single line
if (true) print("hi");

// Should not be on a single line
if (true)
print("hi");

if (true)
  print("hi");

if (true) {
print("this is a");
print("multi line if");
}

if (true)
print("this is not a");
print("multi line if");

// Should all have the if body on a new line
if (true) this_is_a_very_long_function_call_that_really_exceeds_the_line_length_lorem_ipsum_dolor_sit_amet_consectetur_adipiscing_elit();

if (true)
this_is_a_very_long_function_call_that_really_exceeds_the_line_length_lorem_ipsum_dolor_sit_amet_consectetur_adipiscing_elit();

if (true)
  this_is_a_very_long_function_call_that_really_exceeds_the_line_length_lorem_ipsum_dolor_sit_amet_consectetur_adipiscing_elit();

// Should preserve the single line if
f(g($a, $b), () ==> {
  if (true) print('hi');
  if (true)
  print('hi');
});

if (true) {
print('hi');
if (true) print('hi');
}

if ($a)
f();
else if ($b)
g();
else
h();

if ($a) f();
else if ($b) g();
else h();

if ($a)
f();
else if ($b) {
g();
} else
h();

if ($a)
f();
else if ($b) {
g();
} else{
h();
}

if ($a)
f();
else if ($b) {
g();
g2();
} else
h();

if ($a) f();
else if ($b) {
g();
g2();
} else h();

if ($this->head->isEmpty()) return new MyQueue($this->head->push($item), $this->tail);
else return new MyQueue($this->head, $this->tail->push($item));

if ($this->head->isEmpty())
return new MyQueue($this->head->push($item), $this->tail);
else
return new MyQueue($this->head, $this->tail->push($item));
