#!/bin/sh

tar -xvf 2.0.tar.gz
cd opencv-benchmarks-2.0/
make -j $NUM_CPU_JOBS
echo $? > ~/install-exit-status
cd ~

echo "#!/bin/sh
cd opencv-benchmarks-2.0/
./main.py > \$LOG_FILE
echo \$? > ~/test-exit-status" > opencv-bench 
chmod +x opencv-bench
