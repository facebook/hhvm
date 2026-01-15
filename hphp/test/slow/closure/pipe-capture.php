<?hh

<<file:__EnableUnstableFeatures('capture_pipe_variables')>>

<<__EntryPoint>>
async function main() {
  var_dump(
    42
      |> (() ==> $$ + 1)
      |> (() ==> $$())
      |> $$() * 2
  );

  var_dump(
    (10 |> (() ==> $$ * 2)()) |> (20 |> (() ==> $$ * 4) |> $$()) + $$
  );

  var_dump(
    (() ==> {
      return 10 |> (() ==> $$ + 1) |> $$();
    })()
  );

  var_dump(
    10 |> (() ==> { return () ==> { return () ==> $$; }; }) |> $$() |> $$() |> $$()
  );

  var_dump(10 |> (5 + (() ==> $$)()));
  var_dump(10 |> (await async { return $$; }));
  var_dump(10 |?> (5 + (() ==> $$)()));
  var_dump(null |?> (5 + (() ==> $$)())); // <?

  "hello"
    |> (() ==> {
         $r = $$.", world"              // $$ is captured from the outer scope ("hello")
           |> \HH\Lib\Str\uppercase($$) // Shadowing: $$ is "hello, world"
           |> $$."!"                    // Shadowing: $$ is "HELLO, WOLRD"
           |> (() ==> $$);              // Shadowing: (() ==> "HELLO, WORLD!")
         return $$.": ".$r();           // $$ is the unshadowed "hello", $r has the lambda
                                        // which captured "HELLO, WORLD!"
       })
    |> var_dump($$()); // We print string(20) "hello: HELLO, WORLD!"
}
