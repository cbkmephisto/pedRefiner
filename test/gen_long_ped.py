#!/usr/bin/env python3

n=100   # 1000000
for i in range(1, n, 2):
    print(i, i + 1, i + 2, sep=",")
    print(i+1, i+1+n, i+2+n, sep=",")
