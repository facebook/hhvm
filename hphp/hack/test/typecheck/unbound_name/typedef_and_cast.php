<?hh

type Amount = dynamic;

function test(): void {
  (int)((Amount $amount) ==> 0);
}
