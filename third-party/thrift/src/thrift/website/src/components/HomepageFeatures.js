/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 */

import React from 'react';
import clsx from 'clsx';
import styles from './HomepageFeatures.module.css';

const FeatureList = [
  {
    title: 'Extensive',
    imageUrl: 'img/toolgear.png',
    imageWidth: '250px',
    description: (
      <>
        Contains over 750 C++ libraries, from data structures to logging to concurrency.
      </>
    ),
  },
  {
    title: 'Performance',
    imageUrl: 'img/server.jpeg',
    imageWidth: '300px',
    description: (
      <>
        Designed for and executed on Meta&#39;s enormous fleet, folly focuses on
        server performance.
      </>
    ),
  },
  {
    title: 'Open Source',
    imageUrl: 'img/meta_oss.svg',
    imageWidth: '400px',
    description: (
      <>
        Check out folly on <a href="https://github.com/facebook/folly">GitHub</a>.
      </>
    ),
  },
];

function Feature({imageUrl, imageWidth, title, description}) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
        <img className={styles.featureImage} src={imageUrl} alt={title} width={imageWidth} />
      </div>
      <div className="text--center padding-horiz--md">
        <h3>{title}</h3>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures() {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
