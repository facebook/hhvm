<?hh

class G_DuplicateTest extends WWWTest {
  public function test(): void {
    G_CallsTwice::callsFooTwice();
  }
}
