/**
 * (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.
 */

/* eslint-disable */

import React from 'react';
import {OssOnly, FbInternalOnly} from 'docusaurus-plugin-internaldocs-fb/internal';
import MDXContent from '@theme/MDXContent';
import Admonition from '@theme/Admonition';

export default function FbCaution({children}) {
  return (
    <FbInternalOnly>
    <Admonition type="caution" title="Caution">
      <MDXContent>
        {children}
      </MDXContent>
    </Admonition>
    </FbInternalOnly>
  );
}
