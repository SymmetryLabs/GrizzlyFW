/* 
    Example of two different ways to process received OSC messages using oscpack.
    Receives the messages from the SimpleSend.cpp example.
*/

#include <iostream>
#include <string>
#include <cassert>
#include "grizzlylib.h"

using namespace GrizzyLib;

#include "osc/OscReceivedElements.h"
#include "osc/OscPacketListener.h"
#include "ip/UdpSocket.h"

#include "test_helper.cpp"

#define PORT 7000


ObjectManager *objman;
std::shared_ptr<FrameGroup> frame;
uint32_t current_frame_num;


void test_osc_message(const osc::ReceivedMessage& m)
{
    osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
    uint32_t frame_num = (arg++)->AsInt32();
    uint32_t packet_num = (arg++)->AsInt32();
    uint32_t num_channels = (arg++)->AsInt32();

    if (frame == 0 || current_frame_num < frame_num)
    {
        current_frame_num = frame_num;
        // objman->allocateFrame();
    }

    const char* ch_type[num_channels];
    uint32_t ch_num[num_channels];
    uint32_t ch_offset[num_channels];
    uint32_t ch_size[num_channels];
    for (int i = 0; i < num_channels; i++)
    {
      ch_type[i] = (arg++)->AsString();
      ch_num[i] = (arg++)->AsInt32();
      ch_offset[i] = (arg++)->AsInt32();
      ch_size[i] = (arg++)->AsInt32();
      std::cout << "Type: " << ch_type[i] << "   Channel Number: " << ch_num[i] << "    Number of LEDs:" << ch_size[i] << std::endl;
    }
    uint32_t total_size = (arg++)->AsInt32();
    const void* data;
    osc::osc_bundle_element_size_t data_size;
    arg->AsBlob(data, data_size);
    for (int i = 0; i < num_channels; i++)
    {
        if (std::strcmp(ch_type[i], "argb") == 0)
        {
            auto buf = objman->allocateBuffer<RGBFormat>(ch_size[i]);
            auto buf_ptr = objman->getBuffer<RGBFormat>(buf);
            auto void_ptr = const_cast<void*>(data);
            auto format_ptr = static_cast<ARGBFormat*>(void_ptr);

            for (auto ite = buf_ptr->begin(); ite != buf_ptr->end(); ite++)
            {
                ElementConverter<ARGBFormat, RGBFormat>::convertElement(format_ptr++, ite);
                // *(ite++) = *(format_ptr++);
            }
            printBufferElements<RGBFormat, uint8_t, int>(buf_ptr->end()-3, buf_ptr->end());

            
        }
    }

    std::cout << "received '/shady/pointbuffer' message with "
                    << num_channels << " channels and a payload size of  " << total_size << " bytes" << "\n";
}

class ExamplePacketListener : public osc::OscPacketListener {
protected:

    virtual void ProcessMessage( const osc::ReceivedMessage& m, 
				const IpEndpointName& remoteEndpoint )
    {
        (void) remoteEndpoint; // suppress unused parameter warning

        try{
            // example of parsing single messages. osc::OsckPacketListener
            // handles the bundle traversal.
            
            if( std::strcmp( m.AddressPattern(), "/shady/pointbuffer" ) == 0 ){
                // example #1 -- argument stream interface
                
                test_osc_message(m);
                
            }
        }catch( osc::Exception& e ){
            // any parsing errors such as unexpected argument types, or 
            // missing arguments get thrown as exceptions.
            std::cout << "error while parsing message: "
                << m.AddressPattern() << ": " << e.what() << "\n";
        }
    }
};

int main(int argc, char* argv[])
{
    (void) argc; // suppress unused parameter warnings
    (void) argv; // suppress unused parameter warnings

    objman = new ObjectManager();

    ExamplePacketListener listener;
    UdpListeningReceiveSocket s(
            IpEndpointName( IpEndpointName::ANY_ADDRESS, PORT ),
            &listener );

    std::cout << "press ctrl-c to end\n";

    s.RunUntilSigInt();

    cout << endl << " ===== FINISHED =====" << endl;
    cout << "Number of objects created: " << objman->num_objects << endl;
    cout << "Number of objects freed: " << objman->num_objects_deleted << endl;
    return 0;
}

    