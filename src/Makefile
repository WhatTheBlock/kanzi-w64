CXX=g++
CFLAGS=-c -std=c++17 -Wall -Wextra -O3 -fomit-frame-pointer -fPIC -DNDEBUG -pedantic -march=native
LDFLAGS=-pthread
LIB_SOURCES=Global.cpp \
	Event.cpp \
	transform/BWT.cpp \
	transform/BWTS.cpp \
	transform/DivSufSort.cpp \
	transform/SBRT.cpp \
	bitstream/DefaultInputBitStream.cpp \
	bitstream/DefaultOutputBitStream.cpp \
	io/CompressedInputStream.cpp \
	io/CompressedOutputStream.cpp \
	entropy/ANSRangeDecoder.cpp \
	entropy/ANSRangeEncoder.cpp \
	entropy/BinaryEntropyDecoder.cpp \
	entropy/BinaryEntropyEncoder.cpp \
	entropy/EntropyUtils.cpp \
	entropy/ExpGolombDecoder.cpp \
	entropy/ExpGolombEncoder.cpp \
	entropy/HuffmanCommon.cpp \
	entropy/HuffmanDecoder.cpp \
	entropy/HuffmanEncoder.cpp \
	entropy/RangeDecoder.cpp \
	entropy/RangeEncoder.cpp \
	entropy/RiceGolombEncoder.cpp \
	entropy/RiceGolombDecoder.cpp \
	entropy/CMPredictor.cpp \
	entropy/FPAQPredictor.cpp \
	entropy/TPAQPredictor.cpp \
	function/BWTBlockCodec.cpp \
	function/LZCodec.cpp \
	function/ROLZCodec.cpp \
	function/RLT.cpp \
	function/SRT.cpp \
	function/TextCodec.cpp \
	function/X86Codec.cpp \
	function/ZRLT.cpp 
LIB_OBJECTS=$(LIB_SOURCES:.cpp=.o)

APP_SOURCES=app/Kanzi.cpp \
	app/InfoPrinter.cpp \
	app/BlockCompressor.cpp \
	app/BlockDecompressor.cpp
APP_OBJECTS=$(APP_SOURCES:.cpp=.o)

SOURCES=$(LIB_SOURCES) $(APP_SOURCES)
OBJECTS=$(SOURCES:.cpp=.o)
RPTS=$(SOURCES:.cpp=.optrpt)


APP=kanzi
	
STATIC_LIB_SUFFIX := .lib
SHARED_LIB_SUFFIX := .dll
PROG_SUFFIX       := .exe

STATIC_LIB := lib$(APP)$(STATIC_LIB_SUFFIX)
SHARED_LIB := lib$(APP)$(SHARED_LIB_SUFFIX)

all: $(STATIC_LIB) $(SHARED_LIB) $(APP)

# Create static library
$(STATIC_LIB):$(LIB_OBJECTS)
	$(AR) cr ../lib/$@ $+

# Create shared library
$(SHARED_LIB):$(LIB_OBJECTS)
	$(CXX) -o ../lib/$@ $(LDFLAGS) -shared $+

kanzi: $(OBJECTS) app/Kanzi.o
	$(CXX) $^ -o ../bin/$@ $(LDFLAGS)

.cpp.o:
	$(CXX) $(CFLAGS) $< -o $@
