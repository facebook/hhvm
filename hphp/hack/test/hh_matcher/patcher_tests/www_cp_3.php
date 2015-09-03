//// tosearch.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface IGraphQLConfigNamedParam {
  public function getName(): string;
}

abstract class GraphQLConfigParam<T> implements IGraphQLConfigNamedParam {

  public function __construct(
    private string $name,
    private T $value
  ) {}

  public static function create(
    string $type,
    string $name,
    mixed $value,

  ): IGraphQLConfigNamedParam {
    switch ($type) {
      case 'String':
        return new GraphQLStringConfigParam($name, (string)$value);
        break;
      case 'Int':
        return new GraphQLIntConfigParam($name, (int)$value);
        break;
      case 'Float':
        return new GraphQLFloatConfigParam($name, (float)$value);
        break;
      case 'Bool':
        return new GraphQLBoolConfigParam($name, (bool)$value);
        break;
      default:
        invariant_violation('Unknown config param type %s', $type);
    }
  }

  public function getName(): string {
    return $this->name;
  }

  public function getValue(): T {
    return $this->value;
  }
}

class GraphQLStringConfigParam extends GraphQLConfigParam<string> {}

class GraphQLIntConfigParam extends GraphQLConfigParam<int> {}

class GraphQLFloatConfigParam extends GraphQLConfigParam<float> {}

class GraphQLBoolConfigParam extends GraphQLConfigParam<bool> {}

//// pattern.php
<?hh //strict

class __KSTAR {}

class __SOMENODE {
  public function __KSTAR(): void {}

  public function __SOMENODE(): string {
    "__SKIPANY";
    {
      "__KSTAR";
      return "__ANY";
      break;
    }
  }

  public function __KSTAR(): void {}
}

class __KSTAR {}

//// target.php
<?hh //strict

class __KSTAR {}

class __SOMENODE {
  public function __KSTAR(): void {}

  public function __SOMENODE(): string {
    "__SKIPANY";
    {
      "__KSTAR";
      return "__ANY";
      /*DELETE STMTS*//**/
    }
  }

  public function __KSTAR(): void {}
}

class __KSTAR {}
