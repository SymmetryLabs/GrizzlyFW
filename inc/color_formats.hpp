#ifndef HEADER_COLOR_FORMATS
#define HEADER_COLOR_FORMATS

struct ARGBFormat
{
  uint8_t A;
  uint8_t R;
  uint8_t G;
  uint8_t B;
};

struct RGBFormat
{
  uint8_t R;
  uint8_t G;
  uint8_t B;
};

struct XYZFormat
{
  float X;
  float Y;
  float Z;
};

struct SlowXYZFormat : XYZFormat {};

struct DMAFormat
{
  uint32_t R[4];
  uint32_t G[4];
  uint32_t B[4];
};

struct SlowDMAFormat {
  uint16_t R[8];
  uint16_t G[8];
  uint16_t B[8];
};

// const std::string ts_ = std::string("");
const std::string ts_argb = std::string("argb");
const std::string ts_rgb = std::string("rgb");
const std::string ts_xyz = std::string("xyz");
const std::string ts_s_xyz = std::string("s_xyz");
const std::string ts_dma = std::string("dma");
const std::string ts_s_dma = std::string("s_dma");

def_bufferType(ARGBFormat)()  { return ts_argb; }
def_bufferType(RGBFormat)() { return ts_rgb; }
def_bufferType(XYZFormat)() { return ts_xyz; }
def_bufferType(SlowXYZFormat)() { return ts_s_xyz; }
def_bufferType(DMAFormat)() { return ts_dma; }
def_bufferType(SlowDMAFormat)() { return ts_s_dma; }

#endif /* HEADER_COLOR_FORMATS */
