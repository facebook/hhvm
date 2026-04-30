# Examples

Here are some examples representing a slew of possible async scenarios. Obviously, this does not cover all possible situations, but they
provide an idea of how and where async can be used effectively. Some of these examples are found spread out through the rest of the async
documentation; they are added here again for consolidation purposes.

## Basic

This example shows the basic tenets of async, particularly the keywords used:

```hack
// async specifies a function will return an awaitable. Awaitable<string> means
// that the awaitable will ultimately return a string when complete
async function trivial(): Awaitable<string> {
  return "Hello";
}

<<__EntryPoint>>
async function call_trivial(): Awaitable<void> {
  // These first two lines could be combined into
  //     $result = await trivial();
  // but wanted to show the process

  // get awaitable that you can wait for completion
  $aw = trivial();
  // wait for the operation to complete and get the result
  $result = await $aw;
  echo $result; // "Hello"
}
```

## Async Closures and Lambdas

Closure and lambda expressions can involve async functions:

```hack
<<__EntryPoint>>
async function closure_async(): Awaitable<void> {
  // closure
  $hello = async function(): Awaitable<string> {
    return 'Hello';
  };
  // lambda
  $bye = async ($str) ==> $str;

  // The call style to either closure or lambda is the same
  $rh = await $hello();
  $rb = await $bye("bye");

  echo $rh." ".$rb.\PHP_EOL;
}
```

## Data Fetching

This shows a way to organize async functions such that we have a nice clean data dependency graph:

```hack
class PostData {
  // using constructor argument promotion
  public function __construct(public string $text) {}
}

async function fetch_all_post_ids_for_author(
  int $author_id,
): Awaitable<vec<int>> {

  // Query database, etc., but for now, just return made up stuff
  return vec[4, 53, 99];
}

async function fetch_post_data(int $post_id): Awaitable<PostData> {
  // Query database, etc. but for now, return something random
  return new PostData(\str_shuffle("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
}

async function fetch_comment_count(int $post_id): Awaitable<int> {
  // Query database, etc., but for now, return something random
  return \rand(0, 50);
}

async function fetch_page_data(
  int $author_id,
): Awaitable<vec<(PostData, int)>> {

  $all_post_ids = await fetch_all_post_ids_for_author($author_id);
  // An async closure that will turn a post ID into a tuple of
  // post data and comment count
  $post_fetcher = async function(int $post_id): Awaitable<(PostData, int)> {
    concurrent {
      $post_data = await fetch_post_data($post_id);
      $comment_count = await fetch_comment_count($post_id);
    }
    return tuple($post_data, $comment_count);
    // alternatively:
    return tuple(
      await fetch_post_data($post_id),
      await fetch_comment_count($post_id),
    );
  };

  // Transform the array of post IDs into a vec of results,
  // using the Vec\map_async function
  return await Vec\map_async($all_post_ids, $post_fetcher);
}

async function generate_page(int $author_id): Awaitable<string> {
  $tuples = await fetch_page_data($author_id);
  $page = "";
  foreach ($tuples as $tuple) {
    list($post_data, $comment_count) = $tuple;
    // Normally render the data into HTML, but for now, just create a
    // normal string
    $page .= $post_data->text." ".$comment_count.\PHP_EOL;
  }
  return $page;
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  print await generate_page(13324); // just made up a user id
}
```
