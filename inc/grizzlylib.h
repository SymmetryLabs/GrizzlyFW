#ifndef HEADER_GRIZZLYLIB
#define HEADER_GRIZZLYLIB

#include <queue>
#include <map>

#include <memory>
#include <cmath>
#include <vector>
#include <unordered_map>


namespace GrizzyLib {
  
  template< template <typename> class sObject, class sSubObject >
  using SharedBufferPtr = std::shared_ptr< sObject<sSubObject> >;

  template< template <typename, typename> class sObject, class sSubObject1, class sSubObject2>
  using SharedConverterPtr = std::shared_ptr< sObject<sSubObject1, sSubObject2> >;

  template< template <typename, typename, typename> class sObject, class sSubObject1, class sSubObject2, class sSubObject3>
  using SharedInterpolatorPtr = std::shared_ptr< sObject<sSubObject1, sSubObject2, sSubObject3> >;

  typedef uint32_t ObjectId;
  typedef const std::string& ObjectType;
  typedef uint32_t ChannelId;

  template< class sElementFormat >
  class ElementBuffer
  {
  public:
    typedef sElementFormat* iterator;
    typedef const sElementFormat* const_iterator;
    typedef SharedBufferPtr<ElementBuffer, sElementFormat> BufferPtr;

  private:
    iterator first;
    iterator last;

  public:
    static const std::string& bufferType();

    iterator begin() { return first; }
    iterator end() { return last; }

    ElementBuffer(iterator _first, iterator _last) : first(_first), last(_last)
    {

    };

    ~ElementBuffer()
    {

    };  

    template< class cSrcBuffer >
    static BufferPtr createBufferFromSource(cSrcBuffer *buf_ptr, uint32_t offset, uint32_t length);

    static BufferPtr createBufferFromIterators(iterator src_first, iterator src_last) {
      BufferPtr new_buf( new ElementBuffer<sElementFormat>(src_first, src_last) );

      return new_buf;
    };
  };

  template<class sFormat>
  using BufferPtr = typename ElementBuffer<sFormat>::BufferPtr;

  template<class sFormat>
  using BufferType = typename ElementBuffer<sFormat>::bufferType;

  template< class sSrcFormat, class sDestFormat >
  class ElementConverter
  {
  private:
    typename ElementBuffer<sSrcFormat>::BufferPtr source;
    typename ElementBuffer<sSrcFormat>::iterator source_itr;

    typename ElementBuffer<sDestFormat>::BufferPtr destination;
    typename ElementBuffer<sDestFormat>::iterator destination_itr;
    
  public:
    typedef SharedConverterPtr<ElementConverter, sSrcFormat, sDestFormat>  ConverterPtr;

    static inline bool compareElement(sSrcFormat *src, sDestFormat *dest);
    static inline void convertElement(sSrcFormat *src, sDestFormat *dest);
    static inline const std::string& converterType();

    typename ElementBuffer<sSrcFormat>::iterator lastElement(uint32_t num_elements)
    {
      typename ElementBuffer<sSrcFormat>::iterator ite = source_itr + num_elements;

      if (ite > source->end())
      {
        ite = source->end();
      }

      if ((destination_itr + num_elements) > destination->end())
      {
        ite = source_itr;
        typename ElementBuffer<sDestFormat>::iterator itd = destination_itr;
        while (itd != destination->end())
        {
          ite++;
          itd++;
        }
      }

      return ite;
    }

    void convertElements(uint32_t num_elements)
    {
      typename ElementBuffer<sSrcFormat>::iterator ite = lastElement(num_elements);

      while (source_itr != ite)
      {
        ElementConverter<sSrcFormat, sDestFormat>::convertElement(source_itr++, destination_itr++);
      }
    }

    /* DMA converter needs channel number. uint32_t as placeholder for ptr/other data */
    void convertElements(uint32_t num_elements, uint32_t channel_num)
    {
      convertElements(num_elements);
    }

    /* ===== Static Members ===== */
    static ConverterPtr createConverter(typename ElementBuffer<sSrcFormat>::BufferPtr _source, \
                                        typename ElementBuffer<sDestFormat>::BufferPtr _destination)
    {
      ConverterPtr new_conv( new ElementConverter<sSrcFormat, sDestFormat> );
      new_conv->source = _source;
      new_conv->source_itr = _source->begin();
      new_conv->destination = _destination;
      new_conv->destination_itr = _destination->begin();
      return new_conv;
    }

    ElementConverter()
    {
      // cout << "ElementConverter constructed" << endl;
    }

    ~ElementConverter()
    {
      // cout << "ElementConverter destructed" << endl;
    }
  };

  template<class sSrcFormat, class sDestFormat>
  using ConverterPtr = typename ElementConverter<sSrcFormat,sDestFormat>::ConverterPtr;

  template<class sSrc1Format, class sSrc2Format, class sDestFormat>
  class ElementInterpolator
  {
  private:
    typename ElementBuffer<sSrc1Format>::BufferPtr source1;
    typename ElementBuffer<sSrc1Format>::iterator source1_itr;

    typename ElementBuffer<sSrc2Format>::BufferPtr source2;
    typename ElementBuffer<sSrc2Format>::iterator source2_itr;

    typename ElementBuffer<sDestFormat>::BufferPtr destination;
    typename ElementBuffer<sDestFormat>::iterator destination_itr;

    typename ElementConverter<sSrc2Format, sSrc1Format>::ConverterPtr source_converter;
    typename ElementConverter<sSrc1Format, sDestFormat>::ConverterPtr destination_converter;

  public:
    typedef SharedInterpolatorPtr<ElementInterpolator, sSrc1Format, sSrc2Format, sDestFormat>  InterpolatorPtr;

    /* ===== Function Definitions ===== */
    static inline void interpolateElement(sSrc1Format *src1, sSrc2Format *src2, sDestFormat *dest, float ratio);
    static inline const std::string& interpolatorType();

    /* ===== Utility Functions ===== */
    inline bool checkBuffers() { return (source1 != 0) && (source2 != 0) && (destination !=0); }

    typename ElementBuffer<sSrc1Format>::iterator lastElement(uint32_t num_elements)
    {
      typename ElementBuffer<sSrc1Format>::iterator ite = source1_itr + num_elements;

      if (ite > source1->end())
      {
        ite = source1->end();
      }

      if ((destination_itr + num_elements) > destination->end() || (source2_itr + num_elements) > source2->end())
      {
        typename ElementBuffer<sSrc2Format>::iterator ite2 = source2_itr;
        typename ElementBuffer<sDestFormat>::iterator itd = destination_itr;
        ite = source1_itr;

        while (itd != destination->end() && ite2 != source2->end() && ite != source1->end())
        {
          ite++;
          ite2++;
          itd++;
        }
      }

      return ite;
    }

    /* ===== Worker Functions ===== */
    void setSources(BufferPtr<sSrc1Format> src1, BufferPtr<sSrc2Format> src2)
    {
      // if (ElementBufferer<sSrc1Format>::bufferType != )
      source1 = src1;
      source2 = src2;
      source1_itr = src1->begin();
      source2_itr = src2->begin();
    }

    void setDestination(BufferPtr<sDestFormat> dest)
    {
      destination = dest;
      destination_itr = dest->begin();
    }

    void interpolateElements(uint32_t num_elements)
    {
      auto ite = lastElement(num_elements);
      
      float ratio = .5f;
      while (source1_itr != ite)
      {
        ElementInterpolator<sSrc1Format, sSrc2Format, sDestFormat>::interpolateElement(source1_itr++, source2_itr++, destination_itr++, ratio);
      }
    }

    ElementInterpolator()
    {
      // cout << "ElementInterpolator constructed" << endl;
    };

    ~ElementInterpolator()
    {
      // cout << "ElementInterpolator destructed" << endl;
    };
  };
	
	
	
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
	
	#define elementBufferT(_format) ElementBuffer<_format>
	#define elementBufferF(_function, _format) elementBufferT(_format)::_function
	#define def_bufferType(_format) template<> const std::string&  elementBufferF(bufferType, _format)

	/* ElementConverter template function implementation macros */
	#define elementConverterT(_src_format, _dest_format) ElementConverter<_src_format, _dest_format>
	#define elementConverterF(_function, _src_format, _dest_format) elementConverterT(_src_format, _dest_format)::_function
	#define def_convertElement(_src_format, _dest_format) template<> inline void elementConverterF(convertElement, _src_format, _dest_format)
	#define def_convertElements(_src_format, _dest_format) template<> void elementConverterF(convertElements, _src_format, _dest_format)
	#define def_converterType(_src_format, _dest_format) template<> inline const std::string& elementConverterF(converterType, _src_format, _dest_format)

	/* ElementInterpolator template function implementation macros */
	#define elementInterpolatorT(_src1_format, _src2_format, _dest_format) ElementInterpolator<_src1_format, _src2_format, _dest_format>
	#define elementInterpolatorF(_function, _src1_format, _src2_format, _dest_format) elementInterpolatorT(_src1_format, _src2_format, _dest_format)::_function
	#define def_interpolateElement(_src1_format, _src2_format, _dest_format) template<> inline void elementInterpolatorF(interpolateElement, _src1_format, _src2_format, _dest_format)
	#define def_interpolateElements(_src1_format, _src2_format, _dest_format) template<> void elementInterpolatorF(interpolateElements, _src1_format, _src2_format, _dest_format)
	#define def_interpolatorType(_src1_format, _src2_format, _dest_format) template<> inline const std::string& elementInterpolatorF(interpolatorType, _src1_format, _src2_format, _dest_format)

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


	const std::string ts_rgbrgb2rgb = std::string("rgbrgb2rgb");

	def_interpolatorType(RGBFormat, RGBFormat, RGBFormat)() {return ts_rgbrgb2rgb; }

	def_interpolateElement(RGBFormat, RGBFormat, RGBFormat)(RGBFormat *src1, RGBFormat *src2, RGBFormat *dest, float ratio)
	{
		dest->R = (uint8_t) (src1->R + ratio * (src2->R - src1->R));
		dest->G = (uint8_t) (src1->G + ratio * (src2->G - src1->G));
		dest->B = (uint8_t) (src1->B + ratio * (src2->B - src1->B));
	}

  class FrameGroup {
		private:
			/* buffers[ChannelId][ObjectType] = ObjectId */
			std::unordered_map<ChannelId , std::unordered_map<std::string, ObjectId> > buffers;

		public:
			friend class ObjectManager;

			ObjectId getChannelObjectId(ChannelId ch_id, ObjectType obj_type)
			{
				auto iter = buffers.find(ch_id);
				if (iter == buffers.end())
					return 0;
				auto iter2 = iter->second.find(obj_type);
				if (iter2 == iter->second.end())
					return 0;
				return iter2->second;
			};

			template<class sFormat>
			ObjectId getChannelObjectId(ChannelId ch_id)
			{
				return getChannelObjectId(ch_id, BufferType<sFormat>());
			}

			inline bool hasChannelObject(ChannelId ch_id)
			{
				return !( buffers.find(ch_id) == buffers.end() );
			}

			inline bool hasChannelObject(ChannelId ch_id, ObjectType obj_type)
			{
				if (!hasChannelObject(ch_id))
					return false;
				return !( buffers[ch_id].find(obj_type) == buffers[ch_id].end() );
			}

			void addChannelObject(ChannelId ch_id, ObjectType obj_type, ObjectId obj_id)
			{
				if (hasChannelObject(ch_id, obj_type))
					return;
				buffers[ch_id][obj_type] = obj_id;
			}

			template<class sFormat>
			void addChannelObject(ChannelId ch_id, ObjectId obj_id)
			{
				addChannelObject(ch_id, BufferType<sFormat>(), obj_id);
			}
		};

	typedef std::shared_ptr<FrameGroup> FramePtr;

	class ObjectManager
	{
	// private:  // TODO: CHANGE BACK TO PRIVATE!
	public:
		struct ObjectPtr {
				uint32_t id;
				ObjectType type;
				std::shared_ptr<void> mem_ptr;
				std::shared_ptr<void> obj_ptr;
		};
		std::unordered_map<ObjectId, ObjectPtr> objects;
		uint32_t num_objects;
		uint32_t num_objects_deleted;

		std::unordered_map<ChannelId, ObjectId> pending_buffers;

		uint32_t allocateObject(ObjectType obj_type, std::shared_ptr<void> block_ptr, std::shared_ptr<void> obj_ptr)
		{
			num_objects++;
			objects.emplace(num_objects, ObjectPtr{ num_objects, \
																							obj_type, \
																							block_ptr, \
																							obj_ptr });
			// cout << " === Object #" << num_objects << " allocated === " << endl;
			return num_objects;
		}

		std::shared_ptr<void> getObject(ObjectId obj_id, ObjectType obj_type)
		{
			auto iter = objects.find(obj_id);
			if (iter == objects.end())
				return 0;
			if (obj_type != iter->second.type)
				return 0;
			return iter->second.obj_ptr;
		}

		void releaseObject(ObjectId obj_id)
		{
			auto obj = objects.find(obj_id);
			if (obj == objects.end())
				return;
			if (obj->second.mem_ptr != 0)
				obj->second.mem_ptr.reset();
			if (obj->second.obj_ptr != 0)
				obj->second.obj_ptr.reset();
			objects.erase(obj);
			// cout << " === Object #" << num_objects_deleted++ << " released === " << endl;
		}

	public:
		ObjectId allocateFrame()
		{
			auto obj_ptr = std::make_shared<FrameGroup>();
			return allocateObject("frame", 0, obj_ptr);
		}

		FramePtr getFrame(ObjectId obj_id)
		{
			auto obj_ptr = getObject(obj_id, "frame");
			return std::static_pointer_cast< FrameGroup >(obj_ptr);
		}

		void releaseFrame(ObjectId obj_id)
		{
			auto obj_ptr = std::static_pointer_cast<FrameGroup>(getObject(obj_id, "frame"));
			if (obj_ptr == 0)
				return;
			for ( auto it = obj_ptr->buffers.begin(); it != obj_ptr->buffers.end(); ++it )
			{
				for ( auto ite = it->second.begin(); ite != it->second.end(); ++ite )
				{
					releaseObject(ite->second);
				}
			}
			releaseObject(obj_id);
		}

		template<class sFormat>
		ObjectId allocateBuffer(uint32_t size)
		{
			auto block_ptr = std::make_shared< std::vector<sFormat> >(size);
			auto obj_ptr = std::make_shared< ElementBuffer<sFormat> >(&*block_ptr->begin(), &*block_ptr->end());
			return allocateObject(ElementBuffer<sFormat>::bufferType(), block_ptr, obj_ptr);
		}
		
		void releaseBuffer(ObjectId obj_id)
		{
			releaseObject(obj_id);
		}

		template<class sFormat>
		BufferPtr<sFormat> getBuffer(ObjectId obj_id)
		{
			auto obj_ptr = getObject(obj_id, ElementBuffer<sFormat>::bufferType());
			return std::static_pointer_cast< ElementBuffer<sFormat> >(obj_ptr);
		}

		void pushPendingBuffer(ChannelId ch_id, ObjectId obj_id)
		{
			pending_buffers[ch_id] = obj_id;
		}

		template<class sSrcFormat, class sDestFormat>
		ObjectId createConverter()
		{
			auto obj_ptr = std::make_shared< ElementConverter<sSrcFormat, sDestFormat> >();
			return allocateObject(ElementConverter<sSrcFormat, sDestFormat>::converterType(), \
														0, obj_ptr);
		}

		template<class sSrcFormat, class sDestFormat>
		ConverterPtr<sSrcFormat, sDestFormat> getConverter(ObjectId obj_id)
		{
			auto obj_ptr = getObject(obj_id, ElementConverter<sSrcFormat, sDestFormat>::converterType());
			return std::static_pointer_cast< ElementConverter<sSrcFormat, sDestFormat> >(obj_ptr);
		}
	};
	
	typename<class sFormat>
	struct FormatType {
		//static ElementBuffer<sFormat> 
	};
}

#endif /* HEADER_GRIZZLYLIB */
