/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 */

/* eslint-disable */

import clsx from 'clsx';
import Heading from '@theme/Heading';

// NOTE: Here, we are swizzling NotFound/Content instead of NotFound seeing
// that Docusaurus will only render the NotFound/Content component to avoid
// a double layout issue; cf. https://bit.ly/4q2LEhi.
//
// That said, this Content sub-component is marked as 'unsafe' to swizzle [1],
// as opposed to its parent NotFound-component, per the following command:
// > yarn swizzle @docusaurus/theme-classic --list
//
// This means that the theme component (marked as unsafe) could potentially
// change in backward-incompatible ways between theme minor versions. So when
// upgrading the theme (or Docusaurus), this customisation might in the
// future behave unexpectedly.
//
// [1] https://docusaurus.io/docs/swizzling#what-is-safe-to-swizzle

export default function ContentWrapper(props) {

  function cleanupSearchString(value) {
    let cleanedValue = decodeURIComponent(value);
    ['/', '#', '-', '_', 'hack', 'docs'].forEach((char) => {
      cleanedValue = cleanedValue.replaceAll(char, ' ');
    });
    return cleanedValue.trim();
  }

  function triggerChangeEvents(field) {
    field.dispatchEvent(new Event('input', { bubbles: true }));
    field.dispatchEvent(new Event('change', { bubbles: true }));
  }

  function setAndFocusSearchField() {
    const searchField = document.getElementById('search_input_react');
    const searchString = window.location.pathname;
    searchField.focus();
    searchField.value = '';
    triggerChangeEvents(searchField);
    searchField.value = cleanupSearchString(searchString);
    triggerChangeEvents(searchField);
  }

  return (
    <>
      <main className={clsx('container margin-vert--xl', props.className)}>
        <div className="row">
          <div className="col col--6 col--offset-3">
            <Heading as="h1" className="hero__title">
              Page Not Found
            </Heading>
            <p>
              We could not find what you were looking for. Please check the URL for any typos or
              try <a href="#" onClick={setAndFocusSearchField}><b>using the search bar</b></a>.
            </p>
          </div>
        </div>
      </main>
    </>
  );
}
