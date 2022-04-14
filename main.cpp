#include <iostream>
#include <iomanip>
#include <windows.h>
#include <cassert>
#include <set>
#include <iterator>
#include <algorithm>
#include "libusb/libusb.h"

constexpr uint32_t SLEEP_MS = 1000;


class Context
{
public:
    Context ()
    {
        assert(libusb_init(&m_context) == LIBUSB_SUCCESS);
    }
    ~Context()
    {
        libusb_exit(m_context);
    }
    libusb_context* get_context()
    {
        return m_context;
    }
private:
    libusb_context* m_context = nullptr;
};

class Device
{
public:
    Device(libusb_device* m_device)
        :m_device(m_device)
    {

        assert(libusb_get_device_descriptor(m_device, &m_desc) == LIBUSB_SUCCESS);
        m_bus = libusb_get_bus_number(m_device);
        m_port = libusb_get_port_number(m_device);
        m_serial = m_desc.iSerialNumber;
        m_vendor = m_desc.idVendor;
        m_product = m_desc.idProduct;
    }

    void print_info() const
    {
        std::cout << "Hub: "       << std::setfill('0') << std::setw(3) << static_cast <int>(libusb_get_bus_number(m_device)) << " "
                  << "Port: "      << std::setfill('0') << std::setw(2) << static_cast <int>(libusb_get_port_number(m_device)) << "; "
                  << "Serial: "    << std::setfill('0') << std::setw(4) << static_cast <int>(m_desc.iSerialNumber) << "; "
                  << "Vendor: 0x"  << std::setfill('0') << std::setw(4) << m_desc.idVendor << "; "
                  << "Product: 0x" << std::setfill('0') << std::setw(4) << m_desc.idProduct << ".";
        std::cout << std::endl;
    }

    uint16_t get_product() const
        {
            return m_product;
        }

    bool operator<(const Device &another) const
    {
        return get_product() < another.get_product();
    }

private:
    libusb_device* m_device;
    libusb_device_descriptor m_desc;
    uint8_t m_bus;
    uint8_t m_port;
    uint8_t m_serial;
    uint16_t m_vendor;
    uint16_t m_product;
};

class DeviceWatcher
{
public:
    DeviceWatcher(Context& m_ctx)
        :m_ctx(m_ctx.get_context())
    {
        m_cnt = libusb_get_device_list(m_ctx.get_context(), &m_devices);

        for(int i = 0; i < m_cnt; i++)
        {
            m_early_list.insert(Device(m_devices[i]));
        }
    }

    ~DeviceWatcher()
    {
        libusb_free_device_list(m_devices, m_cnt);
    }

    void update()
    {
        libusb_free_device_list(m_devices, m_cnt);
        m_cnt = libusb_get_device_list(m_ctx, &m_devices);
        m_new_devices.clear();
    }

    std::vector<Device>& get_new_devices()
    {
        std::set<Device> actual_list;
        for (int i = 0; i < m_cnt; i++)
        {
            actual_list.insert(Device(m_devices[i]));
        }

        std::set_difference(actual_list.begin(), actual_list.end(),
                            m_early_list.begin(), m_early_list.end(),
                            std::back_inserter(m_new_devices));

        m_early_list = actual_list;
        return m_new_devices;
    }

    libusb_device* operator[](size_t index)
    {
        if (index > m_cnt - 1)
            return nullptr;

        return m_devices[index];
    }

private:
    std::vector<Device> m_new_devices;
    std::set<Device> m_early_list;
    libusb_context* m_ctx;
    ssize_t m_cnt;
    libusb_device** m_devices;
};

int main()
{
  Context ctx;
  DeviceWatcher device_watcher(ctx);

  while(true)
  {
      device_watcher.update();

      for (auto& device : device_watcher.get_new_devices())
          device.print_info();

      Sleep(SLEEP_MS);
  }

  return 0;
}
