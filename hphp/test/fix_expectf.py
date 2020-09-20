#!/usr/bin/env python3
import glob
import re
import os
import sys
f=glob.iglob(sys.argv[1] + '/**/*.php.diff', recursive=True)
vec = []
for tmp in f:
    name = tmp[:len(tmp) - 5]
    if os.path.isfile(name + '.expectf'):
        vec.append(name)
try:
    for ll in vec:
        l2 = ll.strip('\n')
        with open(l2 + '.diff') as f:
            add = 0
            vecAdd = []
            sub = 0
            vecSub = []
            stringVec = []
            lineNumber = []
            oldNumber = []
            for line in f:
                if re.match('(\d+)\+.*', line):
                    add += 1
                    vecAdd.append(line)
                if re.match('(\d+)\-.*', line):
                    sub += 1
                    vecSub.append(line)
            if add == sub:
                flag = True
                for i in range(add):
                    if flag is False:
                        break
                    p1 = '(\d+)\+\s(.*line\s)(\d+)'
                    p2 = '(\d+)\-\s(.*line\s)(\d+)'
                    s1 = vecAdd[i]
                    s2 = vecSub[i].replace('[^\r\n]', '[^\\r\\n]')
                    res1 = re.match(p1, s1)
                    res2 = re.match(p2, s2)
                    if res1 is None or res2 is None:
                        flag = False
                        break
                    elif re.match(res2.group(2), res1.group(2)) is None:
                        flag = False
                        break
                    else:
                        stringVec.append(res2.group(2).replace('[^\\r\\n]', '[^\r\n]'))
                        lineNumber.append(res1.group(3))
                        oldNumber.append(res2.group(3))
                if flag is True:
                    change = ''
                    used = {}
                    for i in range(add):
                        used[i] = False
                    with open(l2 + '.expectf') as text:
                        for l in text:
                            tmp = l
                            for i in range(add):
                                if re.match(stringVec[i], l) and used[i] is False:
                                    tmp = l.replace('line ' + oldNumber[i], 'line ' + lineNumber[i])
                                    used[i] = True
                                    break
                            change += tmp

                    f = open(l2 + '.expectf', 'w')
                    f.write(change)
                    f.close()
except Exception:
        pass
