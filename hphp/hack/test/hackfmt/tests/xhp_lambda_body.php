<?hh
$items = $items->map(
  $item ==> <li class={cx('myOrderedListService/item')}>
    <fb:ordered-list-service:ordered-list:item item={$item} />
  </li>,
);
