#!/usr/bin/python

import os
import sys
import tempfile
import time
import re
import glob

files = glob.glob("../src/user/*.c")
for f in files:
    if (os.path.islink(f)):
	os.remove(f)
