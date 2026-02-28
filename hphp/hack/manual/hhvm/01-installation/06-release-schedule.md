# Release Schedule

We release biweekly (every two weeks), promoting a recent
nightly build. The majority of releases are supported for 6 weeks.

## Long Term Support

Every 24 weeks (roughly twice a year), a release has "long term support" for
roughly 48 weeks - until two more LTS releases have been made. If the release
due at that time is delayed, support is extended.

Here's a table of recent LTS releases as well as *expected* future LTS releases:

| Version |    \*Release Date       |    \*End of Support       |      MacOS Homebrew Support       |
| ------- | ----------------------- | ------------------------- | --------------------------------- |
| [4.153¹](https://hhvm.com/blog/2022/03/17/hhvm-4.153.html) | March 17, 2022        | *February 16, 2023*      | Supported     |
| [4.128¹](https://hhvm.com/blog/2021/09/21/hhvm-4.128.html) | September 21, 2021    | *August 24, 2022*        | Supported     |
| [4.102²](https://hhvm.com/blog/2021/03/23/hhvm-4.102.html) | March 23, 2021        | March 17, 2022           | Not Supported |
| [4.80](https://hhvm.com/blog/2020/10/21/hhvm-4.80.html)    | October 21, 2020      | September 21, 2021       | Not Supported |
| [4.56](https://hhvm.com/blog/2020/05/04/hhvm-4.56.html)    | May 4, 2020           | April 5, 2021            | Not Supported |
| [4.32](https://hhvm.com/blog/2019/11/19/hhvm-4.32.html)    | November 19, 2019     | October 21, 2020         | Not Supported |


\* *Dates in italics signify an **expected** date.*

¹ We are continuing support for MacOS Homebrew on LTS releases (4.128 and 4.153) until they reach EOL, but [we are stopping nightly builds of HHVM on MacOS homebrew](https://hhvm.com/blog/2022/06/17/deprecating-homebrew.html), will no longer be publishing MacOS homebrew packages for new releases, and will not be testing new versions of HHVM on MacOS going forward.

² HHVM 4.102 was promoted to lts status instead of HHVM 4.104. Its expected support duration has been extended by 2 weeks (to a total of 50) to compensate for the "early start". See the [blog post](https://hhvm.com/blog/2021/03/29/extending-hhvm-4.102-support.html) for more information.
