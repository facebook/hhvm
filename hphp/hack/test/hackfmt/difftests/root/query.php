<?hh

$matches =
  await SomeQueriableEntityWithAnExtraordinarilyLongIdentifier::entityQuery(
    $context_object,
    $id,
  )
    ->queryEdges()
    ->whereTime(P::lessThanOrEquals((int)time()))
    ->orderByTimeDesc()
    ->take(10)
    |> gen_new_array($$);

return $matches;
