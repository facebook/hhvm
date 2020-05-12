<?hh

class Box<T> {
  public function __construct() {}
  public function put(T $_): void {}
  public function get(): T {
    throw new Exception();
  }
}

function main(): void {
  // create 2^N type variables using boxes:
  // Tip: increase N by copy pasting and column editing.
  $box00000 = new Box();
  $box00001 = new Box();
  $box00010 = new Box();
  $box00011 = new Box();
  $box00100 = new Box();
  $box00101 = new Box();
  $box00110 = new Box();
  $box00111 = new Box();
  $box01000 = new Box();
  $box01001 = new Box();
  $box01010 = new Box();
  $box01011 = new Box();
  $box01100 = new Box();
  $box01101 = new Box();
  $box01110 = new Box();
  $box01111 = new Box();
  $box10000 = new Box();
  $box10001 = new Box();
  $box10010 = new Box();
  $box10011 = new Box();
  $box10100 = new Box();
  $box10101 = new Box();
  $box10110 = new Box();
  $box10111 = new Box();
  $box11000 = new Box();
  $box11001 = new Box();
  $box11010 = new Box();
  $box11011 = new Box();
  $box11100 = new Box();
  $box11101 = new Box();
  $box11110 = new Box();
  $box11111 = new Box();

  // put an int in the bottom box:
  $box00000->put(0);
  // require an in from the top box:
  expect_int($box11111->get());

  // connect every 2 vars together
  // by connecting 2i with 2i+1:
  // 0--1, 2--3, 4--5...
  $box00001->put($box00000->get());
  $box00011->put($box00010->get());
  $box00101->put($box00100->get());
  $box00111->put($box00110->get());
  $box01001->put($box01000->get());
  $box01011->put($box01010->get());
  $box01101->put($box01100->get());
  $box01111->put($box01110->get());
  $box10001->put($box10000->get());
  $box10011->put($box10010->get());
  $box10101->put($box10100->get());
  $box10111->put($box10110->get());
  $box11001->put($box11000->get());
  $box11011->put($box11010->get());
  $box11101->put($box11100->get());
  $box11111->put($box11110->get());

  // connect every 4 vars together
  // by connecting 4i+1 with 4i+2:
  // 1--2, 5--6 ...
  $box00010->put($box00001->get());
  $box00110->put($box00101->get());
  $box01010->put($box01001->get());
  $box01110->put($box01101->get());
  $box10010->put($box10001->get());
  $box10110->put($box10101->get());
  $box11010->put($box11001->get());
  $box11110->put($box11101->get());

  // connect every 8 vars together
  // by connecting 8i+3 with 8i+4:
  // 3--4, 11--12...
  $box00100->put($box00011->get());
  $box01100->put($box01011->get());
  $box10100->put($box10011->get());
  $box11100->put($box11011->get());

  // connect every 16 vars together
  // by connecting 16i+7 with 16i+8:
  // 7--8, 23--24...
  $box01000->put($box00111->get());
  $box11000->put($box10111->get());

  // connect every 32 vars together
  // by connecting 32i+15 with 32i+16:
  // 15--16...
  $box10000->put($box01111->get());

}

function expect_int(int $_): void {}
