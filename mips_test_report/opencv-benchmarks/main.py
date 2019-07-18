#!/usr/bin/python

import time
import os
start_time = time.time()
os.system("./bin/discrete_fourier_transform")
os.system("./bin/contours2")
os.system("./bin/edge")
os.system("./bin/smoothing")
os.system("./bin/morphology_2")

print
print("Benchmark took  %s seconds" % (time.time() - start_time))

