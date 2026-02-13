<?hh

use HH\Asio;

async function bar(): Awaitable<void> {
	$s = new \HH\Lib\Async\Semaphore(10, async (bool $should_wait) ==> {
		if ($should_wait) {
			await Asio\usleep(20000*1000);
		} else {
			await Asio\later();
		}
	});

	concurrent {
		await $s->waitForAsync(true);
		await async {
			for ($i = 0; $i < 1000000; $i++) {
				await $s->waitForAsync(false);
			}

			echo "$i\n";
		};
	}
}

<<__EntryPoint>>
function main(): void {
	Asio\join(bar());
}
