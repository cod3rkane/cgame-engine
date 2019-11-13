#!/usr/bin/python3

import sys

red = float(sys.argv[1]) / 255.0
blue = float(sys.argv[2]) / 255.0
green = float(sys.argv[3]) / 255.0

print('color: red: ', red, ' blue: ', blue, ' green: ', green)
print('%gf, %gf, %gf' % (red, blue, green))
