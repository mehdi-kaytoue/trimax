

# Compile.
g++ -I /usr/local/boost_1_54_0/ *.cpp -o trimax

# Exec.
#         datafile  theta min_objects min_attributes max_objects max_attributes
./trimax sample.txt 2 0 0 4 5

