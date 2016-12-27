for i in tc_*; do
make clean
make bin/${i%.c}
./bin/${i%.c}
done





