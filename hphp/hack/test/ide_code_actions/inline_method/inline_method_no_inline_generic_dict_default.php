<?hh

class InlineGenericDictDefault {
  private function inline_me<T>(dict<string, T> $values = dict<string, T>[]): dict<string, T> {
    return $values;
  }

  public function caller(): dict<string, int> {
    return $this->/*range-start*/inline_me/*range-end*/<int>();
  }
}
