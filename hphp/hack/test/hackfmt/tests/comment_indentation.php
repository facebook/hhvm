<?hh

bake_bread(
  // rye
);

preheat_oven(
  $temp,
  $time,
  // celsius
);

find_caraway_seed(/* somewhere */);

grease_tin(
  /* nonstick spray */
);

get_electric_mixer(/* plug in */
);

mixer_settings($minutes, $speed /* nullable */);

mixer_settings(/* mins */ $minutes /* nullable */, /* spd */ $speed);

mixer_settings(/*mins*/$minutes/*null*/,/*spd*/$speed/*null*/);

mixer_settings(  /*mins*/  $minutes  /*null*/  ,  /*spd*/  $speed  /*null*/  );

mix_dough(
  $flour,
  $butter,
  /* $molasses */
);

beat_mixture(
  /* looooooooooooooooooooooooooooooooooooooooooooooooooooooong time */
);

knead_dough(
  $smoothness_to_be_obtained,
  $time_to_be_spent_kneading,
  /* usually about 5 mins */
);

punch_dough($violence, $duration
/**
 * This duration is important.
 */);


brush_with(
  $cooking_oil,
  $generousness,
/**
 * Don't be too generous.
 */
);

slash_top(
  $knife,
  $times,
  /**
   * Only a few times needed.
   */
);

emptee($oven /* yep */);

emptee(
  $tin
  /* nope */
);

emptee(
  /* let */
  $bread
  /* rise */
);

function isKitchenEmpty(): bool {
  return
    // check oven first
    $this->oven_contents === '' &&
    $this->tin_contents === null &&
    $this->mixer_contents === null &&
    $this->items_on_counter === 0;
  // now we've checked everything
}
