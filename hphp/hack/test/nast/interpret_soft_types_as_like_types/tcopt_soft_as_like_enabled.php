<<file:__EnableExperimentalFeatures('interpret_soft_types_as_like_types')>>
<?hh

function tcopt_soft_as_like_enabled(<<__Soft>> int $x): @int {
  return $x + 1;
}
