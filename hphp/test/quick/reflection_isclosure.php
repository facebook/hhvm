<?hh

<<__DynamicallyCallable>>
function not_a_closure() :mixed{
  return 1;
}

<<__DynamicallyCallable>>
function is_a_generator() :AsyncGenerator<mixed,mixed,void>{
  yield 1;
  yield 2;
}

<<__EntryPoint>>
function main_entry(): void {

  $rf = new ReflectionFunction('not_a_closure');
  var_dump($rf->isClosure());
  var_dump($rf->isGenerator());

  $rf = new ReflectionFunction('is_a_generator');
  var_dump($rf->isClosure());
  var_dump($rf->isGenerator());

  $cl = function() {
    return 1;
  };
  $rf = new ReflectionFunction($cl);
  var_dump($rf->isClosure());
  var_dump($rf->isGenerator());

  $cl = function() {
    yield 1;
    yield 2;
  };
  $rf = new ReflectionFunction($cl);
  var_dump($rf->isClosure());
  var_dump($rf->isGenerator());
}
