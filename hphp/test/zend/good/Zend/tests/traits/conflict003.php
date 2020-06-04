<?hh

trait A {
   public function smallTalk() {
     echo 'a';
   }
   public function bigTalk() {
     echo 'A';
   }
}

trait B {
   public function smallTalk() {
     echo 'b';
   }
   public function bigTalk() {
     echo 'B';
   }
}

class Talker {
   use A, B;
}

<<__EntryPoint>>
function entrypoint_conflict003(): void {
  error_reporting(E_ALL);
}
