<?hh

// What happens when the attribute is on an already-public member?
// This is pointless but should it error or just be silently ignored?

class AlreadyPublic {
  <<__TestsBypassVisibility>>
  public function pub_method(): void {}

  <<__TestsBypassVisibility>>
  public int $pub_prop = 0;
}
