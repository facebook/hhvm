<?hh // strict

function test(dynamic $foo): void {
  $foo->getSomeValue<ExplicitTypeArgument1>();

  $foo->getSomeValue<ExplicitTypeArgument1, ExplicitTypeArgument2>();

  $foo->getSomeValue<ExplicitTypeArgument1, ExplicitTypeArgument2, ExplicitTypeArgument3>();

  $foo->getSomeValue<ExplicitTypeArgument1, ExplicitTypeArgument2, ExplicitTypeArgument3>(1, 2, 3);

  $foo->getSomeValue<ExplicitTypeArgument1, ExplicitTypeArgument2, ExplicitTypeArgument3>(111111111111111111111111, 222222222222222222222222, 333333333333333333333333);

  $foo->getSomeValue<ExplicitTypeArgument1>()
    ->getSomeValue<ExplicitTypeArgument1, ExplicitTypeArgument2>()
    ->getSomeValue<ExplicitTypeArgument1, ExplicitTypeArgument2, ExplicitTypeArgument3>(1, 2, 3)
    ->getSomeValue<ExplicitTypeArgument1, ExplicitTypeArgument2, ExplicitTypeArgument3>(111111111111111111111111, 222222222222222222222222, 333333333333333333333333);
}
