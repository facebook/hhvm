<?php
function add_points($player, $points) {
    $player->energy += $points;
    print_r($player);
}
add_points(NULL, 2);