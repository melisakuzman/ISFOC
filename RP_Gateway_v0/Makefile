# === Configurable Variables ===

# Source file
SRC = lora_rx.cpp simple_paquete.cpp

# Output executable
OUT = lora_rx.exe

# Include and Library paths (change these as needed)
RADIO_LIB_INCLUDE = /usr/local/include/RadioLib/
RADIO_LIB_STATIC = /usr/local/lib/libRadioLib.a
LGPIO_LIB_PATH = /usr/local/lib/

# Compiler and flags
CXX = g++
CXXFLAGS = -I$(RADIO_LIB_INCLUDE) -I.
LDFLAGS = $(RADIO_LIB_STATIC) -L$(LGPIO_LIB_PATH) -llgpio

# === Rules ===

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(LDFLAGS) -o $(OUT)

clean:
	rm -f $(OUT)

.PHONY: all clean
