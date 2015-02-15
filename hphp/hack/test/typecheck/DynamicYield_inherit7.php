<?hh // strict

interface IUseDynamicYield {}
trait DynamicYield {}

interface IFeedObjectIdentifier {}
interface IStoryIdentifier extends IFeedObjectIdentifier {}

interface IEntStory extends IUseDynamicYield {
  public function genX(): Awaitable<IStoryIdentifier>;
}

abstract class FeedObjectPreview {
  use DynamicYield;

  public async function genX(): Awaitable<IFeedObjectIdentifier> {
    // UNSAFE
  }
}

abstract class StoryPreview extends FeedObjectPreview implements IEntStory {
  public async function genX(): Awaitable<IStoryIdentifier> {
    // UNSAFE
  }
}
