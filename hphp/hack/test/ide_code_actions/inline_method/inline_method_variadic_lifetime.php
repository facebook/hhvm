<?hh
//

class InlineVariadicLifetimeBox {
  public function __construct(public string $name) {}
}

class InlineVariadicLifetime {
  private function inline_me(InlineVariadicLifetimeBox ...$rest): void {
    $rest[0] = new InlineVariadicLifetimeBox('new');
    echo 'body';
  }

  public function caller(): void {
    $this->/*range-start*/inline_me/*range-end*/(new InlineVariadicLifetimeBox('old'));
  }
}
//
