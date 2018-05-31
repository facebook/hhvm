<?hh

list($notice, $rendered_notif) = await genva($this->genNotice($vc), $renderer->genRenderNotification());

list($a, $b) = await genva($some_generator_of_a->genAnAwaitableOfA(), $some_generator_of_b->genAnAwaitableOfB());
