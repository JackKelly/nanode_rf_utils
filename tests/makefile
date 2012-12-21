# DIRECTORIES
nanode_rf_utils_dir = /home/jack/workspace/avr/nanode_rf_utils

# COMPILATION AND LINKING VARIABLES
CXX = g++
CXXFLAGS := -Wall -MMD -g -O0 -D TESTING -I$(nanode_rf_utils_dir)
LINK = ${CXX} $^ -lboost_unit_test_framework -o $@ && ./$@

# RULES
all: Packet_test

Packet_test: Packet_test.o FakeArduino.o
	$(LINK)
	
# Include dependency files:
-include *.d
-include ../*.d

# Clean
clean:
	rm -rf *.o *_test *.d ../*.o ../*.d