MPICXX = mpicxx

CPP_FLAGS = -std=c++11 -Wall

SRC_DIR = src
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
GENERATOR = $(BUILD_DIR)/generator
EXECUTABLE = $(BUILD_DIR)/birch

.PHONY: all debug generator clean 

all: $(EXECUTABLE)

debug: CPP_FLAGS += -DDEBUG
debug: $(EXECUTABLE)

generator: $(GENERATOR)

$(EXECUTABLE): $(SRCS) $(BUILD_DIR)
	$(MPICXX) $(CPP_FLAGS) $(filter-out $(wildcard $(SRC_DIR)/*generator.cpp), $(SRCS)) -o $(EXECUTABLE)

$(GENERATOR): $(SRCS) $(BUILD_DIR)
	$(MPICXX) $(CPP_FLAGS) $(filter-out $(SRC_DIR)/birch.cpp, $(SRCS)) -o $(GENERATOR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -r $(BUILD_DIR)
