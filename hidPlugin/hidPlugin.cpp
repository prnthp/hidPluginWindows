// hidPlugin.cpp : Defines the exported functions for the DLL.
//
#include "framework.h"
#include "hidPlugin.h"
#include "hidapi.h"
#include <math.h>
#include <set>
#include <map>

std::map<std::tuple<int, int>, hid_device*> devices;

extern "C" 
{
    int r;
    hid_device* invalid_handle = NULL;

    HIDPLUGIN_API hid_device* Connect(int VID, int PID, int usage_page)
    {
        // USAGE_PAGE = 0xFF00
        // USAGE = 0x0100
        std::tuple<int, int> vidpid = std::make_tuple(VID, PID);

        hid_device* handle = invalid_handle;

        for (auto& device : devices)
        {
            if (device.first == vidpid)
            {
                return device.second;
            }
        }

        handle = hid_open(VID, PID, NULL);

        if (handle)
        {
            devices.insert(std::make_pair(vidpid, handle));
        }

        return handle;
    }

    unsigned char buf[65];
    char ret_buf[64];

    HIDPLUGIN_API void* Receive(hid_device* device_handle)
    {
        memset(ret_buf, 0, 64);
        int r = -1;

        for (auto& device : devices)
        {
            if (device.second == device_handle)
            {
                r = hid_read_timeout(device_handle, buf, 64, 1);
            }
        }

        // If recv fail, we set all to -1
        if (r <= 0)
        {
            for (int i = 0; i < 64; i++)
            {
                buf[i] = 0;
            }
        }

        memcpy(ret_buf, buf, 64);
        return ret_buf;
    }

    HIDPLUGIN_API int Send(hid_device* device_handle, char* buf, int num_bytes)
    {
        int r = 0;

        for (auto& device : devices)
        {
            if (device.second == device_handle)
            {
                int num_packets = ceil((float)num_bytes / 61);
                unsigned char write_buf[65];
                memset(write_buf, 0, 65);
                write_buf[1] = 0xAB; // Signature
                write_buf[2] = num_bytes >> 8;
                write_buf[3] = num_bytes;

                int idx = 0;
                for (int p = 0; p < num_packets; p++)
                {
                    for (int i = 4; i < 65; i++)
                    {
                        if (idx >= num_bytes)
                        {
                            write_buf[i] = 0;
                        }
                        else
                        {
                            write_buf[i] = buf[idx];
                        }
                        idx++;
                    }
                    r = hid_write(device_handle, write_buf, 65);
                }
                return r;
            }
        }

        return -1;
    }

    HIDPLUGIN_API void Disconnect(hid_device* device_handle)
    {
        for (auto device = devices.begin(); device != devices.end();)
        {
            if (device->second == device_handle)
            {
                hid_close(device_handle);
                devices.erase(device++);
            }
            else
            {
                ++device;
            }
        }

        return;
    }
}

// This is the constructor of a class that has been exported.
ChidPlugin::ChidPlugin()
{
    return;
}
