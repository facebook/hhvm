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
  $box0000000 = new Box();
  $box0000001 = new Box();
  $box0000010 = new Box();
  $box0000011 = new Box();
  $box0000100 = new Box();
  $box0000101 = new Box();
  $box0000110 = new Box();
  $box0000111 = new Box();
  $box0001000 = new Box();
  $box0001001 = new Box();
  $box0001010 = new Box();
  $box0001011 = new Box();
  $box0001100 = new Box();
  $box0001101 = new Box();
  $box0001110 = new Box();
  $box0001111 = new Box();
  $box0010000 = new Box();
  $box0010001 = new Box();
  $box0010010 = new Box();
  $box0010011 = new Box();
  $box0010100 = new Box();
  $box0010101 = new Box();
  $box0010110 = new Box();
  $box0010111 = new Box();
  $box0011000 = new Box();
  $box0011001 = new Box();
  $box0011010 = new Box();
  $box0011011 = new Box();
  $box0011100 = new Box();
  $box0011101 = new Box();
  $box0011110 = new Box();
  $box0011111 = new Box();
  $box0100000 = new Box();
  $box0100001 = new Box();
  $box0100010 = new Box();
  $box0100011 = new Box();
  $box0100100 = new Box();
  $box0100101 = new Box();
  $box0100110 = new Box();
  $box0100111 = new Box();
  $box0101000 = new Box();
  $box0101001 = new Box();
  $box0101010 = new Box();
  $box0101011 = new Box();
  $box0101100 = new Box();
  $box0101101 = new Box();
  $box0101110 = new Box();
  $box0101111 = new Box();
  $box0110000 = new Box();
  $box0110001 = new Box();
  $box0110010 = new Box();
  $box0110011 = new Box();
  $box0110100 = new Box();
  $box0110101 = new Box();
  $box0110110 = new Box();
  $box0110111 = new Box();
  $box0111000 = new Box();
  $box0111001 = new Box();
  $box0111010 = new Box();
  $box0111011 = new Box();
  $box0111100 = new Box();
  $box0111101 = new Box();
  $box0111110 = new Box();
  $box0111111 = new Box();
  $box1000000 = new Box();
  $box1000001 = new Box();
  $box1000010 = new Box();
  $box1000011 = new Box();
  $box1000100 = new Box();
  $box1000101 = new Box();
  $box1000110 = new Box();
  $box1000111 = new Box();
  $box1001000 = new Box();
  $box1001001 = new Box();
  $box1001010 = new Box();
  $box1001011 = new Box();
  $box1001100 = new Box();
  $box1001101 = new Box();
  $box1001110 = new Box();
  $box1001111 = new Box();
  $box1010000 = new Box();
  $box1010001 = new Box();
  $box1010010 = new Box();
  $box1010011 = new Box();
  $box1010100 = new Box();
  $box1010101 = new Box();
  $box1010110 = new Box();
  $box1010111 = new Box();
  $box1011000 = new Box();
  $box1011001 = new Box();
  $box1011010 = new Box();
  $box1011011 = new Box();
  $box1011100 = new Box();
  $box1011101 = new Box();
  $box1011110 = new Box();
  $box1011111 = new Box();
  $box1100000 = new Box();
  $box1100001 = new Box();
  $box1100010 = new Box();
  $box1100011 = new Box();
  $box1100100 = new Box();
  $box1100101 = new Box();
  $box1100110 = new Box();
  $box1100111 = new Box();
  $box1101000 = new Box();
  $box1101001 = new Box();
  $box1101010 = new Box();
  $box1101011 = new Box();
  $box1101100 = new Box();
  $box1101101 = new Box();
  $box1101110 = new Box();
  $box1101111 = new Box();
  $box1110000 = new Box();
  $box1110001 = new Box();
  $box1110010 = new Box();
  $box1110011 = new Box();
  $box1110100 = new Box();
  $box1110101 = new Box();
  $box1110110 = new Box();
  $box1110111 = new Box();
  $box1111000 = new Box();
  $box1111001 = new Box();
  $box1111010 = new Box();
  $box1111011 = new Box();
  $box1111100 = new Box();
  $box1111101 = new Box();
  $box1111110 = new Box();
  $box1111111 = new Box();

  // put an int in the bottom box:
  $box0000000->put(0);
  // require an in from the top box:
  expect_int($box1111111->get());

  // connect every 2 vars together
  // by connecting 2i with 2i+1:
  // 0--1, 2--3, 4--5...
  $box0000001->put($box0000000->get());
  $box0000011->put($box0000010->get());
  $box0000101->put($box0000100->get());
  $box0000111->put($box0000110->get());
  $box0001001->put($box0001000->get());
  $box0001011->put($box0001010->get());
  $box0001101->put($box0001100->get());
  $box0001111->put($box0001110->get());
  $box0010001->put($box0010000->get());
  $box0010011->put($box0010010->get());
  $box0010101->put($box0010100->get());
  $box0010111->put($box0010110->get());
  $box0011001->put($box0011000->get());
  $box0011011->put($box0011010->get());
  $box0011101->put($box0011100->get());
  $box0011111->put($box0011110->get());
  $box0100001->put($box0100000->get());
  $box0100011->put($box0100010->get());
  $box0100101->put($box0100100->get());
  $box0100111->put($box0100110->get());
  $box0101001->put($box0101000->get());
  $box0101011->put($box0101010->get());
  $box0101101->put($box0101100->get());
  $box0101111->put($box0101110->get());
  $box0110001->put($box0110000->get());
  $box0110011->put($box0110010->get());
  $box0110101->put($box0110100->get());
  $box0110111->put($box0110110->get());
  $box0111001->put($box0111000->get());
  $box0111011->put($box0111010->get());
  $box0111101->put($box0111100->get());
  $box0111111->put($box0111110->get());
  $box1000001->put($box1000000->get());
  $box1000011->put($box1000010->get());
  $box1000101->put($box1000100->get());
  $box1000111->put($box1000110->get());
  $box1001001->put($box1001000->get());
  $box1001011->put($box1001010->get());
  $box1001101->put($box1001100->get());
  $box1001111->put($box1001110->get());
  $box1010001->put($box1010000->get());
  $box1010011->put($box1010010->get());
  $box1010101->put($box1010100->get());
  $box1010111->put($box1010110->get());
  $box1011001->put($box1011000->get());
  $box1011011->put($box1011010->get());
  $box1011101->put($box1011100->get());
  $box1011111->put($box1011110->get());
  $box1100001->put($box1100000->get());
  $box1100011->put($box1100010->get());
  $box1100101->put($box1100100->get());
  $box1100111->put($box1100110->get());
  $box1101001->put($box1101000->get());
  $box1101011->put($box1101010->get());
  $box1101101->put($box1101100->get());
  $box1101111->put($box1101110->get());
  $box1110001->put($box1110000->get());
  $box1110011->put($box1110010->get());
  $box1110101->put($box1110100->get());
  $box1110111->put($box1110110->get());
  $box1111001->put($box1111000->get());
  $box1111011->put($box1111010->get());
  $box1111101->put($box1111100->get());
  $box1111111->put($box1111110->get());

  // connect every 4 vars together
  // by connecting 4i+1 with 4i+2:
  // 1--2, 5--6 ...
  $box0000010->put($box0000001->get());
  $box0000110->put($box0000101->get());
  $box0001010->put($box0001001->get());
  $box0001110->put($box0001101->get());
  $box0010010->put($box0010001->get());
  $box0010110->put($box0010101->get());
  $box0011010->put($box0011001->get());
  $box0011110->put($box0011101->get());
  $box0100010->put($box0100001->get());
  $box0100110->put($box0100101->get());
  $box0101010->put($box0101001->get());
  $box0101110->put($box0101101->get());
  $box0110010->put($box0110001->get());
  $box0110110->put($box0110101->get());
  $box0111010->put($box0111001->get());
  $box0111110->put($box0111101->get());
  $box1000010->put($box1000001->get());
  $box1000110->put($box1000101->get());
  $box1001010->put($box1001001->get());
  $box1001110->put($box1001101->get());
  $box1010010->put($box1010001->get());
  $box1010110->put($box1010101->get());
  $box1011010->put($box1011001->get());
  $box1011110->put($box1011101->get());
  $box1100010->put($box1100001->get());
  $box1100110->put($box1100101->get());
  $box1101010->put($box1101001->get());
  $box1101110->put($box1101101->get());
  $box1110010->put($box1110001->get());
  $box1110110->put($box1110101->get());
  $box1111010->put($box1111001->get());
  $box1111110->put($box1111101->get());

  // connect every 8 vars together
  // by connecting 8i+3 with 8i+4:
  // 3--4, 11--12...
  $box0000100->put($box0000011->get());
  $box0001100->put($box0001011->get());
  $box0010100->put($box0010011->get());
  $box0011100->put($box0011011->get());
  $box0100100->put($box0100011->get());
  $box0101100->put($box0101011->get());
  $box0110100->put($box0110011->get());
  $box0111100->put($box0111011->get());
  $box1000100->put($box1000011->get());
  $box1001100->put($box1001011->get());
  $box1010100->put($box1010011->get());
  $box1011100->put($box1011011->get());
  $box1100100->put($box1100011->get());
  $box1101100->put($box1101011->get());
  $box1110100->put($box1110011->get());
  $box1111100->put($box1111011->get());

  // connect every 16 vars together
  // by connecting 16i+7 with 16i+8:
  // 7--8, 23--24...
  $box0001000->put($box0000111->get());
  $box0011000->put($box0010111->get());
  $box0101000->put($box0100111->get());
  $box0111000->put($box0110111->get());
  $box1001000->put($box1000111->get());
  $box1011000->put($box1010111->get());
  $box1101000->put($box1100111->get());
  $box1111000->put($box1110111->get());

  // connect every 32 vars together
  // by connecting 32i+15 with 32i+16:
  // 15--16...
  $box0010000->put($box0001111->get());
  $box0110000->put($box0101111->get());
  $box1010000->put($box1001111->get());
  $box1110000->put($box1101111->get());

  // and so on...
  $box0100000->put($box0011111->get());
  $box1100000->put($box1011111->get());

  $box1000000->put($box0111111->get());
}

function expect_int(int $_): void {}
