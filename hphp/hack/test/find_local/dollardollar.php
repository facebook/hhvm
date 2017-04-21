<?hh
function test() {
   (a()
 |>                // introduces $$ 1
   b($$))          // $$ 1
|>                  // introduces $$ 2
 (c(
     (d($$)        // $$ 2  should find
   |>              // introduces $$ 3
     e($$)),       // $$ 3
   $$));           // $$ 2  should find
}
