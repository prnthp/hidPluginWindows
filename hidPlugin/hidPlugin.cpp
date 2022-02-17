// hidPlugin.cpp : Defines the exported functions for the DLL.
//
#include "framework.h"
#include "hidPlugin.h"
#include "hidapi.h"
#include <math.h>

extern "C" 
{
    hid_device* device_handle;
    int latest_vid = 0;
    int latest_pid = 0;
    int latest_usage_page = 0;

    HIDPLUGIN_API int Connect(int VID, int PID, int usage_page)
    {
        latest_vid = VID;
        latest_pid = PID;
        latest_usage_page = usage_page;

        if (device_handle != nullptr)
        {
            hid_close(device_handle);
        }

        int r;
        r = hid_init();
        if (r < 0)
        {
            return r;
        }

        device_handle = hid_open(VID, PID, usage_page);
        if (device_handle == nullptr)
        {
            return -2;
        }
        else
        {
            return 1;
        }
    }

    unsigned char buf[65];
    char ret_buf[64];

    HIDPLUGIN_API void* Receive()
    {
        memset(ret_buf, 0, 64);

        if (device_handle == nullptr)
        {
            int connect_r = Connect(latest_vid, latest_pid, latest_usage_page);

            if (connect_r < 0)
            {
                return ret_buf;
            }
        }

        int r;
        r = hid_read_timeout(device_handle, buf, 64, 1);

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

    HIDPLUGIN_API int Send(char* buf, int num_bytes)
    {
        if (device_handle == nullptr)
        {
            int connect_r = Connect(latest_vid, latest_pid, latest_usage_page);

            if (connect_r < 0)
            {
                return -1;
            }
        }

        int r = 0;
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

    HIDPLUGIN_API void Disconnect()
    {
        if (device_handle != nullptr)
        {
            hid_close(device_handle);
            device_handle = nullptr;
        }
        return;
    }

    HIDPLUGIN_API int TestCommand()
    {
        if (device_handle != nullptr)
        {
            return latest_usage_page;
        }
        else
        {
            return 0;
        }
    }
}


// This is an example of an exported variable
HIDPLUGIN_API int nhidPlugin=0;

// This is an example of an exported function.
HIDPLUGIN_API int fnhidPlugin(void)
{
    return 0;
}

// This is the constructor of a class that has been exported.
ChidPlugin::ChidPlugin()
{
    return;
}
