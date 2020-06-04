<?hh

class TraitsTest {
	use THello1;
	use THello2;
	public function getHello() {
	    return $this->hello;
	}
}
