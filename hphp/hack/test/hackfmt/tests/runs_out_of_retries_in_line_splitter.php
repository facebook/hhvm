<?hh

class Foo {
  public static function foo() {
    return $doughnutFryer
      ->start()
      ->then($_ ==> $frosting_glazer->start())
      ->then($_ ==>
        Future::wait(vec[
          $conveyor_belts->start(),
          $sprinkle_sprinkler->start(),
          $sauce_dripper->start(),
        ]))
      ->catchError($cannot_get_conveyor_belt_running)
      ->then($_ ==> $tell_everyone_donuts_are_just_about_done())
      ->then($_ ==>
        Future::wait(vec[
          $croissant_factory->start(),
          $giant_baking_ovens->start(),
          $butter_butterer->start(),
        ])
          ->catchError($handle_baking_failures)
          ->timeout($script_load_timeout, $handle_baking_failures)
          ->catchError($cannot_get_conveyor_belt_running))
      ->catchError($cannot_get_conveyor_belt_running)
      ->restart()
      ->then($_ ==> $frosting_glazer->start())
      ->then($_ ==>
        Future::wait(vec[
          $conveyor_belts->start(),
          $sprinkle_sprinkler->start(),
          $sauce_dripper->start(),
        ]))
      ->catchError($cannot_get_conveyor_belt_running)
      ->then($_ ==> $tell_everyone_donuts_are_just_about_done())
      ->then($_ ==>
        Future::wait(vec[
          $croissant_factory->start(),
          $giant_baking_ovens->start(),
          $butter_butterer->start(),
        ])
          ->catchError($handle_baking_failures)
          ->timeout($script_load_timeout, $handle_baking_failures)
          ->catchError($cannot_get_conveyor_belt_running))
      ->catchError($cannot_get_conveyor_belt_running)
      ->restart()
      ->then($_ ==> $frosting_glazer->start())
      ->then($_ ==>
        Future::wait(vec[
          $conveyor_belts->start(),
          $sprinkle_sprinkler->start(),
          $sauce_dripper->start(),
        ]))
      ->catchError($cannot_get_conveyor_belt_running)
      ->then($_ ==> $tell_everyone_donuts_are_just_about_done())
      ->then($_ ==>
        Future::wait(vec[
          $croissant_factory->start(),
          $giant_baking_ovens->start(),
          $butter_butterer->start(),
        ])
          ->catchError($handle_baking_failures)
          ->timeout($script_load_timeout, $handle_baking_failures)
          ->catchError($cannot_get_conveyor_belt_running))
      ->catchError($cannot_get_conveyor_belt_running)
      ->then($_ ==> $tell_everyone_donuts_are_just_about_done())
      ->then($_ ==>
        Future::wait(vec[
          $croissant_factory->start(),
          $giant_baking_ovens->start(),
          $butter_butterer->start(),
        ])
          ->catchError($handle_baking_failures)
          ->timeout($script_load_timeout, $handle_baking_failures)
          ->catchError($cannot_get_conveyor_belt_running))
      ->catchError($cannot_get_conveyor_belt_running)
      ->restart()
      ->then($_ ==> $frosting_glazer->start())
      ->then($_ ==>
        Future::wait(vec[
          $conveyor_belts->start(),
          $sprinkle_sprinkler->start(),
          $sauce_dripper->start(),
        ]))
      ->catchError($cannot_get_conveyor_belt_running)
      ->then($_ ==> $tell_everyone_donuts_are_just_about_done())
      ->then($_ ==>
        Future::wait(vec[
          $croissant_factory->start(),
          $giant_baking_ovens->start(),
          $butter_butterer->start(),
        ])
          ->catchError($handle_baking_failures)
          ->timeout($script_load_timeout, $handle_baking_failures)
          ->catchError($cannot_get_conveyor_belt_running))
      ->catchError($cannot_get_conveyor_belt_running)
      ->then($_ ==> {
        Logger::info("Let's eat!");
      });
  }
}
