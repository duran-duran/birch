MPICXX = mpicxx

CPP_FLAGS = -std=c++11 -Wall

SRC_DIR = src
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
GENERATOR = $(BUILD_DIR)/generator
CLUSTERIZER = $(BUILD_DIR)/birch

PROC_NUM = 4

.PHONY: all clean run

all: $(GENERATOR) $(CLUSTERIZER)

$(GENERATOR): $(SRCS) $(BUILD_DIR)
	$(MPICXX) $(CPP_FLAGS) $(filter-out $(SRC_DIR)/birch.cpp, $(SRCS)) -o $(GENERATOR)

$(CLUSTERIZER): $(SRCS) $(BUILD_DIR)
	$(MPICXX) $(CPP_FLAGS) $(filter-out $(SRC_DIR)/generator.cpp, $(SRCS)) -o $(CLUSTERIZER)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -r $(BUILD_DIR)

run:
	mpiexec -np $(PROC_NUM) ./$(CLUSTERIZER)
