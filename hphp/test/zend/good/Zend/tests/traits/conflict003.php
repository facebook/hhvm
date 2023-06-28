<?hh

trait A {
   public function smallTalk() :mixed{
     echo 'a';
   }
   public function bigTalk() :mixed{
     echo 'A';
   }
}

trait B {
   public function smallTalk() :mixed{
     echo 'b';
   }
   public function bigTalk() :mixed{
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
