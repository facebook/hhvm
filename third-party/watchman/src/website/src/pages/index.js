/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

import React from 'react';
import clsx from 'clsx';
import Layout from '@theme/Layout';
import CodeBlock from '@theme/CodeBlock';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import styles from './index.module.css';
import WatchmanLogo from '@site/static/img/logo.png';

function HomepageHeader() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <header className={clsx('hero hero--primary', styles.heroBanner)}>
      <div className="container">
        <img src={WatchmanLogo} alt="Watchman logo" className={styles.logo} />
        <h1 className="hero__title">{siteConfig.title}</h1>
        <p className="hero__subtitle">{siteConfig.tagline}</p>
        <p>Watches files and records, or triggers actions, when they change.</p>
        <div className={styles.buttons}>
          <Link
            className={clsx('button button--lg', styles.getStarted)}
            to="/docs/install">
            Get Started
          </Link>
        </div>
      </div>
    </header>
  );
}

export default function Home() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout
      title={`${siteConfig.title} - ${siteConfig.tagline}`}
      description="Watches files and records, or triggers actions, when they change.">
      <HomepageHeader />
      <main>
        <section className={styles.introduction}>
          <p>
            Watchman exists to watch files and record when they change. It can
            also trigger actions (such as rebuilding assets) when matching files
            change.
          </p>
          <h2>Concepts</h2>
          <ul>
            <li>
              Watchman can recursively watch one or more directory trees (we
              call them roots).
            </li>
            <li>
              Watchman does not follow symlinks. It knows they exist, but they
              show up the same as any other file in its reporting.
            </li>
            <li>
              Watchman waits for a root to settle down before it will start to
              trigger notifications or command execution.
            </li>
            <li>
              Watchman is conservative, preferring to err on the side of
              caution; it considers files to be freshly changed when you start
              to watch them or when it is unsure.
            </li>
            <li>
              You can query a root for file changes since you last checked, or
              the current state of the tree
            </li>
            <li>You can subscribe to file changes that occur in a root</li>
          </ul>
          <h2>Quickstart</h2>
          <p>
            These two lines establish a watch on a source directory and then set
            up a trigger named buildme that will run a tool named{' '}
            <code>minify-css</code>
            whenever a CSS file is changed. The tool will be passed a list of
            the changed filenames.
          </p>
          <CodeBlock language="bash">
            {`$ watchman watch ~/src
# the single quotes around '*.css' are important!
$ watchman -- trigger ~/src buildme '*.css' -- minify-css`}
          </CodeBlock>
          <p>
            The output for buildme will land in the Watchman log file unless you
            send it somewhere else.
          </p>
        </section>
      </main>
    </Layout>
  );
}
