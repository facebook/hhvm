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
    use A, B {
		B::smallTalk insteadof A; 
		A::bigTalk insteadof B;
		B::bigTalk as talk;
	}
}

<<__EntryPoint>>
function main_entry(): void {
  error_reporting(E_ALL);

  $t = new Talker;
  $t->smallTalk();
  $t->bigTalk();
  $t->talk();
}
