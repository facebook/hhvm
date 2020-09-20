<?hh

$x =
  <ui:selector:option
    value={$value________}
    selected={$label === $selected_value}>
    {$label}
  </ui:selector:option>;

$x =
  <p>
    <ui:selector:option
      value={$value________}
      selected={$label === $selected_value}>
      {$label}
    </ui:selector:option>;
  </p>;

f(
  <ui:selector:option
    value={$value________}
    selected={$label === $selected_value}>
    {$label}
  </ui:selector:option>,
);

f(<ui:selector:option
  value={$value________}
  selected={$label === $selected_value}>
  {$label}
</ui:selector:option>);
