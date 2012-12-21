# DIRECTORIES
nanode_rf_utils_dir = /home/jack/workspace/avr/nanode_rf_utils

# COMPILATION AND LINKING VARIABLES
CXX = g++
CXXFLAGS := -Wall -MMD -g -O0 -D TESTING -I$(nanode_rf_utils_dir)

# TARGETS
EXECS = Packet_test Utils_test

# RULE FOR all
all: $(EXECS)

# DEPENDENCIES FOR LINKING STEP
Packet_test: Packet_test.o FakeArduino.o
Utils_test: Utils_test.o ../utils.o FakeArduino.o

# LINKING STEP:
$(EXECS):
	${CXX} $^ -lboost_unit_test_framework -o $@

# INCLUDE COMPILATION DEPENDENCIES
-include *.d
-include ../*.d

# Clean
clean:
	rm -rf *.o *_test *.d ../*.o ../*.d