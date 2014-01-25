#ifndef HEADER_COLOR_COVERTERS
#define HEADER_COLOR_COVERTERS

const std::string ts_argb2rgb = std::string("argb2rgb");
const std::string ts_rgb2argb = std::string("rgb2argb");
const std::string ts_argb2dma = std::string("argb2dma");
const std::string ts_rgb2dma = std::string("rgb2dma");
const std::string ts_rgb2s_xyz = std::string("rgb2s_xyz");
const std::string ts_s_xyz2rgb = std::string("s_xyz2rgb");

def_converterType(ARGBFormat, RGBFormat)()  { return ts_argb2rgb; }
def_converterType(RGBFormat, ARGBFormat)() { return ts_rgb2argb; }
def_converterType(ARGBFormat, DMAFormat)() { return ts_argb2dma; }
def_converterType(RGBFormat, DMAFormat)() { return ts_rgb2dma; }
def_converterType(RGBFormat, SlowXYZFormat)() { return ts_rgb2s_xyz; }
def_converterType(SlowXYZFormat, RGBFormat)() { return ts_s_xyz2rgb; }

def_convertElement(ARGBFormat, RGBFormat)(ARGBFormat *src, RGBFormat *dest) 
{
  dest->R = src->R;
  dest->G = src->G;
  dest->B = src->B;
}

def_convertElement(RGBFormat, ARGBFormat)(RGBFormat *src, ARGBFormat *dest)
{
  dest->R = src->R;
  dest->G = src->G;
  dest->B = src->B;
}

def_convertElement(RGBFormat, SlowDMAFormat)(RGBFormat *src, SlowDMAFormat *dest)
{
  for (int i = 0; i < 8; i++)
  {
    dest->R[7 - i] = ((src->R >> i) & 0x01);
    dest->G[7 - i] = ((src->G >> i) & 0x01);
    dest->B[7 - i] = ((src->B >> i) & 0x01);
  }
}

def_convertElements(RGBFormat, SlowDMAFormat)(uint32_t num_elements, uint32_t channel_num)
{
  typename ElementBuffer<RGBFormat>::iterator ite = lastElement(num_elements);

  while (source_itr != ite)
  {
    for (int i = 0; i < 8; i++) {
      destination_itr->R[i] = ((source_itr->R >> i) & 0x01) << channel_num;
      destination_itr->G[i] = ((source_itr->G >> i) & 0x01) << channel_num;
      destination_itr->B[i] = ((source_itr->B >> i) & 0x01) << channel_num;
    }
    source_itr++;
    destination_itr++;
  }
}
            
def_convertElement(ARGBFormat, SlowDMAFormat)(ARGBFormat *src, SlowDMAFormat *dest)
{
  for (int i = 0; i < 8; i++)
  {
    dest->R[7 - i] = ((src->R >> i) & 0x01);
    dest->G[7 - i] = ((src->G >> i) & 0x01);
    dest->B[7 - i] = ((src->B >> i) & 0x01);
  }
}

def_convertElements(ARGBFormat, SlowDMAFormat)(uint32_t num_elements, uint32_t channel_num)
{
  typename ElementBuffer<ARGBFormat>::iterator ite = lastElement(num_elements);

  while (source_itr != ite)
  {
    for (int i = 0; i < 8; i++) {
      destination_itr->R[i] = ((source_itr->R >> i) & 0x01) << channel_num;
      destination_itr->G[i] = ((source_itr->G >> i) & 0x01) << channel_num;
      destination_itr->B[i] = ((source_itr->B >> i) & 0x01) << channel_num;
    }
    source_itr++;
    destination_itr++;
  }
}

def_convertElement(RGBFormat, SlowXYZFormat)(RGBFormat *src, SlowXYZFormat *dest)
{
  float r, g, b;

  r = src->R / 255.f;
  g = src->G / 255.f;
  b = src->B / 255.f;

  if (r <= 0.04045f)
    r = r / 12.92f;
  else
    r = (float) std::pow((r + 0.055f)/1.055f, 2.4f);
  r *= 100.0f;

  if (g <= 0.04045f)
    g = g / 12.92f;
  else
    g = (float) std::pow((g + 0.055f)/1.055f, 2.4f);
  g *= 100.0f;

  if (b <= 0.04045f)
    b = b / 12.92f;
  else
    b = (float) std::pow((b + 0.055f)/1.055f, 2.4f);
  b *= 100.0f;

  // dest->X =  0.436052025f*r     + 0.385081593f*g + 0.143087414f *b;
  // dest->Y =  0.222491598f*r     + 0.71688606f *g + 0.060621486f *b;
  // dest->Z =  0.013929122f*r     + 0.097097002f*g + 0.71418547f  *b;
  
  // dest->X = 0.412453f * r + 0.357580f * g + 0.180423f * b;
  // dest->Y = 0.212671f * r + 0.715160f * g + 0.072169f * b;
  // dest->Z = 0.019334f * r + 0.119193f * g + 0.950227f * b;

  dest->X =  0.4124f * r + 0.3576f * g + 0.1805f * b;
  dest->Y =  0.2126f * r + 0.7152f * g + 0.0722f * b;
  dest->Z =  0.0193f * r + 0.1192f * g + 0.9505f * b;
}

def_convertElement(SlowXYZFormat, RGBFormat)(SlowXYZFormat *src, RGBFormat *dest)
{
  float x, y, z;

  x = src->X / 100.0f;
  y = src->Y / 100.0f;
  z = src->Z / 100.0f;

  float r = x *  3.2406f + y * -1.5372f + z * -0.4986f;
  float g = x * -0.9689f + y *  1.8758f + z *  0.0415f;
  float b = x *  0.0557f + y * -0.2040f + z *  1.0570f;

  r = (r > 0.0031308f) ? (1.055f * std::pow(r, 1.f / 2.4f) - 0.055f) : (12.92f * r);
  g = (g > 0.0031308f) ? (1.055f * std::pow(g, 1.f / 2.4f) - 0.055f) : (12.92f * g);
  b = (b > 0.0031308f) ? (1.055f * std::pow(b, 1.f / 2.4f) - 0.055f) : (12.92f * b);

  r = r * 255.0f;
  g = g * 255.0f;
  b = b * 255.0f;

  dest->R = (r > 255) ? 255 : ((r < 0) ? 0 : (uint8_t) r);
  dest->G = (g > 255) ? 255 : ((g < 0) ? 0 : (uint8_t) g);
  dest->B = (b > 255) ? 255 : ((b < 0) ? 0 : (uint8_t) b); 
}

#endif /* HEADER_COLOR_COVERTERS */
