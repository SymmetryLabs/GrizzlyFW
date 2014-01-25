#ifndef HEADER_COLOR_INTERPOLATORS 
#define HEADER_COLOR_INTERPOLATORS 

const std::string ts_rgbrgb2rgb = std::string("rgbrgb2rgb");

def_interpolatorType(RGBFormat, RGBFormat, RGBFormat)() {return ts_rgbrgb2rgb; }

def_interpolateElement(RGBFormat, RGBFormat, RGBFormat)(RGBFormat *src1, RGBFormat *src2, RGBFormat *dest, float ratio)
{
	dest->R = (uint8_t) (src1->R + ratio * (src2->R - src1->R));
	dest->G = (uint8_t) (src1->G + ratio * (src2->G - src1->G));
	dest->B = (uint8_t) (src1->B + ratio * (src2->B - src1->B));
}

// template<class sFormat>
// void elementInterpolatorF(interpolateElements, sFormat, sFormat, sFormat)(uint32_t num_elements)
// {
// 	elementInterpolatorT(sFormat, sFormat, sFormat)::interpolateElement(src1, src2, dest);
// }
#endif /* HEADER_COLOR_INTERPOLATORS */