#!/usr/bin/python3

import sys

d = {}
with open(sys.argv[1]) as f:
    for l in f:
        if l.startswith("#define"):
            l = l.split()
            d[l[1]] = l[2]
            continue
        for k in d:
            l = l.replace(k, d[k])
        print(l, end="")
