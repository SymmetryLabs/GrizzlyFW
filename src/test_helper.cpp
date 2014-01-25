// #include <iostream>
// #include <string>
// #include <cassert>
// #include "grizzlylib.h"

// using namespace GrizzyLib;
// using std::cout;
// using std::endl;

typedef ElementBuffer<ARGBFormat> argb_t;
typedef ElementBuffer<RGBFormat> rgb_t;
typedef ElementBuffer<SlowXYZFormat> xyz_t;
typedef ElementBuffer<SlowDMAFormat> dma_t;

typedef ElementConverter<ARGBFormat, RGBFormat> argb2rgb_t;
typedef ElementConverter<RGBFormat, ARGBFormat> rgb2argb_t;
typedef ElementConverter<RGBFormat, SlowXYZFormat> rgb2xyz_t;
typedef ElementConverter<SlowXYZFormat, RGBFormat> xyz2rgb_t;
typedef ElementConverter<RGBFormat, SlowDMAFormat> rgb2dma_t;

BufferPtr<ARGBFormat> init_argb_buffer(ObjectManager *objman, uint32_t length)
{
  auto new_buf = objman->getBuffer<ARGBFormat>(objman->allocateBuffer<ARGBFormat>(length));
  int i = 0;
  argb_t::iterator ite = new_buf->end();
  for ( argb_t::iterator it = new_buf->begin(); \
        it != ite; it++ )
  {
    it->A = i % 256;
    it->R = i % 256;
    it->G = (255 - i) % 256 ;
    it->B = 0;
    i++;
  }
  return new_buf;
}

BufferPtr<RGBFormat> init_rgb_buffer(ObjectManager *objman, uint32_t length)
{
  auto new_buf = objman->getBuffer<RGBFormat>(objman->allocateBuffer<RGBFormat>(length));
  int i = 0;
  rgb_t::iterator ite = new_buf->end();
  for ( rgb_t::iterator it = new_buf->begin(); \
        it != ite; it++ )
  {
    it->R = i % 256;
    it->G = (255 - i) % 256 ;
    it->B = 0;
    i++;
  }

  return new_buf;
}

void argb_check(ObjectManager *objman, const int num_pixels)
{
  auto argb = init_argb_buffer(objman, num_pixels);
  auto rgb = objman->getBuffer<RGBFormat>(objman->allocateBuffer<RGBFormat>(num_pixels));

  auto test_conv = argb2rgb_t::createConverter(argb, rgb);
  
  test_conv->convertElements(num_pixels);

//   objman->releaseBuffer(argb);
//   objman->releaseBuffer(rgb);

  printBufferElements<RGBFormat, uint8_t, int>(rgb->begin(), rgb->end());
}

void slow_xyz_check(ObjectManager *objman, const int num_pixels)
{
  auto argb = init_argb_buffer(objman, num_pixels);
  auto rgb = objman->getBuffer<RGBFormat>(objman->allocateBuffer<RGBFormat>(num_pixels));
  auto rgb2 = objman->getBuffer<RGBFormat>(objman->allocateBuffer<RGBFormat>(num_pixels));
  auto test_conv = argb2rgb_t::createConverter(argb, rgb);
  test_conv->convertElements(num_pixels);
  auto xyz = objman->getBuffer<SlowXYZFormat>(objman->allocateBuffer<SlowXYZFormat>(num_pixels));

  auto test_conv2 = rgb2xyz_t::createConverter(rgb, xyz);
  test_conv2->convertElements(num_pixels);
  // printBufferElements<SlowXYZFormat, float, float>(xyz->begin(), xyz->end());

  auto test_conv3 = xyz2rgb_t::createConverter(xyz, rgb2);
  test_conv3->convertElements(num_pixels);
  // printBufferElements<RGBFormat, uint8_t, int>(rgb2->begin(), rgb2->end());

//   objman->releaseBuffer(argb);
//   objman->releaseBuffer(rgb);
//   objman->releaseBuffer(rgb2);
//   objman->releaseBuffer(xyz);
}

void slow_dma_check(ObjectManager *objman, const int num_pixels)
{
  auto rgb = init_rgb_buffer(objman, num_pixels);
  auto dma = objman->getBuffer<SlowDMAFormat>(objman->allocateBuffer<SlowDMAFormat>(num_pixels));
  
  auto test_conv = rgb2dma_t::createConverter(rgb, dma);
  test_conv->convertElements(num_pixels, 15);

  // printBufferElements<SlowDMAFormat, uint32_t, uint32_t>(dma->begin(), dma->end());
//   objman->releaseBuffer(rgb);
//   objman->releaseBuffer(dma);
}

void rgb3_interpolate_check(ObjectManager *objman, const int num_pixels)
{
  auto rgb  = init_rgb_buffer(objman, num_pixels);
  auto rgb2 = objman->getBuffer<RGBFormat>(objman->allocateBuffer<RGBFormat>(num_pixels));
  auto rgb3 = objman->getBuffer<RGBFormat>(objman->allocateBuffer<RGBFormat>(num_pixels));

  auto interp = std::make_shared< ElementInterpolator<RGBFormat, RGBFormat, RGBFormat> >();
  interp->setSources(rgb, rgb2);
  interp->setDestination(rgb3);
  interp->interpolateElements(num_pixels);

  printBufferElements<RGBFormat, uint8_t, int>(rgb->end()-3, rgb->end());
  printBufferElements<RGBFormat, uint8_t, int>(rgb2->end()-3, rgb2->end());
  printBufferElements<RGBFormat, uint8_t, int>(rgb3->end()-3, rgb3->end());

//   objman->releaseBuffer(rgb);
//   objman->releaseBuffer(rgb2);
//   objman->releaseBuffer(rgb3);
}

// int main(int argc, char* argv[])
// {
//   const int num_pixels = 128;

//   cout << " ==============================" << endl;
//   cout << " GrizzlyLIB Testing application" << endl;
//   cout << " ==============================" << endl;

//   ObjectManager *objman = new ObjectManager();

//   argb_check(objman, num_pixels);
//   slow_xyz_check(objman, num_pixels);
//   slow_dma_check(objman, num_pixels);
//   rgb3_interpolate_check(objman, num_pixels);

//   auto rgb = objman->getBuffer<RGBFormat>(objman->allocateBuffer<RGBFormat>(num_pixels));
// //   objman->releaseBuffer(rgb);
//   // argb_check(num_pixels);

//   return 0;
// }
