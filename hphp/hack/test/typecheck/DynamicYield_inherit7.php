<?hh // strict

interface IUseDynamicYield {}
trait DynamicYield {}

interface IFeedObjectIdentifier {}
interface IStoryIdentifier extends IFeedObjectIdentifier {}

interface IEntStory extends IUseDynamicYield {
  public function yieldX(): Awaitable<IStoryIdentifier>;
}

abstract class FeedObjectPreview {
  use DynamicYield;

  public async function yieldX(): Awaitable<IFeedObjectIdentifier> {
    // UNSAFE
  }
}

abstract class StoryPreview extends FeedObjectPreview implements IEntStory {
  public async function yieldX(): Awaitable<IStoryIdentifier> {
    // UNSAFE
  }
}
