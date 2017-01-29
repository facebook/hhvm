<?hh
function f()
{
  // casts
  (x) ($y);
  (x) ++$y;
  (x) --$y;
  (x) new C();
  (x) clone $y;
  (x) await $y;
  (x) include $y;
  (x) include_once $y;
  (x) require $y;
  (x) require_once $y;
  (x) async function () {};
  $y |> (x)$$;
  (x) static::y;
  (x) ~ $y;
  (x) ! $y;
  (x) @ $y;
  (x) list($y);
  (x) array();
  (x) shape("a" => $y);
  (x) function () {};
  (x) $y;
  (x) self::y;
  (x) $this;
  (x) yield $y;
  (x) await $y;
  (x) 123;
  (x) "";
  (x) true;
  (x) null;
  (x) parent::y;
  (x) - $y;
  (x) + $y;
}
function g() {
  // parenthesized expressions
  (x) & $y;
  (x) * $y;
  (x) / $y;
  (x) % $y;
  (x) ^ $y;
  (x) < $y;
  (x) > $y;
  (x) << $y;
  (x) >> $y;
  (x)[$y];
  $y[(x)];
  (x).y;
  (x) ? $y : $z;
  (x);
  (x) and $y;
  (x) or $y;
  (x) xor $y;
  (x) | $y;
  ($y + (x));
  (x) |> $$;
  (x) ?? $y;
  (x) || $y;
  (x) && $y;
  (x) == $y;
  (x) === $y;
  (x) != $y;
  (x) <= $y;
  (x) >= $y;
  (x) ** $y;
  (x) = $y;
  (x) += $y;
  (x) -= $y;
  (x) /= $y;
  (x) **= $y;
  (x) *= $y;
  (x) .= $y;
  (x) %= $y;
  (x) &= $y;
  (x) |= $y;
  (x) ^= $y;
  (x) <<= $y;
  (x) >>= $y;
  (x)->y;
  (x)?->y;
  (x)::y;
  (x) instanceof C;
  $y ? (x) : $y;
  function ( int $x = (x), int $y = 123) {};
  array((x) => 123);
  foreach ((x) as y) { }
}
function h() {
  (int)$x + $y;
}
