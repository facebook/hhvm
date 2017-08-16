<?hh

// This is an anonymized excerpt of a file from www.
// It's syntactically correct and should therefore parse without error.
// It's being included in the full_fidelity tests simply to increase coverage.

type T = shape(
  C::PROPERTY1 => string,
  C::PROPERTY2 => int,
  C::PROPERTY3 => int,
  C::PROPERTY4 => string,
  C::PROPERTY5 => int,
  C::PROPERTY6 => ?int,
  C::PROPERTY7 => ?string,
  C::PROPERTY8 => ?Set<int>,
  C::PROPERTY9 => ?Set<string>,
  C::PROPERTY10 => ?Set<string>,
  C::PROPERTY11 => ?int,
);

abstract final class C{
  const string PROPERTY1 = 'property1';
  const string PROPERTY2 = 'property2';
  const string PROPERTY3 = 'property3';
  const string PROPERTY4 = 'property4';
  const string PROPERTY5 = 'property5';
  const string PROPERTY6 = 'property6';
  const string PROPERTY7 = 'property7';
  const string PROPERTY8 = 'property8';
  const string PROPERTY9 = 'property9';
  const string PROPERTY10 = 'property10';
  const string PROPERTY11 = 'property11';


  public static async function propertyID(
    string $id,
    string $type,
  ): Awaitable<?ID_of<Property>> {
    $intermediate_id = 0;
    switch ($type) {
      case self::PROPERTY1:
        $intermediate_id = await
          (new Something())->genOne($id);
        break;

      case self::PROPERTY2:
        // Comment
        $info = await Something::at()->gen($id);
        // country code needs to be upper cased for whatever reason
        $result1 = Str\uppercase((string)idx($info, SomeConst::PROPERTY));
        $result2 = idx($info, 'code');
        $intermediate_id = await
          (new Something())->genOne($var . ':' . $result2);
        break;

      case self::PROPERTY3:
        // Comment (TODO)
        // Comment comment comment
        // Comment
        $result2 = Str\pad_left($result2, 2, "0");
        $intermediate_id = await
          (new Something())->genOne('STRING:'.$id);
        break;

      case self::PROPERTY4:
        $intermediate_id = await
          (new Something())->genOne($id);
        break;

      case self::PROPERTY5:
        // Comment comment
        // Comment
        if (preg_match('/^\d+$/', $id) == 1) {
          $id = 'STRING:'.$id;
        }
        $intermediate_id = await (new Something())->genOne($id);
        break;

      default:
        FBLogger('Something')->warn('Warning', $type);
    }

    $intermediate_id = maybe_oid($intermediate_id);
    if ($intermediate_id === null) {
      return null;
    }

    // TODO: Something
    $result4 = await Something::genSomething(); /* comment */
    $result5 = await Something::genSomething($input1, $input2);
    $result6 = await $result4->genID();

    return maybe_oid($result6);
  }
}
